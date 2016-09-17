#include <cstdlib>
#include <cstring>

#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <Tmx.h>
#include <critterbits.hpp>

namespace Critterbits {

/*
 * Support functions for Tilemap::RenderMap()
 */
namespace {
void draw_object_polyline(SDL_Renderer * renderer, const Tmx::Polyline * polyline, double x, double y) {
    for (int i = 1; i < polyline->GetNumPoints(); i++) {
        SDL_RenderDrawLine(renderer, x + polyline->GetPoint(i - 1).x, y + polyline->GetPoint(i - 1).y,
                           x + polyline->GetPoint(i).x, y + polyline->GetPoint(i).y);
    }
}

void draw_object_polygon(SDL_Renderer * renderer, const Tmx::Polygon * polygon, double x, double y) {
    for (int i = 1; i < polygon->GetNumPoints(); i++) {
        SDL_RenderDrawLine(renderer, x + polygon->GetPoint(i - 1).x, y + polygon->GetPoint(i - 1).y,
                           x + polygon->GetPoint(i).x, y + polygon->GetPoint(i).y);
    }
    if (polygon->GetNumPoints() > 2) {
        SDL_RenderDrawLine(renderer, x + polygon->GetPoint(0).x, y + polygon->GetPoint(0).y,
                           x + polygon->GetPoint(polygon->GetNumPoints() - 1).x,
                           y + polygon->GetPoint(polygon->GetNumPoints() - 1).y);
    }
}

SDL_Color tmx_to_sdl_color(const std::string & tmx_color) {
    SDL_Color color{0, 0, 0, 0};

    if (!tmx_color.empty()) {
        // format is #RRGGBB[AA]
        int tmx_color_val = std::strtol(tmx_color.c_str() + 1, NULL, 16);
        if (tmx_color.length() == 7) {
            tmx_color_val = (tmx_color_val << 8) | 0xFF;
        }
        color.r = (tmx_color_val >> 24) & 0xFF;
        color.g = (tmx_color_val >> 16) & 0xFF;
        color.b = (tmx_color_val >> 8) & 0xFF;
        color.a = tmx_color_val & 0xFF;
    }

    return color;
}
}
/*
 * End support functions
 */

Tilemap::Tilemap(const std::string & map_path) : tmx_path(map_path) {
    this->draw_debug = Engine::GetInstance().config->debug.draw_map_regions;
}

Tilemap::~Tilemap() { SDLx::SDL_CleanUp(this->bg_map_texture, this->fg_map_texture); }

void Tilemap::CreateCollisionRegion(const CB_Rect & dim) {
    std::shared_ptr<TilemapRegion> region{std::make_shared<TilemapRegion>()};
    region->dim.x = dim.x * this->render_scale;
    region->dim.y = dim.y * this->render_scale;
    region->dim.w = dim.w * this->render_scale;
    region->dim.h = dim.h * this->render_scale;
    region->collision = CollisionType::Collide;
    this->regions.push_back(std::move(region));
}

bool Tilemap::CreateTextures(float scale) {
    // check to see if we already created the textures
    if (this->bg_map_texture != nullptr && this->fg_map_texture != nullptr) {
        return true;
    }

    // first load up the tilemap
    this->map.reset(new Tmx::Map());
    map->ParseFile(this->tmx_path);
    if (this->map->HasError()) {
        LOG_ERR("Tilemap::CreateTextures unable to load TMX map " + this->map->GetErrorText());
        return false;
    }
    if (this->map->GetOrientation() != Tmx::TMX_MO_ORTHOGONAL) {
        LOG_ERR("Tilemap::CreateTextures non-orthogonal maps are not supported");
        return false;
    }

    this->tile_height = this->map->GetTileHeight() * scale;
    this->tile_width = this->map->GetTileWidth() * scale;
    this->dim.w = NextPowerOf2(this->map->GetWidth() * this->tile_height);
    this->dim.h = NextPowerOf2(this->map->GetHeight() * this->tile_width);
    this->dim.x = 0;
    this->dim.y = 0;
    this->render_scale = scale;
    this->bg_color = tmx_to_sdl_color(this->map->GetBackgroundColor());

    return this->RenderMap(Engine::GetInstance().GetRenderer(), scale);
}

void Tilemap::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip) {
    SDL_Texture * texture = nullptr;
    if (clip.z_index == ZIndex::Background && this->bg_map_texture != nullptr) {
        texture = this->bg_map_texture;
    } else if (clip.z_index == ZIndex::Foreground && this->fg_map_texture != nullptr) {
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
    SDL_Texture * fg_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->dim.w, this->dim.h);
    if (fg_texture == nullptr) {
        LOG_SDL_ERR("Tilemap::RenderMap unable to create foreground texture for map");
        return false;
    }
    SDL_Texture * bg_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->dim.w, this->dim.h);
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
    SDL_SetRenderDrawColor(renderer, this->bg_color.r, this->bg_color.g, this->bg_color.b, this->bg_color.a);
    SDL_RenderClear(renderer);

    // clear foreground texture to transparent
    SDL_SetRenderTarget(renderer, fg_texture);
    SDL_SetTextureBlendMode(fg_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // iterate layers and draw
    for (auto & current_layer : this->map->GetLayers()) {
        if (current_layer->IsVisible()) {
            // check layer properties for additional features
            bool is_collide = false, is_foreground = false;
            Tmx::PropertySet properties = current_layer->GetProperties();
            if (properties.GetStringProperty(CB_TILEMAP_COLLIDE) == CB_TILEMAP_TMX_PROP_BOOL_TRUE) {
                is_collide = true;
            }
            if (properties.GetStringProperty(CB_TILEMAP_FOREGROUND) == CB_TILEMAP_TMX_PROP_BOOL_TRUE) {
                is_foreground = true;
            }

            if (is_foreground) {
                SDL_SetRenderTarget(renderer, fg_texture);
                current_texture = fg_texture;
            } else {
                SDL_SetRenderTarget(renderer, bg_texture);
                current_texture = bg_texture;
            }
            SDL_RenderSetScale(renderer, scale, scale);

            switch (current_layer->GetLayerType()) {
                case Tmx::TMX_LAYERTYPE_TILE:
                    this->DrawMapLayer(renderer, current_texture, static_cast<Tmx::TileLayer *>(current_layer),
                                       is_collide ? &collision_regions : nullptr);
                    break;
                case Tmx::TMX_LAYERTYPE_OBJECTGROUP:
                    this->DrawObjectLayer(renderer, current_texture, static_cast<Tmx::ObjectGroup *>(current_layer));
                    break;
                case Tmx::TMX_LAYERTYPE_IMAGE_LAYER:
                    this->DrawImageLayer(renderer, current_texture, static_cast<Tmx::ImageLayer *>(current_layer));
                    break;
                default:
                    LOG_INFO("Tilemap::RenderMap encountered unknown layer type in TMX file");
                    break;
            }
        }
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

void Tilemap::DrawImageLayer(SDL_Renderer * renderer, SDL_Texture * texture, const Tmx::ImageLayer * layer) {
    SDL_Rect dim;
    const Tmx::Image * image = layer->GetImage();
    if (image == nullptr) {
        return;
    }

    std::shared_ptr<SDL_Texture> source_image =
        TilesetImageManager::GetInstance().GetTilesetImage(this->tmx_path, image->GetSource());
    float op = layer->GetOpacity();

    dim.x = layer->GetOffsetX();
    dim.y = layer->GetOffsetY();
    SDL_QueryTexture(source_image.get(), NULL, NULL, &(dim.w), &(dim.h));

    if (op < 1.) {
        SDL_SetTextureAlphaMod(source_image.get(), op * SDL_ALPHA_OPAQUE);
    }
    SDL_RenderCopy(renderer, source_image.get(), NULL, &dim);
    if (op < 1.) {
        SDL_SetTextureAlphaMod(source_image.get(), SDL_ALPHA_OPAQUE);
    }
}

void Tilemap::DrawMapLayer(SDL_Renderer * renderer, SDL_Texture * texture, const Tmx::TileLayer * layer,
                           RectRegionCombiner * collision_regions) {
    // set layer opacity
    int alpha_mod = layer->GetOpacity() * SDL_ALPHA_OPAQUE;

    // loop through tiles in map
    struct MapTileInfo tile_info;
    tile_info.offsetx = layer->GetOffsetX();
    tile_info.offsety = layer->GetOffsetY();
    tile_info.alpha_mod = alpha_mod;
    for (int i = 0; i < this->map->GetHeight(); i++) {
        for (int j = 0; j < this->map->GetWidth(); j++) {
            tile_info.row = i;
            tile_info.col = j;
            this->DrawTileOnMap(renderer, layer->GetTile(j, i), tile_info, collision_regions);
        }
    }
}

void Tilemap::DrawObjectLayer(SDL_Renderer * renderer, SDL_Texture * texture, const Tmx::ObjectGroup * object_group) {
    SDL_Rect rect;
    SDL_Color obj_color;
    if (object_group->GetColor().empty()) {
        obj_color = {255, 0, 0, SDL_ALPHA_OPAQUE};
    } else {
        obj_color = tmx_to_sdl_color(object_group->GetColor());
    }

    // for S_TILE objects
    struct MapTileInfo tile_info;
    tile_info.offsetx = object_group->GetOffsetX();
    tile_info.offsety = object_group->GetOffsetY();
    tile_info.alpha_mod = object_group->GetOpacity() * SDL_ALPHA_OPAQUE;

    for (auto & current_obj : object_group->GetObjects()) {
        if (current_obj->IsVisible()) {
            if (current_obj->GetGid() > 0) {
                tile_info.col = current_obj->GetX() * -1;
                tile_info.row = current_obj->GetY() * -1;
                Tmx::MapTile tile{(unsigned int)current_obj->GetGid(),
                                  this->map->FindTileset((unsigned int)current_obj->GetGid())->GetFirstGid(),
                                  (unsigned int)this->map->FindTilesetIndex((unsigned int)current_obj->GetGid())};
                this->DrawTileOnMap(renderer, tile, tile_info);
            } else if (this->draw_debug) {
                // region objects are normally hidden and used as event triggers
                if (current_obj->GetPolygon() != nullptr) {
                    SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                    draw_object_polygon(renderer, current_obj->GetPolygon(), current_obj->GetX() + tile_info.offsetx,
                                        current_obj->GetY() + tile_info.offsety);
                } else if (current_obj->GetPolyline() != nullptr) {
                    SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                    draw_object_polyline(renderer, current_obj->GetPolyline(), current_obj->GetX() + tile_info.offsetx,
                                         current_obj->GetY() + tile_info.offsety);
                } else if (current_obj->GetEllipse() != nullptr) {
                    int radius_x = current_obj->GetWidth() / 2, radius_y = current_obj->GetHeight() / 2;
                    int center_x = current_obj->GetX() + tile_info.offsetx + radius_x,
                        center_y = current_obj->GetY() + tile_info.offsety + radius_y;
                    ellipseRGBA(renderer, center_x, center_y, radius_x, radius_y, obj_color.r, obj_color.g, obj_color.b,
                                obj_color.a);
                } else {
                    rect.x = current_obj->GetX() + tile_info.offsetx;
                    rect.y = current_obj->GetY() + tile_info.offsety;
                    rect.w = current_obj->GetWidth();
                    rect.h = current_obj->GetHeight();
                    SDL_SetRenderDrawColor(renderer, obj_color.r, obj_color.g, obj_color.b, obj_color.a);
                    SDL_RenderDrawRect(renderer, &rect);
                }
            }
        }
    }
}

void Tilemap::DrawTileOnMap(SDL_Renderer * renderer, const Tmx::MapTile & tile, const MapTileInfo & tile_info,
                            RectRegionCombiner * collision_regions) {
    CB_Rect srcrect, dstrect;

    // if we have a tile at this position, draw it
    const Tmx::Tileset * tiles = this->map->FindTileset(tile.gid);
    if (tiles != nullptr) {
        // get image and calculate tile offsets
        const Tmx::Image * im = tiles->GetImage();
        if (im == nullptr) {
            return;
        }
        int tileset_width = im->GetWidth() - (2 * tiles->GetMargin()) + tiles->GetSpacing();
        int tiles_x_count = tileset_width / (tiles->GetTileWidth() + tiles->GetSpacing());
        int tx = tile.id % tiles_x_count;
        int ty = tile.id / tiles_x_count;

        // source dimensions and position
        srcrect.x = tiles->GetMargin() + (tx * tiles->GetTileWidth()) + (tx * tiles->GetSpacing());
        srcrect.y = tiles->GetMargin() + (ty * tiles->GetTileHeight()) + (ty * tiles->GetSpacing());
        srcrect.w = tiles->GetTileWidth();
        srcrect.h = tiles->GetTileHeight();

        // destination dimensions and position
        dstrect.w = tiles->GetTileWidth();
        dstrect.h = tiles->GetTileHeight();
        // FIXME: this is a hack hack hack
        if (tile_info.row < 0) {
            dstrect.x = tile_info.col * -1 + tile_info.offsetx;
            dstrect.y = tile_info.row * -1 + tile_info.offsety;
        } else {
            dstrect.x = tile_info.col * tiles->GetTileWidth() + tile_info.offsetx;
            dstrect.y = tile_info.row * tiles->GetTileHeight() + tile_info.offsety;
        }

        // select source image
        std::shared_ptr<SDL_Texture> tileset_image =
            TilesetImageManager::GetInstance().GetTilesetImage(this->tmx_path, im->GetSource());
        if (tileset_image != nullptr) {

            // set alpha modulation based on layer opacity
            if (tile_info.alpha_mod < SDL_ALPHA_OPAQUE) {
                SDL_SetTextureAlphaMod(tileset_image.get(), tile_info.alpha_mod);
            }

            // render tile
            double rotate = tile.flippedDiagonally ? -90. : 0.;
            SDLx::SDL_RenderTextureClipped(renderer, tileset_image.get(), srcrect, dstrect, tile.flippedHorizontally,
                                           tile.flippedVertically, rotate);

            // reset alpha modulation
            if (tile_info.alpha_mod < SDL_ALPHA_OPAQUE) {
                SDL_SetTextureAlphaMod(tileset_image.get(), SDL_ALPHA_OPAQUE);
            }
        }

        // create collision region if needed
        if (collision_regions != nullptr) {
            collision_regions->regions.push_back(dstrect);
        }
    }
}
}