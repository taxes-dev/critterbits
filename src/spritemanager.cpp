#include <cb/critterbits.hpp>

namespace Critterbits {

std::string SpriteManager::GetSpritePath(const std::string & asset_name) const {
    return CB_SPRITE_PATH PATH_SEP_STR + asset_name + CB_SPRITE_EXT;
}

std::string SpriteManager::GetSpriteSheetPath(const std::string & asset_name) const {
    return CB_SPRITE_PATH PATH_SEP_STR + asset_name;
}

std::shared_ptr<SDL_Texture> SpriteManager::GetSpriteSheet(const std::string & sprite_sheet_path) {
    auto it = this->sprite_sheets.find(sprite_sheet_path);
    if (it == this->sprite_sheets.end()) {
        LOG_INFO("SpriteManager::GetSpriteSheet attempting to load sprite sheet " + sprite_sheet_path);
        std::shared_ptr<SDL_Texture> sprite_sheet = Engine::GetInstance().GetResourceLoader()->GetImageResource(sprite_sheet_path);
        if (sprite_sheet == nullptr) {
            LOG_ERR("SpriteManager::GetSpriteSheet unable to load sprite sheet to texture");
            return nullptr;
        }
        this->sprite_sheets.insert(std::make_pair(sprite_sheet_path, sprite_sheet));
        return std::move(sprite_sheet);
    } else {
        LOG_INFO("SpriteManager::GetSpriteSheet found already loaded sprite sheet " + sprite_sheet_path);
        return it->second;
    }
}

bool SpriteManager::LoadQueuedSprites() {
    bool success = true;

    for (auto it = this->queued_sprites.begin(); it != this->queued_sprites.end();) {
        const QueuedSprite & qsprite = *it;
        std::string sprite_name{qsprite.name};
        std::string sprite_path_current{this->GetSpritePath(sprite_name)};
        LOG_INFO("SpriteManager::LoadQueuedSprites attempting to load " + sprite_path_current);

        auto sprite_file = Engine::GetInstance().GetResourceLoader()->OpenTextResource(sprite_path_current);
        Toml::TomlParser parser{sprite_file};
        if (parser.IsReady()) {
            std::shared_ptr<Sprite> new_sprite = std::make_shared<Sprite>();
            new_sprite->sprite_name = qsprite.name;
            new_sprite->dim.x = qsprite.at.x;
            new_sprite->dim.y = qsprite.at.y;
            new_sprite->sprite_path = sprite_path_current;

            // parse sprite features
            this->ParseSprite(parser, new_sprite);

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

void SpriteManager::ParseSprite(const Toml::TomlParser & parser, std::shared_ptr<Sprite> sprite) const {
    sprite->tag = parser.GetTableString("sprite.tag");
    sprite->script_path = parser.GetTableString("sprite.script");
    if (!sprite->script_path.empty()) {
        sprite->script_path = this->GetSpriteSheetPath(sprite->script_path);
    }
    sprite->sprite_sheet_path = parser.GetTableString("sprite_sheet.image");
    // prepend the asset path to the sprite sheet path if one was set
    if (!sprite->sprite_sheet_path.empty()) {
        sprite->sprite_sheet_path = this->GetSpriteSheetPath(sprite->sprite_sheet_path);
    }
    sprite->tile_height = parser.GetTableInt("sprite_sheet.tile_height");
    sprite->tile_width = parser.GetTableInt("sprite_sheet.tile_width");
    sprite->tile_offset_x = parser.GetTableInt("sprite_sheet.tile_offset_x");
    sprite->tile_offset_y = parser.GetTableInt("sprite_sheet.tile_offset_y");
    sprite->sprite_scale = parser.GetTableFloat("sprite_sheet.sprite_scale");
    std::string collide = parser.GetTableString("2d.collision");
    if (collide == "collide") {
        sprite->collision = CollisionType::Collide;
    } else if (collide == "trigger") {
        sprite->collision = CollisionType::Trigger;
    }
    // animations
    parser.IterateTableArray("animation", [&sprite](const Toml::TomlParser & table) {
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
            sprite->animations.push_back(std::move(anim));
        }
    });
}

void SpriteManager::QueueSprite(const QueuedSprite & queued_sprite) {
    this->queued_sprites.push_back(queued_sprite);
    if (!this->new_sprites) {
        this->new_sprites = true;
        EngineEventQueue::GetInstance().QueuePreUpdate(/*(PreUpdateEvent)*/[this]() {
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