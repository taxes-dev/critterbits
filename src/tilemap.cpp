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

static inline unsigned int tilemap_gid_clear_flags(unsigned int gid) { return gid & TMX_FLIP_BITS_REMOVAL; }

static void tilemap_draw_layer(const tmx_map * map, const tmx_layer * layer) {
    LOG_INFO("tilemap_draw_layer called");
    unsigned long i, j;
    unsigned int gid;
    float op;
    tmx_tileset * ts;
    tmx_image * im;
    SDL_Rect srcrect, dstrect;
    SDL_Texture * tileset;
    op = layer->opacity;
    for (i = 0; i < map->height; i++) {
        for (j = 0; j < map->width; j++) {
            gid = tilemap_gid_clear_flags(layer->content.gids[(i * map->width) + j]);
            if (map->tiles[gid] != NULL) {
                ts = map->tiles[gid]->tileset;
                im = map->tiles[gid]->image;
                srcrect.x = map->tiles[gid]->ul_x;
                srcrect.y = map->tiles[gid]->ul_y;
                srcrect.w = dstrect.w = ts->tile_width;
                srcrect.h = dstrect.h = ts->tile_height;
                dstrect.x = j * ts->tile_width;
                dstrect.y = i * ts->tile_height;
                /* TODO Opacity and Flips */
                if (im) {
                    tileset = (SDL_Texture *)im->resource_image;
                } else {
                    tileset = (SDL_Texture *)ts->image->resource_image;
                }
                SDL_RenderCopy(main_renderer, tileset, &srcrect, &dstrect);
            }
        }
    }
}

static void tilemap_draw_image_layer(const tmx_image * img, int offset_x, int offset_y) {
    LOG_INFO("tilemap_draw_image_layer called");
    SDL_Rect dim;

    dim.x = offset_x;
    dim.y = offset_y;
    SDL_QueryTexture((SDL_Texture *)img->resource_image, NULL, NULL, &(dim.w), &(dim.h));

    SDL_RenderCopy(main_renderer, (SDL_Texture *)img->resource_image, NULL, &dim);
}
/*
 * End support functions
 */

Tilemap::~Tilemap() {
    if (this->map != nullptr) {
        tmx_map_free(this->map);
    }
}

bool Tilemap::CreateTextures() {
    // check to see if we already created the textures
    if (this->map_texture != nullptr) {
        return true;
    }

    // first load up the tilemap
    if (!(this->map = tmx_load(this->tmx_path.c_str()))) {
        LOG_ERR("Tilemap::CreateTextures unable to load TMX map " + tmx_errno);
        return false;
    }

    this->map_rect.w = this->map->width * this->map->tile_width;
    this->map_rect.h = this->map->height * this->map->tile_height;
    this->map_rect.x = 0;
    this->map_rect.y = 0;

    this->map_texture = this->RenderMap();

    return this->map_texture != nullptr;
}

SDL_Texture * Tilemap::RenderMap() {
    SDL_Texture * texture;
    tmx_layer * layers = this->map->ly_head;
    int w = this->map_rect.w, h = this->map_rect.h;

    if (!(texture = SDL_CreateTexture(main_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h))) {
        LOG_SDL_ERR("Tilemap::Rendermap unable to create texture for map");
        return nullptr;
    }

    SDL_SetRenderTarget(main_renderer, texture);

    tilemap_set_color(this->map->backgroundcolor);
    SDL_RenderClear(main_renderer);

    while (layers) {
        if (layers->visible) {
            int offX = layers->offsetx;
            int offY = layers->offsety;
            if (layers->type == L_OBJGR) {
                tilemap_draw_objects(layers->content.objgr);
            } else if (layers->type == L_IMAGE) {
                tilemap_draw_image_layer(layers->content.image, offX, offY);
            } else if (layers->type == L_LAYER) {
                tilemap_draw_layer(this->map, layers);
            }
        }
        layers = layers->next;
    }

    SDL_SetRenderTarget(main_renderer, NULL);
    return texture;
}

static void * sdl_img_loader(const char * path) { return IMG_LoadTexture(main_renderer, path); }

void Tilemap::Tilemap_Init() {
    tmx_img_load_func = (void * (*)(const char *))sdl_img_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;
}
}