#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <experimental/filesystem>

#include <cb/assetpack.hpp>
#include <cb/toml.hpp>
#include <zstd.h>

#ifdef _WIN32
const char PATH_SEP = '\\';
#define PATH_SEP_STR "\\"
#else
const char PATH_SEP = '/';
#define PATH_SEP_STR "/"
#endif

#define CBUF_SIZE 1024

namespace fs = std::experimental::filesystem;

namespace {
struct {
    bool compress{true};
    std::string dest{"." PATH_SEP_STR "assets.pak"};
    bool overwrite{false};
    bool quiet{false};
    bool quit_on_error{true};
    std::string src{"." PATH_SEP_STR "assets" PATH_SEP_STR};
} settings;

bool FileExists(const std::string & file_path) {
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}

inline void LogInfo(const std::string & msg) {
    if (!settings.quiet) {
        std::cout << msg << std::endl;
    }
}

inline void LogInfoNoNL(const std::string & msg) {
    if (!settings.quiet) {
        std::cout << msg;
    }
}

inline void LogError(const std::string & msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
    if (settings.quit_on_error) {
        std::exit(1);
    }
}

void banner() {
    LogInfo("Critterbits Asset Packer v" + std::to_string(CB_ASSETPACK_VER_MAJ) + "." +
            std::to_string(CB_ASSETPACK_VER_MIN));
    LogInfo("Copyright (c) 2016 Havi S.");
    LogInfo("");
}

void help() {
    // TODO
    std::exit(0);
}

void check_dest_exists() {
    if (!settings.overwrite && FileExists(settings.dest)) {
        LogError("Output file already exists");
    }
}

void parse_command_line(int argc, char ** argv) {
    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);

        if (arg == "-?" || arg == "--help") {
            help();
        } else if (arg == "-f" || arg == "--force") {
            settings.overwrite = true;
        } else if (arg == "-q" || arg == "--quiet") {
            settings.quiet = true;
        } else if (arg == "-o" || arg == "--output") {
            if (++i < argc) {
                settings.dest = std::string{argv[i]};
            } else {
                LogError("No output file specified with -o");
                std::exit(1);
            }
        } else if (arg == "--continue") {
            settings.quit_on_error = false;
        } else if (arg == "--no-compress") {
            settings.compress = false;
        } else if (arg[0] == '-') {
            LogError("Uknown paramater: " + arg);
        } else {
            std::string filename{argv[i]};
            settings.src = filename;
            if (settings.src[settings.src.length() - 1] != PATH_SEP) {
                settings.src += PATH_SEP_STR;
            }
        }
    }
}

bool should_compress(const std::string & name) {
    if (!settings.compress) {
        return false;
    }
    std::string filename{name};
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    if (filename.compare(filename.length() - 4, 4, ".png") == 0) {
        return false;
    }
    return true;
}

unsigned long copy_compressed_stream(std::ifstream & ifs, std::ofstream & ofs) {
    const size_t cbuf_size = ZSTD_compressBound(CBUF_SIZE);
    char * i_buffer = new char[CBUF_SIZE];
    char * c_buffer = new char[cbuf_size];

    unsigned long bytes_read = 0L;
    unsigned long bytes_written = 0L;
    while (!ifs.eof()) {
        ifs.read(i_buffer, CBUF_SIZE);
        const size_t csize = ZSTD_compress(c_buffer, cbuf_size, i_buffer, ifs.gcount(), 1);
        if (ZSTD_isError(csize)) {
            LogError("Compression error " + std::string{ZSTD_getErrorName(csize)});
            break;
        }
        ofs.write(c_buffer, csize);
        bytes_read += ifs.gcount();
        bytes_written += csize;
    }
    if (bytes_read > 0L) {
        LogInfo("compressed " + std::to_string(100 - static_cast<int>(static_cast<float>(bytes_written) /
                                                                static_cast<float>(bytes_read) * 100.f)) +
                "%");
    }
    delete[] c_buffer;
    delete[] i_buffer;
    return bytes_written;
}

void write_asset(unsigned long index, std::ofstream & ofs, const std::string & name,
                 std::vector<Critterbits::AssetPack::CB_AssetDictEntry> & entries) {
    std::ifstream ifs{settings.src + name, std::ifstream::binary};
    static char read_buffer[CBUF_SIZE];
    if (ifs.good()) {
        Critterbits::AssetPack::CB_AssetDictEntry dict;
        dict.index = index;
        if (name.length() > CB_ASSETPACK_MAX_NAME_SIZE) {
            LogError("Asset name " + name + " is too long! (max " + std::to_string(CB_ASSETPACK_MAX_NAME_SIZE) +
                     " chars.)");
        }
        std::strncpy(dict.name, name.c_str(), CB_ASSETPACK_MAX_NAME_SIZE);
        dict.pos = ofs.tellp();
        if (should_compress(name)) {
            LogInfoNoNL("[" + std::to_string(index + 1) + "] Writing compressed asset " + name + " ... ");
            dict.length = copy_compressed_stream(ifs, ofs);
        } else {
            LogInfo("[" + std::to_string(index + 1) + "] Writing uncompressed asset " + name);
            while (!ifs.eof()) {
                ifs.read(&read_buffer[0], CBUF_SIZE);
                dict.length += ifs.gcount();
                ofs.write(&read_buffer[0], ifs.gcount());
            }
        }
        entries.push_back(dict);
    } else {
        LogError("Asset " + name + " could not be opened for reading");
    }
}

void write_header(std::ofstream & ofs, const Critterbits::AssetPack::CB_AssetPackHeader & header) {
    Critterbits::AssetPack::CB_AssetPackHeader header_to_write{header};
    header_to_write.flags = htonl(header_to_write.flags);
    header_to_write.table_pos = htonl(header_to_write.table_pos);
    header_to_write.first_resource_pos = htonl(header_to_write.first_resource_pos);

    ofs.seekp(0);
    ofs.write(reinterpret_cast<const char *>(&header_to_write), sizeof(Critterbits::AssetPack::CB_AssetPackHeader));
}

void write_dict_entry(std::ofstream & ofs, const Critterbits::AssetPack::CB_AssetDictEntry & entry) {
    Critterbits::AssetPack::CB_AssetDictEntry entry_to_write{entry};
    entry_to_write.index = htonl(entry_to_write.index);
    entry_to_write.pos = htonl(entry_to_write.pos);
    entry_to_write.length = htonl(entry_to_write.length);

    ofs.write(reinterpret_cast<const char *>(&entry_to_write), sizeof(Critterbits::AssetPack::CB_AssetDictEntry));
}

std::vector<std::string> get_directory_entries(const std::string & path, const std::string & ext) {
    std::vector<std::string> entries;
    for (auto & dir_entry : fs::recursive_directory_iterator(path)) {
        if (ext.empty() || ext == dir_entry.path().extension()) {
            entries.push_back(dir_entry.path().string());
        }
    }
    return entries;
}

std::string strip_file_name_from_path(const std::string & file_path) {
    int index_a = file_path.find_last_of('\\');
    int index_b = file_path.find_last_of('/');
    return file_path.substr(0, std::max(index_a, index_b));
}

std::string make_relative_to(const std::string & parent_file, const std::string & current_file) {
    return strip_file_name_from_path(parent_file) + PATH_SEP_STR + current_file;
}

void discover_scenes(std::vector<std::string> & asset_names) {
    std::vector<std::string> scenes = get_directory_entries(settings.src + "scenes", ".toml");
    for (auto & scene_file : scenes) {
        std::string scene_relative_file = scene_file.substr(settings.src.length());
        asset_names.push_back(scene_relative_file);

        Critterbits::Toml::TomlParser scene_toml{scene_file};
        if (scene_toml.IsReady()) {
            std::string map = scene_toml.GetTableString("scene.map");
            if (!map.empty()) {
                asset_names.push_back(make_relative_to(scene_relative_file, map));
                // TODO: extract tileset images from .tmx
            }

            std::string script = scene_toml.GetTableString("scene.script");
            if (!script.empty()) {
                asset_names.push_back(make_relative_to(scene_relative_file, script));
            }
        } else { 
            LogError("Unable to open scene file " + scene_file + ": " + scene_toml.GetParserError());
        }
    }
}

void discover_sprites(std::vector<std::string> & asset_names) {
    std::vector<std::string> sprites = get_directory_entries(settings.src + "sprites", ".toml");
    for (auto & sprite_file : sprites) {
        std::string sprite_relative_file = sprite_file.substr(settings.src.length());
        asset_names.push_back(sprite_relative_file);

        Critterbits::Toml::TomlParser sprite_toml{sprite_file};
        if (sprite_toml.IsReady()) {
            std::string script = sprite_toml.GetTableString("sprite.script");
            if (!script.empty()) {
                asset_names.push_back(make_relative_to(sprite_relative_file, script));
            }

            std::string sprite_sheet = sprite_toml.GetTableString("sprite_sheet.image");
            if (!sprite_sheet.empty()) {
                asset_names.push_back(make_relative_to(sprite_relative_file, sprite_sheet));
            }
        } else {
            LogError("Unable to open sprite file " + sprite_file + ": " + sprite_toml.GetParserError());
        }
    }
}

void discover_gui(std::vector<std::string> & asset_names) {
    std::vector<std::string> guis = get_directory_entries(settings.src + "gui", ".toml");
    for (auto & gui_file : guis) {
        std::string gui_relative_file = gui_file.substr(settings.src.length());
        asset_names.push_back(gui_relative_file);

        Critterbits::Toml::TomlParser gui_toml{gui_file};
        if (gui_toml.IsReady()) {
            std::string decoration = gui_toml.GetTableString("decoration.image");
            if (!decoration.empty()) {
                asset_names.push_back(make_relative_to(gui_relative_file, decoration));
            }

            gui_toml.IterateTableArray("control", [&](const Critterbits::Toml::TomlParser & table) {
                if (table.GetTableString("type") == "image") {
                    std::string image = table.GetTableString("image");
                    if (!image.empty()) {
                        asset_names.push_back(make_relative_to(gui_relative_file, image));
                    }
                }
            });
        } else {
            LogError("Unable to open GUI file " + gui_file + ": " + gui_toml.GetParserError());
        }
    }
}

void discover_assets(std::vector<std::string> & asset_names) {
    // first find the root cbconfig.toml
    Critterbits::Toml::TomlParser cbconfig{settings.src + "cbconfig.toml"};
    if (cbconfig.IsReady()) {
        asset_names.push_back("cbconfig.toml");

        // icon file
        std::string icon = cbconfig.GetTableString("window.icon");
        if (!icon.empty()) {
            asset_names.push_back(icon);
        }

        // fonts
        cbconfig.IterateTableArray("font", [&asset_names](const Critterbits::Toml::TomlParser & table){
            std::string font_file = table.GetTableString("file");
            if (!font_file.empty()) {
                asset_names.push_back(font_file);
            }
        });

        // scenes
        discover_scenes(asset_names);

        // sprites
        discover_sprites(asset_names);

        // gui
        discover_gui(asset_names);
    } else {
        LogError("Unable to open cbconfig.toml: " + cbconfig.GetParserError());
    }
}
}

using namespace Critterbits::AssetPack;

#ifdef _MSC_VER
// SDL macro creates some confusion for MSVC
#undef main
#endif
int main(int argc, char ** argv) {
    parse_command_line(argc, argv);
    banner();

    // iterate list of assets to pack
    std::vector<std::string> asset_names;
    discover_assets(asset_names);

    // create structures
    CB_AssetPackHeader header;
    header.first_resource_pos = sizeof(CB_AssetPackHeader);
    if (settings.compress) {
        header.flags |= CB_ASSETPACK_FLAGS_COMPRESSED;
    }
    std::vector<CB_AssetDictEntry> dict;

    // open output pack
    check_dest_exists();
    std::ofstream pack{settings.dest, std::ofstream::binary | std::ofstream::trunc};
    pack.seekp(header.first_resource_pos);

    // write each asset and record
    unsigned long asset_index = 0L;
    for (auto & asset_name : asset_names) {
        write_asset(asset_index++, pack, asset_name, dict);
    }

    // write asset table
    header.table_pos = pack.tellp();
    for (auto & entry : dict) {
        write_dict_entry(pack, entry);
    }

    // write header
    write_header(pack, header);

    return 0;
}