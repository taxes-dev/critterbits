#pragma once
#ifndef CBGUI_HPP
#define CBGUI_HPP

#include <map>
#include <memory>
#include <vector>

#include <SDL.h>

#include "coord.hpp"
#include "entity.hpp"
#include "resource.hpp"

#define CB_GUI_PATH "gui"
#define CB_GUI_EXT ".toml"

#define CB_GUI_IMAGE_CONTROL "image"
#define CB_GUI_LABEL_CONTROL "label"

#define CB_GUI_DEFAULT_MIN_W 0
#define CB_GUI_DEFAULT_MAX_W 10000
#define CB_GUI_DEFAULT_MIN_H 0
#define CB_GUI_DEFAULT_MAX_H 10000

#define CB_SDL_GFX_FONT_W 8
#define CB_SDL_GFX_FONT_H 8

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

    int GetBorderBottomScaled() const { return this->border.bottom * this->scale; };
    int GetBorderLeftScaled() const { return this->border.left * this->scale; };
    int GetBorderRightScaled() const { return this->border.right * this->scale; };
    int GetBorderTopScaled() const { return this->border.top * this->scale; };
    SDL_Texture * SliceTo(int, int);

  private:
    std::shared_ptr<SDL_Texture> sliced;
    CB_Rect texture_bounds;
    CB_Point last_sliced_to;
};

enum class ResizeBehavior { Static, Resize };

class GuiControl : public Entity {
  public:
    struct {
        CB_Point at;
        int row_span{1};
        int col_span{1};
    } grid;
    CB_Color bg_color;
    ResizeBehavior resize_behavior{ResizeBehavior::Static};
    int max_w{CB_GUI_DEFAULT_MAX_W};
    int max_h{CB_GUI_DEFAULT_MAX_H};
    int min_w{CB_GUI_DEFAULT_MIN_W};
    int min_h{CB_GUI_DEFAULT_MIN_H};

    GuiControl();
    EntityType GetEntityType() const { return EntityType::GuiControl; };
    void Resize();
    int SortOrder(int width) { return this->grid.at.y * width + this->grid.at.x; }

  protected:
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    virtual void OnResize(){};
};

enum class GuiImageMode { Actual, Fill };

class GuiImage : public GuiControl {
  public:
    GuiImageMode image_mode{GuiImageMode::Actual};
    std::shared_ptr<SDL_Texture> image_texture;

    GuiImage() : GuiControl() { this->resize_behavior = ResizeBehavior::Resize; };

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnResize();

  private:
    CB_Rect image_size;

    CB_Rect GetImageSize();
};

class GuiLabel : public GuiControl {
  public:
    std::string font_name;
    CB_Color text_color{0, 0, 0, 255};

    GuiLabel() : GuiControl() { this->resize_behavior = ResizeBehavior::Resize; };
    ~GuiLabel();
    void SetText(const std::string &);

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnResize();
    bool OnStart();

  private:
    bool text_is_dirty{false};
    std::string text;
    std::shared_ptr<TTF_FontWrapper> font_resource;
    SDL_Texture * rendered_text{nullptr};
    CB_Rect rendered_text_size;

    SDL_Texture * CreateTextureFromText(SDL_Renderer *);
};

typedef struct CB_GridDescriptor {
    int actual_size{0};
    int position{0};
    int desired_size{0};
    bool flex{false};
} CB_GridDescriptor;

class GridLayout {
  public:
    static const int FLEX = -1;

    GridLayout(int rows, int cols, int h_padding, int v_padding)
        : h_padding(h_padding), v_padding(v_padding), rows(static_cast<size_t>(rows)),
          cols(static_cast<size_t>(cols)){};
    GridLayout(int rows, int cols) : GridLayout(rows, cols, 0, 0){};
    CB_Rect GetCellRect(int, int, int = 1, int = 1) const;
    int GetColumnCount() const { return this->cols.size(); };
    int GetRowCount() const { return this->rows.size(); };
    void Reflow(const CB_Rect &, bool = false);
    void SetColumnWidth(int, int);
    void SetRowHeight(int, int);

  private:
    int h_padding;
    int v_padding;
    std::vector<CB_GridDescriptor> rows;
    std::vector<CB_GridDescriptor> cols;
};

class GuiPanel : public Entity {
  public:
    std::string panel_name;
    bool destroy_on_close{false};
    FlexRect flex;
    NineSliceImage decoration;
    std::vector<std::shared_ptr<GuiControl>> children;
    std::unique_ptr<GridLayout> layout;

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