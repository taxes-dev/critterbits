#pragma once
#ifndef CBFILES_HPP
#define CBFILES_HPP

#include <algorithm>
#include <string>
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

inline std::string StripFileFromPath(const std::string & file_name) {
    int index_a = file_name.find_last_of('\\');
    int index_b = file_name.find_last_of('/');
    return file_name.substr(0, std::max(index_a, index_b));
}

std::string GetExpandedPath(const std::string &);
bool ReadTextFile(const std::string & file_name, std::string ** file_contents);
}
#endif