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

#define CB_GUI_LABEL_CONTROL "label"

#define CB_GUI_DEFAULT_MIN_W 0
#define CB_GUI_DEFAULT_MAX_W 10000
#define CB_GUI_DEFAULT_MIN_H 0
#define CB_GUI_DEFAULT_MAX_H 10000

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

enum class ResizeBehavior { Static, Resize };

class GuiControl : public Entity {
  public:
    CB_Point grid{0, 0};
    CB_Color bg_color;
    ResizeBehavior resize_behavior{ResizeBehavior::Static};
    int max_w{CB_GUI_DEFAULT_MAX_W};
    int max_h{CB_GUI_DEFAULT_MAX_H};
    int min_w{CB_GUI_DEFAULT_MIN_W};
    int min_h{CB_GUI_DEFAULT_MIN_H};

    GuiControl() : Entity(){};
    EntityType GetEntityType() const { return EntityType::GuiControl; };
    void Resize();
    int SortOrder(int width) { return this->grid.y * width + this->grid.x; }

  protected:
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    virtual void OnResize(){};
};

class GuiLabel : public GuiControl {
  public:
    std::string text;
    CB_Color text_color{0, 0, 0, 255};

    GuiLabel() : GuiControl() { this->resize_behavior = ResizeBehavior::Resize; };

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnResize();
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
    CB_Point grid_padding;

    GuiPanel();
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