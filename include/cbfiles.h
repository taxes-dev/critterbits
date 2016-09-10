#pragma once
#ifndef CBFILES_H
#define CBFILES_H

#include <sys/stat.h>
#include <string>

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
    return file_name.substr(0, file_name.find_last_of(PATH_SEP));
}


std::string GetExpandedPath(const std::string &);
bool ReadTextFile(const std::string & file_name, std::string ** file_contents);
}
#endif