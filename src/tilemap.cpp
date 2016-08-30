#include <SDL.h>
#include <SDL_image.h>
#include <tmx/tmx.h>

#include "cbengine.h"
#include "cblogging.h"
#include "cbsdl.h"
#include "cbtilemap.h"

namespace Critterbits {

/*
 * Support functions for Tilemap::RenderMap()
 */
static void tilemap_set_color(int color) {
    unsigned char r, g, b;

    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = (color)&0xFF;

    SDL_SetRenderDrawColor(main_renderer, r, g, b, SDL_ALPHA_OPAQUE);
}

static void tilemap_draw_objects(const tmx_object_group * objgr) {
    LOG_INFO("tilemap_draw_objects called");
    SDL_Rect rect;
    tilemap_set_color(objgr->color);
    tmx_object * head = objgr->head;
    /* FIXME line thickness */
    while (head) {
        if (head->visible) {
            if (head->shape == S_SQUARE) {
                rect.x = head->x;
                rect.y = head->y;
                rect.w = head->width;
                rect.h = head->height;
                SDL_RenderDrawRect(main_renderer, &rect);
            } else if (head->shape == S_POLYGON) {
                // draw_polygon(head->points, head->x, head->y, head->points_len);
            } else if (head->shape == S_POLYLINE) {
                // draw_polyline(head->points, head->x, head->y, head->points_len);
            } else if (head->shape == S_ELLIPSE) {
                /* FIXME: no function in SDL2 */
            }
        }
        head = head->next;
    }
}

/*
 * End support functions
 */

Tilemap::~Tilemap() {
    if (this->map != nullptr) {
        tmx_map_free(this->map);
    }
}

bool Tilemap::CreateTextures(float scale) {
    // check to see if we already created the textures
    if (this->map_texture != nullptr) {
        return true;
    }

    // first load up the tilemap
    if (!(this->map = tmx_load(this->tmx_path.c_str()))) {
        LOG_ERR("Tilemap::CreateTextures unable to load TMX map " + tmx_errno);
        return false;
    }

    this->tile_height = this->map->tile_height * scale;
    this->tile_width = this->map->tile_width * scale;
    this->map_rect.w = this->map->width * this->tile_height;
    this->map_rect.h = this->map->height * this->tile_width;
    this->map_rect.x = 0;
    this->map_rect.y = 0;

    this->map_texture = this->RenderMap(main_renderer, scale);

    return this->map_texture != nullptr;
}

SDL_Texture * Tilemap::RenderMap(SDL_Renderer * renderer, float scale) const {
    SDL_Texture * texture;

    if (!(texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->map_rect.w,
                                      this->map_rect.h))) {
        LOG_SDL_ERR("Tilemap::Rendermap unable to create texture for map");
        return nullptr;
    }

    // set render target to the map texture
    SDL_SetRenderTarget(renderer, texture);

    // clear map to background color
    tilemap_set_color(this->map->backgroundcolor);
    SDL_RenderClear(renderer);

    // iterate layers and draw
    tmx_layer * current_layer = this->map->ly_head;
    while (current_layer) {
        if (current_layer->visible) {
            if (current_layer->type == L_OBJGR) {
                tilemap_draw_objects(current_layer->content.objgr);
            } else if (current_layer->type == L_IMAGE) {
                this->DrawImageLayer(renderer, texture, current_layer, scale);
            } else if (current_layer->type == L_LAYER) {
                this->DrawMapLayer(renderer, texture, current_layer);
            }
        }
        current_layer = current_layer->next;
    }

    // reset render target
    SDL_SetRenderTarget(renderer, NULL);

    return texture;
}

void Tilemap::DrawImageLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer,
                             float scale) const {
    SDL_Rect dim;
    SDL_Texture * source_image = static_cast<SDL_Texture *>(layer->content.image->resource_image);
    float op = layer->opacity;

    dim.x = layer->offsetx;
    dim.y = layer->offsety;
    SDL_QueryTexture(source_image, NULL, NULL, &(dim.w), &(dim.h));

    SDL_Rect dst;
    dst.x = dim.x * scale;
    dst.y = dim.y * scale;
    dst.w = dim.w * scale;
    dst.h = dim.h * scale;

    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(source_image, op * SDL_ALPHA_OPAQUE);
    }
    SDL_RenderCopy(renderer, source_image, NULL, &dst);
    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
        SDL_SetTextureAlphaMod(source_image, SDL_ALPHA_OPAQUE);
    }
}

void Tilemap::DrawMapLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) const {
    tmx_tileset * ts;
    tmx_image * im;
    SDL_Rect srcrect, dstrect;
    SDL_Texture * tile;

    // prepare blend mode if opacity less than 100%
    float op = layer->opacity;
    int alpha_mod = op * SDL_ALPHA_OPAQUE;
    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    // loop through tiles in map
    for (unsigned long i = 0; i < map->height; i++) {
        for (unsigned long j = 0; j < map->width; j++) {
            unsigned int gid = (layer->content.gids[(i * map->width) + j]) & TMX_FLIP_BITS_REMOVAL;

            // if we have a tile at this position, draw it
            if (map->tiles[gid] != NULL) {
                // source tileset and image
                ts = map->tiles[gid]->tileset;
                im = map->tiles[gid]->image;

                // source dimensions and position
                srcrect.x = map->tiles[gid]->ul_x;
                srcrect.y = map->tiles[gid]->ul_y;
                srcrect.w = ts->tile_width;
                srcrect.h = ts->tile_height;

                // destination dimensions and position (accounts for any scaling)
                dstrect.w = this->tile_width;
                dstrect.h = this->tile_height;
                dstrect.x = j * this->tile_width;
                dstrect.y = i * this->tile_height;

                /* TODO Flips */
                // select source tile or image
                if (im) {
                    tile = static_cast<SDL_Texture *>(im->resource_image);
                } else {
                    tile = static_cast<SDL_Texture *>(ts->image->resource_image);
                }

                // set alpha modulation based on layer opacity
                if (alpha_mod < SDL_ALPHA_OPAQUE) {
                    SDL_SetTextureAlphaMod(tile, alpha_mod);
                }

                // render tile
                SDL_RenderCopy(renderer, tile, &srcrect, &dstrect);

                // reset alpha modulation
                if (alpha_mod < SDL_ALPHA_OPAQUE) {
                    SDL_SetTextureAlphaMod(tile, SDL_ALPHA_OPAQUE);
                }
            }
        }
    }

    // reset blend mode
    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    }
}

static void * sdl_img_loader(const char * path) { return IMG_LoadTexture(main_renderer, path); }

void Tilemap::Tilemap_Init() {
    tmx_img_load_func = (void * (*)(const char *))sdl_img_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;
}
}