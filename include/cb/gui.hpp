#pragma once
#ifndef CBGUI_HPP
#define CBGUI_HPP

#include <map>
#include <memory>
#include <vector>

#include <SDL.h>

#include "coord.hpp"
#include "entity.hpp"

#define CB_GUI_PATH "gui"
#define CB_GUI_EXT ".toml"

namespace Critterbits {
namespace Gui {

class NineSliceImage {
  public:
    std::shared_ptr<SDL_Texture> texture;
    struct {
        int top{0};
        int left{0};
        int bottom{0};
        int right{0};
    } border;
};

class GuiPanel : public Entity {
  public:
    std::string panel_name;
    bool destroy_on_close{false};
    FlexRect flex;
    std::shared_ptr<NineSliceImage> decoration;

    GuiPanel() : Entity() { this->debug = true; };
    void Close();
    EntityType GetEntityType() const { return EntityType::GuiPanel; };
    void Open();
    void Reflow(const CB_Rect &);

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
};

class GuiManager {
  public:
    std::vector<std::shared_ptr<GuiPanel>> panels;

    bool ClosePanel(entity_id_t);
    std::shared_ptr<GuiPanel> OpenPanel(const std::string &, bool = false);
    void UnloadPanel(std::shared_ptr<GuiPanel>);

  private:
    std::shared_ptr<GuiPanel> LoadGuiPanel(const std::string &);
};
}
}
#endif