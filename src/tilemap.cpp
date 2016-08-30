#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <tmx/tmx.h>

#include "cbengine.h"
#include "cblogging.h"
#include "cbsdl.h"
#include "cbtilemap.h"

namespace Critterbits {

// if set to true, object layer shapes in TMX maps will be drawn
bool cb_force_draw_map_regions = false;

/*
 * Support functions for Tilemap::RenderMap()
 */
static void draw_object_polyline(SDL_Renderer * renderer, double ** points, double x, double y, int pointsc) {
    int i;
    for (i = 1; i < pointsc; i++) {
        SDL_RenderDrawLine(renderer, x + points[i - 1][0], y + points[i - 1][1], x + points[i][0], y + points[i][1]);
    }
}

static void draw_object_polygon(SDL_Renderer * renderer, double ** points, double x, double y, int pointsc) {
    draw_object_polyline(renderer, points, x, y, pointsc);
    if (pointsc > 2) {
        SDL_RenderDrawLine(renderer, x + points[0][0], y + points[0][1], x + points[pointsc - 1][0],
                           y + points[pointsc - 1][1]);
    }
}

static SDL_Color tmx_to_sdl_color(int tmx_color) {
    SDL_Color color;

    color.r = (tmx_color >> 16) & 0xFF;
    color.g = (tmx_color >> 8) & 0xFF;
    color.b = (tmx_color)&0xFF;
    color.a = SDL_ALPHA_OPAQUE;

    return color;
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

    this->map_texture = this->RenderMap(cb_main_renderer, scale);

    return this->map_texture != nullptr;
}

SDL_Texture * Tilemap::RenderMap(SDL_Renderer * renderer, float scale) const {
    // create texture to hold the map
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                              this->map_rect.w, this->map_rect.h);

    if (texture == nullptr) {
        LOG_SDL_ERR("Tilemap::Rendermap unable to create texture for map");
        return nullptr;
    }

    // set render target to the map texture
    float original_scale_x, original_scale_y;
    SDL_RenderGetScale(renderer, &original_scale_x, &original_scale_y);
    SDL_SetRenderTarget(renderer, texture);
    SDL_RenderSetScale(renderer, scale, scale);

    // clear map to background color
    SDL_Color bg_color = tmx_to_sdl_color(this->map->backgroundcolor);
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(renderer);

    // iterate layers and draw
    tmx_layer * current_layer = this->map->ly_head;
    while (current_layer) {
        if (current_layer->visible) {
            if (current_layer->type == L_OBJGR) {
                this->DrawObjectLayer(renderer, texture, current_layer);
            } else if (current_layer->type == L_IMAGE) {
                this->DrawImageLayer(renderer, texture, current_layer);
            } else if (current_layer->type == L_LAYER) {
                this->DrawMapLayer(renderer, texture, current_layer);
            }
        }
        current_layer = current_layer->next;
    }

    // reset render target
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderSetScale(renderer, original_scale_x, original_scale_y);

    return texture;
}

void Tilemap::DrawImageLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) const {
    SDL_Rect dim;
    SDL_Texture * source_image = static_cast<SDL_Texture *>(layer->content.image->resource_image);
    float op = layer->opacity;

    dim.x = layer->offsetx;
    dim.y = layer->offsety;
    SDL_QueryTexture(source_image, NULL, NULL, &(dim.w), &(dim.h));

    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(source_image, op * SDL_ALPHA_OPAQUE);
    }
    SDL_RenderCopy(renderer, source_image, NULL, &dim);
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

                // destination dimensions and position
                dstrect.w = ts->tile_width;
                dstrect.h = ts->tile_height;
                dstrect.x = j * ts->tile_width;
                dstrect.y = i * ts->tile_height;

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

void Tilemap::DrawObjectLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) const {
    SDL_Rect rect;

    tmx_object_group * object_group = layer->content.objgr;
    tmx_object * current_obj = object_group->head;

    SDL_Color obj_color = tmx_to_sdl_color(object_group->color);

    while (current_obj) {
        if (current_obj->visible) {
            if (current_obj->shape == S_TILE) {
                // TODO: draw tile
            } else if (cb_force_draw_map_regions) {
                switch (current_obj->shape) {
                    case S_SQUARE:
                        rect.x = current_obj->x;
                        rect.y = current_obj->y;
                        rect.w = current_obj->width;
                        rect.h = current_obj->height;
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        SDL_RenderDrawRect(renderer, &rect);
                        break;
                    case S_POLYGON:
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        draw_object_polygon(renderer, current_obj->points, current_obj->x, current_obj->y,
                                            current_obj->points_len);
                        break;
                    case S_POLYLINE:
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        draw_object_polyline(renderer, current_obj->points, current_obj->x, current_obj->y,
                                             current_obj->points_len);
                        break;
                    case S_ELLIPSE:
                        int radius_x = current_obj->width / 2, radius_y = current_obj->height / 2;
                        int center_x = current_obj->x + radius_x, center_y = current_obj->y + radius_y;
                        ellipseRGBA(renderer, center_x, center_y, radius_x, radius_y, obj_color.r, obj_color.g,
                                    obj_color.b, obj_color.a);
                        break;
                }
            }
        }
        current_obj = current_obj->next;
    }
}

static void * sdl_img_loader(const char * path) { return IMG_LoadTexture(cb_main_renderer, path); }

void Tilemap::Tilemap_Init(bool draw_map_regions) {
    cb_force_draw_map_regions = draw_map_regions;
    tmx_img_load_func = (void * (*)(const char *))sdl_img_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;
}
}