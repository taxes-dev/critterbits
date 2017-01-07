#include <algorithm>

#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {

Sprite::Sprite() { this->debug = Engine::GetInstance().config->debug.draw_sprite_rects; }

Sprite::~Sprite() {}

CB_Rect Sprite::GetFrameRect() const {
    CB_Rect frame_rect;
    frame_rect.x = this->tile_offset_x + this->tile_width * (this->current_frame % this->sprite_sheet_cols);
    frame_rect.y = this->tile_offset_y + this->tile_height * (this->current_frame / this->sprite_sheet_cols);
    frame_rect.w = this->tile_width;
    frame_rect.h = this->tile_height;
    return frame_rect;
}

void Sprite::NotifyLoaded() {
    LOG_INFO("Sprite::NotifyLoaded sprite was loaded " + this->sprite_name);
    this->dim.w = this->tile_width * this->sprite_scale;
    this->dim.h = this->tile_height * this->sprite_scale;
    if (this->collision != CollisionType::None && this->collision_box.w == 0 && this->collision_box.h == 0) {
        this->collision_box.w = this->dim.w;
        this->collision_box.h = this->dim.h;
    }

    if (!this->sprite_sheet_path.empty()) {
        EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
            LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite sheet " + this->sprite_sheet_path);
            this->sprite_sheet =
                Engine::GetInstance().textures.GetTexture(this->sprite_sheet_path);
            if (this->sprite_sheet == nullptr) {
                LOG_ERR("Sprite::NotifyLoaded(pre-update) unable to load sprite sheet");
            } else {
                int w, h;
                SDL_QueryTexture(this->sprite_sheet.get(), NULL, NULL, &w, &h);
                this->sprite_sheet_cols = (w - this->tile_offset_x) / this->tile_width;
                this->sprite_sheet_rows = (h - this->tile_offset_y) / this->tile_height;
            }
            this->sprite_sheet_loaded = true;
        });
    }

    if (!this->script_path.empty()) {
        EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
            LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite script " + this->script_path);

            this->script = Engine::GetInstance().scripts.LoadScript(this->script_path);
            this->script_loaded = true;
        });
    }
}

void Sprite::NotifyUnloaded() {
    LOG_INFO("Sprite::NotifyUnloaded sprite was unloaded " + this->sprite_name);
    this->state = EntityState::Unloaded;
}

void Sprite::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (this->sprite_sheet != nullptr && clip_rect.z_index == ZIndex::Midground) {
        // FIXME: hack to prevent sprites from getting squished (GetFrameRect() needs to adjust for clipping)
        CB_Rect dst_rect = clip_rect.dest;
        dst_rect.x -= clip_rect.source.x;
        dst_rect.y -= clip_rect.source.y;
        dst_rect.w = this->dim.w;
        dst_rect.h = this->dim.h;
        SDL_SetTextureColorMod(this->sprite_sheet.get(), this->tint_and_opacity.r, this->tint_and_opacity.g, this->tint_and_opacity.b);
        SDL_SetTextureAlphaMod(this->sprite_sheet.get(), this->tint_and_opacity.a);
        SDLx::SDL_RenderTextureClipped(renderer, this->sprite_sheet.get(), this->GetFrameRect(), dst_rect,
                                        this->flip_x, this->flip_y);
    }
}

void Sprite::OnDebugRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (clip_rect.z_index == ZIndex::Foreground && this->sprite_name[0] != ':') {
        rectangleRGBA(renderer, clip_rect.dest.x, clip_rect.dest.y, clip_rect.dest.right(), clip_rect.dest.bottom(),
                        255, 0, 0, 127);
        boxRGBA(renderer, clip_rect.dest.x, clip_rect.dest.bottom(), clip_rect.dest.x + this->tag.length() * 8 + 2,
                clip_rect.dest.bottom() + 10, 255, 0, 0, 127);
        stringRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.bottom() + 1, this->tag.c_str(), 255, 255, 255,
                    255);
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

void Sprite::SetPosition(int new_x, int new_y) {
    if (!this->IsActive() || (new_x == this->dim.x && new_y == this->dim.y)) {
        return;
    }

    CB_Point new_xy = this->GetValidPosition(this->dim.x + this->collision_box.x, this->dim.y + this->collision_box.y,
        new_x + this->collision_box.x, new_y + this->collision_box.y);

    this->dim.x = new_xy.x - this->collision_box.x;
    this->dim.y = new_xy.y - this->collision_box.y;
}

bool Sprite::OnStart() {
    // delay start until resources loaded
    if (this->sprite_sheet_loaded && this->script_loaded) {
        return true;
    }
    return false;
}

void Sprite::OnUpdate(float delta_time) {
    for (auto it = this->animations.begin(); it != this->animations.end();) {
        if ((*it)->IsDestroyed()) {
            it = this->animations.erase(it);
        } else {
            (*it)->Animate(shared_from_this(), delta_time);
            it++;
        }
    }
}
}