#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>

#include <cb/assetpack.hpp>
#include <zstd.h>

#ifdef _WIN32
const char PATH_SEP = '\\';
#define PATH_SEP_STR "\\"
#else
const char PATH_SEP = '/';
#define PATH_SEP_STR "/"
#endif

#define CBUF_SIZE 1024

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
            settings.src = filename + PATH_SEP_STR;
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
}

using namespace Critterbits::AssetPack;

int main(int argc, char ** argv) {
    parse_command_line(argc, argv);
    banner();

    // create structures
    CB_AssetPackHeader header;
    header.first_resource_pos = sizeof(header);
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
    //TODO: iterate assets
    write_asset(asset_index++, pack, "cbconfig.toml", dict);
    write_asset(asset_index++, pack, "fonts/ds9.ttf", dict);
    write_asset(asset_index++, pack, "fonts/sample.ttf", dict);
    write_asset(asset_index++, pack, "gui/grey_panel.png", dict);
    write_asset(asset_index++, pack, "gui/test.toml", dict);
    write_asset(asset_index++, pack, "gui/images/trainer.png", dict);
    write_asset(asset_index++, pack, "scenes/startup.toml", dict);
    write_asset(asset_index++, pack, "scenes/maps/example1.tmx", dict);
    write_asset(asset_index++, pack, "scenes/maps/fr-tileset1.png", dict);
    write_asset(asset_index++, pack, "scenes/scripts/startup.js", dict);
    write_asset(asset_index++, pack, "sprites/oldman.toml", dict);
    write_asset(asset_index++, pack, "sprites/player.toml", dict);
    write_asset(asset_index++, pack, "sprites/firered-f.png", dict);
    write_asset(asset_index++, pack, "sprites/firered-oldman.png", dict);
    write_asset(asset_index++, pack, "sprites/scripts/oldman.js", dict);
    write_asset(asset_index++, pack, "sprites/scripts/player.js", dict);

    //TODO: need to write structures out accounting for endianness
    // write asset table
    header.table_pos = pack.tellp();
    for (auto & entry : dict) {
        pack.write(reinterpret_cast<const char *>(&entry), sizeof(CB_AssetDictEntry));
    }

    // write header
    pack.seekp(0);
    pack.write(reinterpret_cast<const char *>(&header), sizeof(CB_AssetPackHeader));

    return 0;
}