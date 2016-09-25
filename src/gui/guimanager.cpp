#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {
/*
* Control parsing functions
*/
namespace {
std::shared_ptr<GuiImage> ParseImage(const Toml::TomlParser & table, const std::string & gui_path) {
    std::shared_ptr<GuiImage> image = std::make_shared<GuiImage>();
    std::string image_path = table.GetTableString("image");
    if (!image_path.empty()) {
        image->image_texture = Engine::GetInstance().textures.GetTexture(image_path, gui_path);
    }
    std::string image_mode = table.GetTableString("image_mode", "actual");
    if (image_mode == "actual") {
        image->image_mode = GuiImageMode::Actual;
    } else if (image_mode == "fill") {
        image->image_mode = GuiImageMode::Fill;
    } else {
        LOG_ERR("ParseImage unknown image_mode " + image_mode);
    }
    return std::move(image);
}

std::shared_ptr<GuiLabel> ParseLabel(const Toml::TomlParser & table) {
    std::shared_ptr<GuiLabel> label = std::make_shared<GuiLabel>();
    label->SetText(table.GetTableString("text"));
    label->text_color = table.GetTableColor("text_color", label->text_color);
    label->font_name = table.GetTableString("font");
    return std::move(label);
}

void ParseGridLayout(const Toml::TomlParser & table, std::shared_ptr<GuiPanel> panel) {
    CB_Point grid_padding;
    grid_padding = table.GetTablePoint("panel.grid_padding", grid_padding);
    std::vector<std::string> col_widths;
    table.GetArrayString("panel.grid_columns", &col_widths);
    if (col_widths.size() == 0) {
        col_widths.push_back("*");
    }
    std::vector<std::string> row_heights;
    table.GetArrayString("panel.grid_rows", &row_heights);
    if (row_heights.size() == 0) {
        row_heights.push_back("*");
    }

    std::unique_ptr<GridLayout> layout{
        new GridLayout(row_heights.size(), col_widths.size(), grid_padding.x, grid_padding.y)};
    int i = 0;
    for (auto & width : col_widths) {
        if (width == "*") {
            layout->SetColumnWidth(i, GridLayout::FLEX);
        } else {
            layout->SetColumnWidth(i, std::stol(width));
        }
        i++;
    }
    i = 0;
    for (auto & height : row_heights) {
        if (height == "*") {
            layout->SetRowHeight(i, GridLayout::FLEX);
        } else {
            layout->SetRowHeight(i, std::stol(height));
        }
        i++;
    }

    panel->layout = std::move(layout);
}

std::shared_ptr<GuiControl> ParseGuiControlOfType(const std::string & control_type, const Toml::TomlParser & table,
                                                  const std::string & gui_path) {
    if (control_type == CB_GUI_LABEL_CONTROL) {
        return std::dynamic_pointer_cast<GuiControl>(ParseLabel(table));
    } else if (control_type == CB_GUI_IMAGE_CONTROL) {
        return std::dynamic_pointer_cast<GuiControl>(ParseImage(table, gui_path));
    } else {
        return nullptr;
    }
}
}
/*
* End control parsing functions
*/

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
        // paser GuiPanel
        std::shared_ptr<GuiPanel> panel = std::make_shared<GuiPanel>();
        panel->panel_name = gui_name;
        panel->tag = parser.GetTableString("panel.tag");
        panel->destroy_on_close = parser.GetTableBool("panel.destroy_on_close");
        ParseGridLayout(parser, panel);
        parser.GetTableFlexRect("panel.flex", &panel->flex);
        std::string nineslice_image = parser.GetTableString("decoration.image");
        if (!nineslice_image.empty()) {
            panel->decoration.texture = Engine::GetInstance().textures.GetTexture(nineslice_image, gui_path);
        }
        panel->decoration.scale = parser.GetTableFloat("decoration.scale", panel->decoration.scale);
        int border = parser.GetTableInt("decoration.border");
        // TODO: parse irregular border specs
        panel->decoration.border.top = border;
        panel->decoration.border.left = border;
        panel->decoration.border.right = border;
        panel->decoration.border.bottom = border;

        // parse child controls
        parser.IterateTableArray("control", [&panel, &gui_path](const Toml::TomlParser & table) {
            std::string control_type = table.GetTableString("type");
            std::shared_ptr<GuiControl> control = ParseGuiControlOfType(control_type, table, gui_path);
            if (control != nullptr) {
                control->tag = table.GetTableString("tag");
                control->grid.at = table.GetTablePoint("grid");
                control->grid.row_span = table.GetTableInt("row_span", control->grid.row_span);
                control->grid.col_span = table.GetTableInt("col_span", control->grid.col_span);
                control->dim = table.GetTableRect("size");
                CB_Rect maxes = table.GetTableRect("max_size", CB_Rect{0, 0, control->max_w, control->max_h});
                control->max_w = Clamp(maxes.w, 0, CB_GUI_DEFAULT_MAX_W);
                control->max_h = Clamp(maxes.h, 0, CB_GUI_DEFAULT_MAX_H);
                CB_Rect mins = table.GetTableRect("min_size", CB_Rect{0, 0, control->min_w, control->min_h});
                control->min_w = Clamp(mins.w, 0, control->max_w);
                control->min_h = Clamp(mins.h, 0, control->max_h);
                control->bg_color = table.GetTableColor("background_color", control->bg_color);
                if (table.GetTableBool("resize", control->resize_behavior == ResizeBehavior::Resize)) {
                    control->resize_behavior = ResizeBehavior::Resize;
                } else {
                    control->resize_behavior = ResizeBehavior::Static;
                }
                if (control->grid.at.x < 0 || control->grid.at.x > panel->layout->GetColumnCount() - 1) {
                    LOG_ERR("GuiManager::LoadGuiPanel control grid X outside of grid");
                } else if (control->grid.at.y < 0 || control->grid.at.y > panel->layout->GetRowCount() - 1) {
                    LOG_ERR("GuiManager::LoadGuiPanel control grid Y outside of grid");
                } else {
                    panel->children.push_back(std::move(control));
                }
            } else {
                LOG_ERR("GuiManager::LoadGuiPanel unable to load control with type " + control_type);
            }
        });

        // sort child controls based on grid position
        int column_count = panel->layout->GetColumnCount();
        std::sort(panel->children.begin(), panel->children.end(),
                  [column_count](const std::shared_ptr<GuiControl> & lhs, const std::shared_ptr<GuiControl> & rhs) {
                      return lhs->SortOrder(column_count) < rhs->SortOrder(column_count);
                  });

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
        EngineEventQueue::GetInstance().QueuePreUpdate([panel]() {
            CB_Rect viewport_bounds = Engine::GetInstance().viewport->dim;
            viewport_bounds.x = 0;
            viewport_bounds.y = 0;
            panel->Reflow(viewport_bounds);
            panel->Open();
        });
        return std::move(panel);
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