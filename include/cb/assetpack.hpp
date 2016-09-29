#pragma once
#ifndef CB_ASSETPACK_HPP
#define CB_ASSETPACK_HPP
namespace Critterbits{
namespace AssetPack {

#define CB_ASSETPACK_VER_MAJ 1
#define CB_ASSETPACK_VER_MIN 0
#define CB_ASSETPACK_HDR_BYTES 0xef, 0xbb, 0xbf, 'c', 'b', 'p', 'a', 'k', 0xe2, 0x90, 0x84, 0
#define CB_ASSETPACK_MAX_NAME_SIZE 256

#define CB_ASSETPACK_FLAGS_NONE 0
#define CB_ASSETPACK_FLAGS_COMPRESSED 1

typedef struct CB_AssetPackHeader {
    const unsigned char _header[12]{CB_ASSETPACK_HDR_BYTES};
    const unsigned char _version[4]{CB_ASSETPACK_VER_MAJ, CB_ASSETPACK_VER_MIN, 0, 0};
    unsigned int flags{CB_ASSETPACK_FLAGS_NONE};
    unsigned long table_pos{0L};
    unsigned long first_resource_pos{0L};
} CB_AssetPackHeader;

typedef struct CB_AssetDictEntry {
    unsigned long index{0L};
    char name[CB_ASSETPACK_MAX_NAME_SIZE];
    unsigned long pos{0L};
    unsigned long length{0L};
} CB_AssetDictEntry;

}
}
#endif