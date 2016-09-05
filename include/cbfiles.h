#pragma once
#ifndef CBFILES_H
#define CBFILES_H

#include <sys/stat.h>

#ifdef _WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

namespace Critterbits {
inline bool FileExists(const std::string & file_path) {
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}

bool ReadTextFile(const std::string & file_name, std::string ** file_contents);
}
#endif