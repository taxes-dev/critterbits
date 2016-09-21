#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {

bool GuiManager::ClosePanel(entity_id_t panel_id) {
    for (auto it = this->panels.begin(); it != this->panels.end(); it++) {
        if ((*it)->entity_id == panel_id) {
            (*it)->Close();
            return true;
        }
    }
    return false;
}

std::shared_ptr<GuiPanel> GuiManager::LoadGuiPanel(const std::string & gui_name) {
    if (gui_name.empty()) {
        return nullptr;
    }

    std::string gui_path = CB_GUI_PATH PATH_SEP_STR + gui_name + CB_GUI_EXT;
    LOG_INFO("GuiManager::LoadGuiPanel attempting to load GUI panel from " + gui_path);
    auto gui_file = Engine::GetInstance().GetResourceLoader()->OpenTextResource(gui_path);
    Toml::TomlParser parser{gui_file};
    if (parser.IsReady()) {
        std::shared_ptr<GuiPanel> panel = std::make_shared<GuiPanel>();
        panel->panel_name = gui_name;
        panel->destroy_on_close = parser.GetTableBool("panel.destroy_on_close");
        parser.GetTableFlexRect("panel.flex", &panel->flex);
        return std::move(panel);
    } else {
        LOG_ERR("GuiManager::LoadGuiPanel unable to load GUI panel: " + parser.GetParserError());
        return nullptr;
    }
}

std::shared_ptr<GuiPanel> GuiManager::OpenPanel(const std::string & panel_name, bool multiple) {
    if (multiple == false) {
        // if we're not allowing open of multiple of same panel, look for an existing one
        for (auto it = this->panels.begin(); it != this->panels.end(); it++) {
            if ((*it)->panel_name == panel_name) {
                (*it)->Open();
                return *it;
            }
        }
    }

    std::shared_ptr<GuiPanel> panel = this->LoadGuiPanel(panel_name);
    if (panel != nullptr) {
        this->panels.push_back(panel);
        CB_Rect viewport_bounds = Engine::GetInstance().viewport->dim;
        viewport_bounds.x = 0;
        viewport_bounds.y = 0;
        panel->Reflow(viewport_bounds);
        panel->Open();
        return panel;
    }
    return nullptr;
}

void GuiManager::UnloadPanel(std::shared_ptr<GuiPanel> panel) {
    if (panel != nullptr) {
        panel->state = EntityState::Unloaded;
        for (auto it = this->panels.begin(); it != this->panels.end(); it++) {
            if (*it == panel) {
                this->panels.erase(it);
                break;
            }
        }
    }
}

}
}