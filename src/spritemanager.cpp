#include <cb/critterbits.hpp>
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
        const QueuedSprite & qsprite = *it;
        std::string sprite_name{qsprite.name + CB_SPRITE_EXT};
        std::string sprite_path_current{SpriteManager::GetSpritePath(sprite_name)};
        LOG_INFO("SpriteManager::LoadQueuedSprites attempting to load " + sprite_path_current);

        Toml::TomlParser parser{sprite_path_current};
        if (parser.IsReady()) {
            std::shared_ptr<Sprite> new_sprite = std::make_shared<Sprite>();
            new_sprite->sprite_name = qsprite.name;
            new_sprite->dim.x = qsprite.at.x;
            new_sprite->dim.y = qsprite.at.y;
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
                new_sprite->collision = CollisionType::Collide;
            } else if (collide == "trigger") {
                new_sprite->collision = CollisionType::Trigger;
            }
            // animations
            parser.IterateTableArray("animation", [&new_sprite](const Toml::TomlParser & table) {
                std::string animation_name = table.GetTableString("name");
                if (!animation_name.empty()) {
                    std::shared_ptr<Animation> anim = std::make_shared<Animation>(animation_name);
                    anim->loop = table.GetTableBool("loop");
                    table.IterateTableArray("frames", [&anim](const Toml::TomlParser & table) {
                        KeyFrame key_frame{
                            table.GetTableString("prop"),
                            table.GetTableString("val"),
                            table.GetTableInt("dur")
                        };
                        if (key_frame.property.empty()) {
                            LOG_ERR("SpriteManager::LoadQueuedSprites animation key frame must have a property");
                            return;
                        }
                        if (key_frame.duration < 0) {
                            LOG_ERR("SpriteManager::LoadQueuedSprites animation key frame cannot have duration less than zero");
                            return;
                        }
                        anim->AddKeyFrame(key_frame);
                    });
                    if (table.GetTableBool("auto_play")) {
                        anim->Play();
                    }
                    new_sprite->animations.push_back(std::move(anim));
                }
            });

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

    if (success) {
        this->new_sprites = false;
    }

    return success;
}

void SpriteManager::QueueSprite(const QueuedSprite & queued_sprite) {
    this->queued_sprites.push_back(queued_sprite);
    if (!this->new_sprites) {
        this->new_sprites = true;
        EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
            if (!this->LoadQueuedSprites()) {
                LOG_ERR("SpriteManager::QueueSprite(pre-update) unable to load queued sprites");
            }
        });
    }
}

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