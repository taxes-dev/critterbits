#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {

Sprite::Sprite() { this->draw_debug = Engine::GetInstance().config->debug.draw_sprite_rects; }

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
    LOG_INFO("Sprite::NotifyLoaded sprite was loaded " + this->sprite_name);
    this->dim.x = 0;
    this->dim.y = 0;
    this->dim.w = this->tile_width * this->sprite_scale;
    this->dim.h = this->tile_height * this->sprite_scale;

    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite sheet " + this->sprite_sheet_path);
        this->sprite_sheet = IMG_LoadTexture(Engine::GetInstance().GetRenderer(), this->sprite_sheet_path.c_str());
        if (this->sprite_sheet == nullptr) {
            LOG_SDL_ERR("Sprite::NotifyLoaded(pre-update) unable to load sprite sheet to texture");
            return;
        }

        int w, h;
        SDL_QueryTexture(this->sprite_sheet, NULL, NULL, &w, &h);
        this->sprite_sheet_cols = (w - this->tile_offset_x) / this->tile_width;
        this->sprite_sheet_rows = (h - this->tile_offset_y) / this->tile_height;
    });

    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite script " + this->sprite_name);

        this->script = Engine::GetInstance().scripts.LoadScript(this->sprite_name);
    });
}

void Sprite::NotifyUnloaded() { LOG_INFO("Sprite::NotifyUnloaded sprite was unloaded " + this->sprite_name); }

void Sprite::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (this->sprite_sheet != nullptr) {
        SDLx::SDL_RenderTextureClipped(renderer, this->sprite_sheet, this->GetFrameRect(), clip_rect.dest, this->flip_x,
                                       this->flip_y);
    }
    if (this->draw_debug) {
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

void Sprite::SetFrame(int frame) { this->current_frame = Clamp(frame, 0, this->GetFrameCount()); }
}