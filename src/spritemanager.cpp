#include <critterbits.h>
#include <SDL_image.h>

namespace Critterbits {

std::string SpriteManager::GetSpritePath(const std::string & asset_name) {
    return Engine::GetInstance().config->asset_path + CB_SPRITE_PATH + PATH_SEP + asset_name;
}

std::shared_ptr<SDL_Texture> SpriteManager::GetSpriteSheet(const std::string & sprite_sheet_path) {
    auto it = this->sprite_sheets.find(sprite_sheet_path);
    if (it == this->sprite_sheets.end()) {
        LOG_INFO("SpriteManager::GetSpriteSheet attempting to load sprite sheet " + sprite_sheet_path);
        SDL_Texture * sprite_sheet = IMG_LoadTexture(Engine::GetInstance().GetRenderer(), sprite_sheet_path.c_str());
        if (sprite_sheet == nullptr) {
            LOG_SDL_ERR("SpriteManager::GetSpriteSheet unable to load sprite sheet to texture");
            return nullptr;
        }
        std::shared_ptr<SDL_Texture> sprite_sheet_ptr{sprite_sheet,
                                                      [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
        this->sprite_sheets.insert(std::make_pair(sprite_sheet_path, sprite_sheet_ptr));
        return std::move(sprite_sheet_ptr);
    } else {
        LOG_INFO("SpriteManager::GetSpriteSheet found already loaded sprite sheet " + sprite_sheet_path);
        return it->second;
    }
}

bool SpriteManager::LoadQueuedSprites() {
    bool success = true;

    for (auto it = this->queued_sprites.begin(); it != this->queued_sprites.end();) {
        std::string sprite_name{*it + CB_SPRITE_EXT};
        std::string sprite_path_current{SpriteManager::GetSpritePath(sprite_name)};
        LOG_INFO("SpriteManager::LoadQueuedSprites attempting to load " + sprite_path_current);

        Toml::TomlParser parser{sprite_path_current};
        if (parser.IsReady()) {
            std::shared_ptr<Sprite> new_sprite = std::make_shared<Sprite>();
            new_sprite->sprite_name = *it;
            new_sprite->sprite_path = sprite_path_current;

            // parse sprite features
            new_sprite->tag = parser.GetTableString("sprite.tag");
            new_sprite->sprite_sheet_path = parser.GetTableString("sprite_sheet.image");
            // prepend the asset path to the sprite sheet path if one was set
            if (!new_sprite->sprite_sheet_path.empty()) {
                new_sprite->sprite_sheet_path = this->GetSpritePath(new_sprite->sprite_sheet_path);
            }
            new_sprite->tile_height = parser.GetTableInt("sprite_sheet.tile_height");
            new_sprite->tile_width = parser.GetTableInt("sprite_sheet.tile_width");
            new_sprite->tile_offset_x = parser.GetTableInt("sprite_sheet.tile_offset_x");
            new_sprite->tile_offset_y = parser.GetTableInt("sprite_sheet.tile_offset_y");
            new_sprite->sprite_scale = parser.GetTableFloat("sprite_sheet.sprite_scale");
            std::string collide = parser.GetTableString("2d.collision");
            if (collide == "collide") {
                new_sprite->collision = CBE_COLLIDE_COLLIDE;
            } else if (collide == "trigger") {
                new_sprite->collision = CBE_COLLIDE_TRIGGER;
            }

            // notify new sprite that it's been loaded
            new_sprite->NotifyLoaded();

            this->sprites.push_back(std::move(new_sprite));

            it = this->queued_sprites.erase(it);
        } else {
            LOG_ERR("SpriteManager::LoadQueuedSprites unable to load sprite from " + sprite_path_current);
            success = false;
            it++;
        }
    }
    return success;
}

void SpriteManager::QueueSprite(const std::string & sprite_name) { this->queued_sprites.push_back(sprite_name); }

void SpriteManager::UnloadSprite(std::shared_ptr<Sprite> sprite) {
    for (auto it = this->sprites.begin(); it != this->sprites.end();) {
        if (sprite->entity_id == (*it)->entity_id) {
            this->sprites.erase(it);
            break;
        }
        it++;
    }
    sprite->NotifyUnloaded();
}
}