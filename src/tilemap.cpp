#include <cstring>

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

Tilemap::Tilemap(const std::string & map_path) : tmx_path(map_path) {
    this->draw_debug = Engine::GetInstance().config->debug.draw_map_regions;
}

Tilemap::~Tilemap() {
    if (this->map != nullptr) {
        tmx_map_free(this->map);
    }
}

void Tilemap::CreateCollisionRegion(const CB_Rect & dim) {
    std::shared_ptr<TilemapRegion> region{std::make_shared<TilemapRegion>()};
    region->dim.x = dim.x * this->render_scale;
    region->dim.y = dim.y * this->render_scale;
    region->dim.w = dim.w * this->render_scale;
    region->dim.h = dim.h * this->render_scale;
    region->collision = CBE_COLLIDE_COLLIDE;
    this->regions.push_back(std::move(region));
}

bool Tilemap::CreateTextures(float scale) {
    // check to see if we already created the textures
    if (this->bg_map_texture != nullptr && this->fg_map_texture != nullptr) {
        return true;
    }

    // first load up the tilemap
    if (!(this->map = tmx_load(this->tmx_path.c_str()))) {
        LOG_ERR("Tilemap::CreateTextures unable to load TMX map " + tmx_errno);
        return false;
    }

    this->tile_height = this->map->tile_height * scale;
    this->tile_width = this->map->tile_width * scale;
    this->dim.w = NextPowerOf2(this->map->width * this->tile_height);
    this->dim.h = NextPowerOf2(this->map->height * this->tile_width);
    this->dim.x = 0;
    this->dim.y = 0;
    this->render_scale = scale;

    return this->RenderMap(Engine::GetInstance().GetRenderer(), scale);
}

void Tilemap::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip) {
    Entity::Render(renderer, clip);
    SDL_Texture * texture = nullptr;
    if (clip.z_index == CBE_Z_BACKGROUND && this->bg_map_texture != nullptr) {
        texture = this->bg_map_texture;
    } else if (clip.z_index == CBE_Z_FOREGROUND && this->fg_map_texture != nullptr) {
        texture = this->fg_map_texture;
    }
    if (texture != nullptr) {
        SDLx::SDL_RenderTextureClipped(renderer, texture, clip.source, clip.dest);
    }
}

bool Tilemap::RenderMap(SDL_Renderer * renderer, float scale) {
    // create texture to hold the map
    int max_w = Engine::GetInstance().GetMaxTextureWidth();
    int max_h = Engine::GetInstance().GetMaxTextureHeight();
    if (max_w < this->dim.w || max_h < this->dim.h) {
        LOG_ERR("Tilemap::RenderMap map size would result in over-sized texture (" + std::to_string(this->dim.w) + "x" +
                std::to_string(this->dim.h) + ")");
        return false;
    }

    SDL_Texture * current_texture = nullptr;
    SDL_Texture * fg_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->dim.w, this->dim.h);
    if (fg_texture == nullptr) {
        LOG_SDL_ERR("Tilemap::RenderMap unable to create foreground texture for map");
        return false;
    }
    SDL_Texture * bg_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->dim.w, this->dim.h);
    if (bg_texture == nullptr) {
        SDLx::SDL_CleanUp(fg_texture);
        LOG_SDL_ERR("Tilemap::RenderMap unable to create background texture for map");
        return false;
    }

    // used for creating collision regions
    RectRegionCombiner collision_regions;

    // set render target to the map texture
    float original_scale_x, original_scale_y;
    SDL_BlendMode original_blend_mode;
    SDL_RenderGetScale(renderer, &original_scale_x, &original_scale_y);
    SDL_GetRenderDrawBlendMode(renderer, &original_blend_mode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // clear background texture to map background color
    SDL_SetRenderTarget(renderer, bg_texture);
    SDL_SetTextureBlendMode(bg_texture, SDL_BLENDMODE_BLEND);
    SDL_Color bg_color = tmx_to_sdl_color(this->map->backgroundcolor);
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(renderer);

    // clear foreground texture to transparent
    SDL_SetRenderTarget(renderer, fg_texture);
    SDL_SetTextureBlendMode(fg_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // iterate layers and draw
    tmx_layer * current_layer = this->map->ly_head;
    while (current_layer) {
        if (current_layer->visible) {
                // check layer properties for additional features
                bool is_collide = false, is_foreground = false;
                tmx_property * prop = current_layer->properties;
                while (prop) {
                    if (strcmp(prop->name, CB_TILEMAP_COLLIDE) == 0 &&
                        strcmp(prop->value, CB_TILEMAP_TMX_PROP_BOOL_TRUE) == 0) {
                        is_collide = true;
                    } else if (strcmp(prop->name, CB_TILEMAP_FOREGROUND) == 0 &&
                        strcmp(prop->value, CB_TILEMAP_TMX_PROP_BOOL_TRUE) == 0) {
                        is_foreground = true;
                    }
                    prop = prop->next;
                }

                if (is_foreground) {
                    SDL_SetRenderTarget(renderer, fg_texture);
                    current_texture = fg_texture;
                } else {
                    SDL_SetRenderTarget(renderer, bg_texture);
                    current_texture = bg_texture;
                }
                    SDL_RenderSetScale(renderer, scale, scale);

            if (current_layer->type == L_OBJGR) {
                this->DrawObjectLayer(renderer, current_texture, current_layer);
            } else if (current_layer->type == L_IMAGE) {
                this->DrawImageLayer(renderer, current_texture, current_layer);
            } else if (current_layer->type == L_LAYER) {
                this->DrawMapLayer(renderer, current_texture, current_layer, is_collide ? &collision_regions : nullptr);
            }
        }
        current_layer = current_layer->next;
    }

    // reset render target
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawBlendMode(renderer, original_blend_mode);
    SDL_RenderSetScale(renderer, original_scale_x, original_scale_y);

    // finally, create actual collision map regions
    collision_regions.Combine();
    for (CB_Rect & region : collision_regions.regions) {
        this->CreateCollisionRegion(region);
    }

    this->bg_map_texture = bg_texture;
    this->fg_map_texture = fg_texture;
    return true;
}

void Tilemap::DrawImageLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) {
    SDL_Rect dim;
    SDL_Texture * source_image = static_cast<SDL_Texture *>(layer->content.image->resource_image);
    float op = layer->opacity;

    dim.x = layer->offsetx;
    dim.y = layer->offsety;
    SDL_QueryTexture(source_image, NULL, NULL, &(dim.w), &(dim.h));

    if (op < 1.) {
        SDL_SetTextureAlphaMod(source_image, op * SDL_ALPHA_OPAQUE);
    }
    SDL_RenderCopy(renderer, source_image, NULL, &dim);
    if (op < 1.) {
        SDL_SetTextureAlphaMod(source_image, SDL_ALPHA_OPAQUE);
    }
}

void Tilemap::DrawMapLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer, RectRegionCombiner * collision_regions) {
    // prepare blend mode if opacity less than 100%
    float op = layer->opacity;
    int alpha_mod = op * SDL_ALPHA_OPAQUE;

    // loop through tiles in map
    struct MapTile tile;
    tile.offsetx = layer->offsetx;
    tile.offsety = layer->offsety;
    tile.alpha_mod = alpha_mod;
    for (unsigned long i = 0; i < this->map->height; i++) {
        for (unsigned long j = 0; j < this->map->width; j++) {
            tile.gid = layer->content.gids[(i * this->map->width) + j];
            tile.row = i;
            tile.col = j;
            this->DrawTileOnMap(renderer, tile, collision_regions);
        }
    }
}

void Tilemap::DrawObjectLayer(SDL_Renderer * renderer, SDL_Texture * texture, const tmx_layer * layer) {
    SDL_Rect rect;

    tmx_object_group * object_group = layer->content.objgr;
    tmx_object * current_obj = object_group->head;

    SDL_Color obj_color = tmx_to_sdl_color(object_group->color);

    // for S_TILE objects
    struct MapTile tile;
    tile.offsetx = layer->offsetx;
    tile.offsety = layer->offsety;
    tile.alpha_mod = SDL_ALPHA_OPAQUE;

    while (current_obj) {
        if (current_obj->visible) {
            if (current_obj->shape == S_TILE) {
                tile.col = current_obj->x * -1;
                tile.row = current_obj->y * -1;
                tile.gid = current_obj->gid;
                this->DrawTileOnMap(renderer, tile);
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

void Tilemap::DrawTileOnMap(SDL_Renderer * renderer, const struct MapTile & tile, RectRegionCombiner * collision_regions) {
    CB_Rect srcrect, dstrect;
    SDL_Texture * tiletex;
    tmx_tileset * ts;
    tmx_image * im;

    unsigned int tile_gid = tile.gid & TMX_FLIP_BITS_REMOVAL;
    bool flip_x = false;
    bool flip_y = false;
    double rotate = 0.;
    if (TestBitMask<unsigned int>(tile.gid, TMX_FLIPPED_DIAGONALLY)) {
        rotate = -90.;
    } 
    if (TestBitMask<unsigned int>(tile.gid, TMX_FLIPPED_HORIZONTALLY)) {
        flip_x = true;
    }
    if (TestBitMask<unsigned int>(tile.gid, TMX_FLIPPED_VERTICALLY)) {
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
        if (tile.row < 0) {
            dstrect.x = tile.row * -1 + tile.offsetx;
            dstrect.y = tile.col * -1 + tile.offsety;
        } else {
            dstrect.x = tile.col * ts->tile_width + tile.offsetx;
            dstrect.y = tile.row * ts->tile_height + tile.offsety;
        }

        // select source tile or image
        if (im) {
            tiletex = static_cast<SDL_Texture *>(im->resource_image);
        } else {
            tiletex = static_cast<SDL_Texture *>(ts->image->resource_image);
        }

        // set alpha modulation based on layer opacity
        if (tile.alpha_mod < SDL_ALPHA_OPAQUE) {
            SDL_SetTextureAlphaMod(tiletex, tile.alpha_mod);
        }

        // render tile
        SDLx::SDL_RenderTextureClipped(renderer, tiletex, srcrect, dstrect, flip_x, flip_y, rotate);

        // reset alpha modulation
        if (tile.alpha_mod < SDL_ALPHA_OPAQUE) {
            SDL_SetTextureAlphaMod(tiletex, SDL_ALPHA_OPAQUE);
        }

        // create collision region if needed
        if (collision_regions != nullptr) {
            collision_regions->regions.push_back(dstrect);
        }
    }
}

void Tilemap::Tilemap_Init() {
    tmx_img_load_func = (void * (*)(const char *))sdl_img_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;
}

void Tilemap::Tilemap_Quit() {}
}