#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>

namespace Critterbits {
namespace Gui {
GuiLabel::~GuiLabel() { SDLx::SDL_CleanUp(this->rendered_text); }

SDL_Texture * GuiLabel::CreateTextureFromText(SDL_Renderer * renderer) {
    if (this->text_is_dirty) {
        SDLx::SDL_CleanUp(this->rendered_text);
        this->rendered_text = nullptr;

        SDL_Surface * surface = TTF_RenderUTF8_Blended(this->font_resource->font, this->text.c_str(),
                                                       static_cast<SDL_Color>(this->text_color));
        if (surface != nullptr) {
            this->rendered_text = SDL_CreateTextureFromSurface(renderer, surface);
            if (this->rendered_text != nullptr) {
                this->rendered_text_size.x = 0;
                this->rendered_text_size.y = 0;
                SDL_QueryTexture(this->rendered_text, NULL, NULL, &this->rendered_text_size.w,
                                 &this->rendered_text_size.h);
            } else {
                LOG_SDL_ERR("GuiLabel::CreateTextureFromText unable to create texture from rendered text surface");
            }
            SDLx::SDL_CleanUp(surface);
        } else {
            LOG_SDL_ERR("GuiLabel::CreateTextureFromText unable to create surface from font/text");
        }
        this->text_is_dirty = false;
    }
    return this->rendered_text;
}

void GuiLabel::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        if (this->bg_color.a > 0) {
            boxRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(),
                    this->bg_color.r, this->bg_color.g, this->bg_color.b, this->bg_color.a);
        }
        if (this->font_name.empty()) {
            // render using built-in font
            std::string label_text{this->text};
            if (CB_SDL_GFX_FONT_W * label_text.length() > static_cast<size_t>(this->dim.w)) {
                label_text = label_text.substr(0, std::max(this->dim.w / CB_SDL_GFX_FONT_W, 0));
            }
            stringRGBA(renderer, clip_info.dest.x, clip_info.dest.y, label_text.c_str(), this->text_color.r,
                       this->text_color.g, this->text_color.b, this->text_color.a);
        } else if (this->font_resource != nullptr) {
            // render using custom font
            SDL_Texture * texture = this->CreateTextureFromText(renderer);
            if (texture != nullptr) {
                // we have to adjust the destination clip, otherwise the font gets stretched
                CB_Rect dest{clip_info.dest.x, clip_info.dest.y, std::min(clip_info.dest.w, this->rendered_text_size.w),
                             std::min(clip_info.dest.h, this->rendered_text_size.h)};
                CB_Rect src{this->rendered_text_size.x, this->rendered_text_size.y, dest.w, dest.h};
                SDLx::SDL_RenderTextureClipped(renderer, texture, src, dest);
            }
        }
    }
}

void GuiLabel::OnResize() {
    if (this->font_name.empty()) {
        // using SDL2_gfx font
        this->dim.w = CB_SDL_GFX_FONT_W * this->text.length();
        this->dim.h = CB_SDL_GFX_FONT_H;
    } else if (this->font_resource != nullptr) {
        // using custom font
        if (TTF_SizeText(this->font_resource->font, this->text.c_str(), &this->dim.w, &this->dim.h) != 0) {
            LOG_SDL_ERR("GuiLabel::OnResize unable to size text using custom font");
        }
    }
}

bool GuiLabel::OnStart() {
    if (!this->font_name.empty()) {
        this->font_resource = Engine::GetInstance().fonts.GetNamedFont(this->font_name);
        this->Resize();
    }
    return true;
}

void GuiLabel::SetText(const std::string & new_text) {
    if (new_text != this->text) {
        this->text = new_text;
        this->text_is_dirty = true;
        if (this->IsActive()) {
            this->Resize();
        }
    }
}
}
}