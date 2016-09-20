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

entity_id_t GuiManager::OpenPanel(const std::string & panel_name, bool multiple) {
    if (multiple == false) {
        // if we're not allowing open of multiple of same panel, look for an existing one
        for (auto it = this->panels.begin(); it != this->panels.end(); it++) {
            if ((*it)->panel_name == panel_name) {
                (*it)->Open();
                return (*it)->entity_id;
            }
        }
    }

    // DEBUG
    std::shared_ptr<Gui::GuiPanel> panel = std::make_shared<Gui::GuiPanel>();
    panel->panel_name = "test panel";
    panel->dim.x = 20;
    panel->dim.y = Engine::GetInstance().viewport->dim.h - 120;
    panel->dim.w = Engine::GetInstance().viewport->dim.w - 40;
    panel->dim.h = 100;
    this->panels.push_back(panel);
    // DEBUG

    panel->Open();
    return panel->entity_id;
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