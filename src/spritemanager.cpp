#include <critterbits.h>

namespace Critterbits {
/*
* Support functions for SpriteManager::LoadQueuedSprites()
*/
static void sprite_scale_parser(void * context, const std::string & value) {
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

static YamlValueParserCollection sprite_val_parsers = {{"sprite_sheet", sprite_sheet_parser},
                                                       {"sprite_scale", sprite_scale_parser},
                                                       {"tag", tag_parser},
                                                       {"tile_height", tile_height_parser},
                                                       {"tile_offset_x", tile_offset_x_parser},
                                                       {"tile_offset_y", tile_offset_y_parser},
                                                       {"tile_width", tile_width_parser}};
/*
* End support functions
*/

std::string SpriteManager::GetSpritePath(const std::string & asset_name) {
    return Engine::GetInstance().config->asset_path + CB_SPRITE_PATH + PATH_SEP + asset_name;
}

bool SpriteManager::LoadQueuedSprites() {
    bool success = true;
    std::string sprite_path_current;
    std::string * sprite_content = nullptr;
    std::string sprite_name;

    YamlParser parser;
    parser.value_parsers = sprite_val_parsers;

    for (auto it = this->queued_sprites.begin(); it != this->queued_sprites.end();) {
        sprite_name = *it + CB_SPRITE_EXT;
        sprite_path_current = SpriteManager::GetSpritePath(sprite_name);
        LOG_INFO("SpriteManager::LoadQueuedSprites attempting to load " + sprite_path_current);

        std::shared_ptr<Sprite> new_sprite(new Sprite());
        new_sprite->sprite_name = *it;
        new_sprite->sprite_path = sprite_path_current;

        if (ReadTextFile(sprite_path_current, &sprite_content)) {
            parser.Parse(new_sprite.get(), *sprite_content);
            delete sprite_content;

            // prepend the asset path to the sprite sheet path if one was set
            if (!new_sprite->sprite_sheet_path.empty()) {
                new_sprite->sprite_sheet_path = SpriteManager::GetSpritePath(new_sprite->sprite_sheet_path);
            }

            this->sprites.push_back(new_sprite);

            // notify new sprite that it's been loaded
            new_sprite->NotifyLoaded();

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
        if (sprite == *it) {
            this->sprites.erase(it);
            break;
        }
        it++;
    }
    sprite->NotifyUnloaded();
}

}