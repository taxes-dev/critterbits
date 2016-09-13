#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {
/*
* Support functions for SpriteManager::LoadQueuedSprites()
*/
/*static void sprite_scale_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->sprite_scale = YamlParser::ToFloat(value);
}

static void sprite_sheet_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->sprite_sheet_path = value;
}

static void tag_parser(void * context, const std::string & value) { static_cast<Sprite *>(context)->tag = value; }

static void tile_height_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->tile_height = YamlParser::ToInt(value);
}

static void tile_offset_x_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->tile_offset_x = YamlParser::ToInt(value);
}

static void tile_offset_y_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->tile_offset_y = YamlParser::ToInt(value);
}

static void tile_width_parser(void * context, const std::string & value) {
    static_cast<Sprite *>(context)->tile_width = YamlParser::ToInt(value);
}

static void collision_parser(void * context, const std::string & value) {
    CollisionType collision{CBE_COLLIDE_NONE};
    if (value == "collide") {
        collision = CBE_COLLIDE_COLLIDE;
    } else if (value == "trigger") {
        collision = CBE_COLLIDE_TRIGGER;
    }
    static_cast<Sprite *>(context)->collision = collision;
}

static YamlValueParserCollection sprite_val_parsers = {{"sprite_sheet", sprite_sheet_parser},
                                                       {"sprite_scale", sprite_scale_parser},
                                                       {"tag", tag_parser},
                                                       {"tile_height", tile_height_parser},
                                                       {"tile_offset_x", tile_offset_x_parser},
                                                       {"tile_offset_y", tile_offset_y_parser},
                                                       {"tile_width", tile_width_parser},
                                                       {"collision", collision_parser}};
                                                       */
/*
* End support functions
*/

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
    std::string sprite_path_current;
    std::string * sprite_content = nullptr;
    std::string sprite_name;

    /*YamlParser parser;
    parser.value_parsers = sprite_val_parsers;

    for (auto it = this->queued_sprites.begin(); it != this->queued_sprites.end();) {
        sprite_name = *it + CB_SPRITE_EXT;
        sprite_path_current = SpriteManager::GetSpritePath(sprite_name);
        LOG_INFO("SpriteManager::LoadQueuedSprites attempting to load " + sprite_path_current);

        std::shared_ptr<Sprite> new_sprite = std::make_shared<Sprite>();
        new_sprite->sprite_name = *it;
        new_sprite->sprite_path = sprite_path_current;

        if (ReadTextFile(sprite_path_current, &sprite_content)) {
            parser.Parse(new_sprite.get(), *sprite_content);
            delete sprite_content;

            // prepend the asset path to the sprite sheet path if one was set
            if (!new_sprite->sprite_sheet_path.empty()) {
                new_sprite->sprite_sheet_path = SpriteManager::GetSpritePath(new_sprite->sprite_sheet_path);
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
    }*/
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