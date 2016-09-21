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
    float scale{1.0f};

    SDL_Texture * SliceTo(int, int);

  private:
    std::shared_ptr<SDL_Texture> sliced;
    CB_Rect texture_bounds;
    CB_Point last_sliced_to;
};

class GuiControl : public Entity {
    public:
    CB_Point grid{0, 0};

    GuiControl() : Entity() { this->debug = true; };
    EntityType GetEntityType() const { return EntityType::GuiControl; };
    int SortOrder(int width) { return this->grid.y * width + this->grid.x; }

    protected:
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
};

class GuiPanel : public Entity {
  public:
    std::string panel_name;
    bool destroy_on_close{false};
    FlexRect flex;
    NineSliceImage decoration;
    std::vector<std::shared_ptr<GuiControl>> children;
    int grid_rows{1};
    int grid_cols{1};

    GuiPanel() : Entity() { this->debug = true; };
    void Close();
    EntityType GetEntityType() const { return EntityType::GuiPanel; };
    void Open();
    void Reflow(const CB_Rect &);

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);

    private:
    CB_ViewClippingInfo AdjustClipToClientArea(const CB_ViewClippingInfo &, const CB_Rect &) const;
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