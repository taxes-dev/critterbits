#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {

/*
 * Support functions for Tilemap::RenderMap()
 */
namespace {
void draw_object_polyline(SDL_Renderer * renderer, double ** points, double x, double y, int pointsc) {
    int i;
    for (i = 1; i < pointsc; i++) {
        SDL_RenderDrawLine(renderer, x + points[i - 1][0], y + points[i - 1][1], x + points[i][0], y + points[i][1]);
    }
}

void draw_object_polygon(SDL_Renderer * renderer, double ** points, double x, double y, int pointsc) {
    draw_object_polyline(renderer, points, x, y, pointsc);
    if (pointsc > 2) {
        SDL_RenderDrawLine(renderer, x + points[0][0], y + points[0][1], x + points[pointsc - 1][0],
                           y + points[pointsc - 1][1]);
    }
}

SDL_Color tmx_to_sdl_color(int tmx_color) {
    SDL_Color color;

    color.r = (tmx_color >> 16) & 0xFF;
    color.g = (tmx_color >> 8) & 0xFF;
    color.b = (tmx_color)&0xFF;
    color.a = SDL_ALPHA_OPAQUE;

    return color;
}

/*
 * Support functions for Tilemap::Tilemap_Init()
 */

void * sdl_img_loader(const char * path) { return IMG_LoadTexture(Engine::GetInstance().GetRenderer(), path); }
}
/*
 * End support functions
 */

Tilemap::Tilemap(std::string & map_path) : tmx_path(map_path) {
    this->draw_debug = Engine::GetInstance().config->debug.draw_map_regions;
}

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
    this->dim.w = this->map->width * this->tile_height;
    this->dim.h = this->map->height * this->tile_width;
    this->dim.x = 0;
    this->dim.y = 0;

    this->map_texture = this->RenderMap(Engine::GetInstance().GetRenderer(), scale);

    return this->map_texture != nullptr;
}

void Tilemap::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip) {
    if (this->map_texture != nullptr) {
        SDLx::SDL_RenderTextureClipped(renderer, this->map_texture, clip.source, clip.dest);
    }
}

SDL_Texture * Tilemap::RenderMap(SDL_Renderer * renderer, float scale) {
    // create texture to hold the map
    SDL_Texture * texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->dim.w, this->dim.h);

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

void Tilemap::DrawImageLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) {
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

void Tilemap::DrawMapLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) {
    // prepare blend mode if opacity less than 100%
    float op = layer->opacity;
    int alpha_mod = op * SDL_ALPHA_OPAQUE;
    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    // loop through tiles in map
    for (unsigned long i = 0; i < this->map->height; i++) {
        for (unsigned long j = 0; j < this->map->width; j++) {
            this->DrawTileOnMap(renderer, layer->content.gids[(i * this->map->width) + j], i, j, layer->offsetx,
                                layer->offsety, alpha_mod);
        }
    }

    // reset blend mode
    if (op < 1.) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    }
}

void Tilemap::DrawObjectLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) {
    SDL_Rect rect;

    tmx_object_group * object_group = layer->content.objgr;
    tmx_object * current_obj = object_group->head;

    SDL_Color obj_color = tmx_to_sdl_color(object_group->color);

    while (current_obj) {
        if (current_obj->visible) {
            if (current_obj->shape == S_TILE) {
                this->DrawTileOnMap(renderer, current_obj->gid, current_obj->x * -1, current_obj->y * -1,
                                    layer->offsetx, layer->offsety, SDL_ALPHA_OPAQUE);
            } else if (this->draw_debug) {
                // region objects are normally hidden and used as event triggers
                switch (current_obj->shape) {
                    case S_SQUARE:
                        rect.x = current_obj->x + layer->offsetx;
                        rect.y = current_obj->y + layer->offsety;
                        rect.w = current_obj->width;
                        rect.h = current_obj->height;
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        SDL_RenderDrawRect(renderer, &rect);
                        break;
                    case S_POLYGON:
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        draw_object_polygon(renderer, current_obj->points, current_obj->x + layer->offsetx,
                                            current_obj->y + layer->offsety, current_obj->points_len);
                        break;
                    case S_POLYLINE:
                        SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                        draw_object_polyline(renderer, current_obj->points, current_obj->x + layer->offsetx,
                                             current_obj->y + layer->offsety, current_obj->points_len);
                        break;
                    case S_ELLIPSE:
                        int radius_x = current_obj->width / 2, radius_y = current_obj->height / 2;
                        int center_x = current_obj->x + layer->offsetx + radius_x,
                            center_y = current_obj->y + layer->offsety + radius_y;
                        ellipseRGBA(renderer, center_x, center_y, radius_x, radius_y, obj_color.r, obj_color.g,
                                    obj_color.b, obj_color.a);
                        break;
                }
            }
        }
        current_obj = current_obj->next;
    }
}

void Tilemap::DrawTileOnMap(SDL_Renderer * renderer, unsigned int gid, int row, int col, int offsetx, int offsety,
                            int alpha_mod) {
    CB_Rect srcrect, dstrect;
    SDL_Texture * tile;
    tmx_tileset * ts;
    tmx_image * im;

    unsigned int tile_gid = gid & TMX_FLIP_BITS_REMOVAL;
    bool flip_x = false;
    bool flip_y = false;
    if (TestBitMask<unsigned int>(gid, TMX_FLIPPED_DIAGONALLY)) {
        flip_x = true;
        flip_y = true;
    } else if (TestBitMask<unsigned int>(gid, TMX_FLIPPED_HORIZONTALLY)) {
        flip_x = true;
    } else if (TestBitMask<unsigned int>(gid, TMX_FLIPPED_VERTICALLY)) {
        flip_y = true;
    }

    // if we have a tile at this position, draw it
    if (this->map->tiles[tile_gid] != NULL) {
        // source tileset and image
        ts = this->map->tiles[tile_gid]->tileset;
        im = this->map->tiles[tile_gid]->image;

        // source dimensions and position
        srcrect.x = this->map->tiles[tile_gid]->ul_x;
        srcrect.y = this->map->tiles[tile_gid]->ul_y;
        srcrect.w = ts->tile_width;
        srcrect.h = ts->tile_height;

        // destination dimensions and position
        dstrect.w = ts->tile_width;
        dstrect.h = ts->tile_height;
        // FIXME: this is a hack hack hack
        if (row < 0) {
            dstrect.x = row * -1 + offsetx;
            dstrect.y = col * -1 + offsety;
        } else {
            dstrect.x = col * ts->tile_width + offsetx;
            dstrect.y = row * ts->tile_height + offsety;
        }

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
        SDLx::SDL_RenderTextureClipped(renderer, tile, srcrect, dstrect, flip_x, flip_y);

        // reset alpha modulation
        if (alpha_mod < SDL_ALPHA_OPAQUE) {
            SDL_SetTextureAlphaMod(tile, SDL_ALPHA_OPAQUE);
        }
    }
}

void Tilemap::Tilemap_Init() {
    tmx_img_load_func = (void * (*)(const char *))sdl_img_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;
}

void Tilemap::Tilemap_Quit() {}
}