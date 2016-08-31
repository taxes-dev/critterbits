#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {
bool cb_draw_debug_sprite_rects = false;

Sprite::~Sprite() {
    if (this->sprite_sheet != nullptr) {
        SDLx::SDL_CleanUp(this->sprite_sheet);
    }
}

CB_Rect Sprite::GetFrameRect() const {
    CB_Rect frame_rect;
    frame_rect.x = this->tile_offset_x + this->tile_width * (this->current_frame % this->sprite_sheet_rows);
    frame_rect.y = this->tile_offset_y + this->tile_height * (this->current_frame / this->sprite_sheet_cols);
    frame_rect.w = this->tile_width;
    frame_rect.h = this->tile_height;
    return frame_rect;
}

void Sprite::NotifyLoaded() {
    LOG_INFO("Sprite::NotifyLoaded sprite was loaded " + this->tag);
    this->dim.x = 0;
    this->dim.y = 0;
    this->dim.w = this->tile_width * this->sprite_scale;
    this->dim.h = this->tile_height * this->sprite_scale;

    LOG_INFO("Sprite::NotifyLoaded attempting to load sprite sheet " + this->sprite_sheet_path);
    SDL_Surface * sheet_surface = IMG_Load(this->sprite_sheet_path.c_str());
    if (sheet_surface == nullptr) {
        LOG_SDL_ERR("Sprite::NotifyLoaded unable to load sprite sheet");
        return;
    }
    this->sprite_sheet = SDL_CreateTextureFromSurface(cb_main_renderer, sheet_surface);
    SDLx::SDL_CleanUp(sheet_surface);
    if (this->sprite_sheet == nullptr) {
        LOG_SDL_ERR("Sprite::NotifyLoaded unable to convert sprite sheet to texture");
        return;
    }

    int w, h;
    SDL_QueryTexture(this->sprite_sheet, NULL, NULL, &w, &h);
    this->sprite_sheet_cols = (w - this->tile_offset_x) / this->tile_width;
    this->sprite_sheet_rows = (h - this->tile_offset_y) / this->tile_height;
}

void Sprite::NotifyUnloaded() { LOG_INFO("Sprite::NotifyUnloaded sprite was unloaded " + this->tag); }

void Sprite::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (this->sprite_sheet != nullptr) {
        SDLx::SDL_RenderTextureClipped(renderer, this->sprite_sheet, this->GetFrameRect(), clip_rect.dest, this->flip_x,
                                       this->flip_y);
    }
    if (cb_draw_debug_sprite_rects) {
        rectangleRGBA(renderer, clip_rect.dest.x, clip_rect.dest.y, clip_rect.dest.right(), clip_rect.dest.bottom(),
                      255, 0, 0, 127);
        boxRGBA(renderer, clip_rect.dest.x, clip_rect.dest.bottom(), clip_rect.dest.x + this->tag.length() * 8 + 2,
                clip_rect.dest.bottom() + 10, 255, 0, 0, 127);
        stringRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.bottom() + 1, this->tag.c_str(), 255, 255, 255, 255);
        std::string coords = this->dim.xy().to_string();
        boxRGBA(renderer, clip_rect.dest.right() - 8 * coords.length() - 2, clip_rect.dest.y - 10,
                clip_rect.dest.right() - 1, clip_rect.dest.y - 1, 255, 0, 0, 127);
        stringRGBA(renderer, clip_rect.dest.right() - 8 * coords.length() - 1, clip_rect.dest.y - 9, coords.c_str(),
                   255, 255, 255, 255);
        std::string f = std::to_string(this->current_frame);
        boxRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.y + 1, clip_rect.dest.x + f.length() * 8 + 2,
                clip_rect.dest.y + 10, 255, 0, 0, 127);
        stringRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.y + 1, f.c_str(), 255, 255, 255, 255);
    }
}

void Sprite::SetFrame(int frame) {
    if (frame >= 0 && frame < this->GetFrameCount()) {
        this->current_frame = frame;
    }
}
}