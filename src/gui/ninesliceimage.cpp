#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {

SDL_Texture * NineSliceImage::SliceTo(int w, int h) {
    CB_Point target_dim{w, h};
    if (target_dim == this->last_sliced_to) {
        return this->sliced.get();
    }

    if (this->texture_bounds.w == 0) {
        SDL_QueryTexture(this->texture.get(), NULL, NULL, &this->texture_bounds.w, &this->texture_bounds.h);
    }

    this->sliced = TextureManager::GetInstance().CreateTargetTexture(
        w, h, this->scale, [&](SDL_Renderer * renderer, SDL_Texture * texture) {
            // clear texture to transparent
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            // convenience variables
            int bt = this->border.top, bl = this->border.left, br = this->border.right, bb = this->border.bottom;
            int tbw = this->texture_bounds.w, tbh = this->texture_bounds.h;
            SDL_Texture * srctex = this->texture.get();

            // adjust the target width/height based on our scale
            w /= scale;
            h /= scale;

            // top left
            CB_Rect src{0, 0, bl, bt};
            CB_Rect dst = src;
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // top right
            src = {tbw - br, 0, br, bt};
            dst = {w - br, 0, br, bt};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // bottom left
            src = {0, tbh - bb, bl, bb};
            dst = {0, h - bb, bl, bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // bottom right
            src = {tbw - br, tbh - bb, br, bb};
            dst = {w - br, h - bb, br, bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // top middle
            src = {bl, 0, tbw - bl - br, bt};
            dst = {bl, 0, w - bl - br, bt};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // bottom middle
            src = {bl, tbh - bb, tbw - bl - br, bb};
            dst = {bl, h - bb, w - bl - br, bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // left middle
            src = {0, bt, bl, tbh - bt - bb};
            dst = {0, bt, bl, h - bt - bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // right middle
            src = {tbw - br, bt, br, tbh - bt - bb};
            dst = {w - br, bt, br, h - bt - bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);

            // middle
            src = {bt, bl, tbw - bl - br, tbh - bt - bb};
            dst = {bt, bl, w - bl - br, h - bt - bb};
            SDLx::SDL_RenderTextureClipped(renderer, srctex, src, dst);
        });
    this->last_sliced_to = target_dim;
    return this->sliced.get();
}
}
}