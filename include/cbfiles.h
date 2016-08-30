#pragma once
#ifndef CBFILES_H
#define CBFILES_H

#ifdef _WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

namespace Critterbits {
bool ReadTextFile(const std::string & file_name, std::string ** file_contents);
}
#endif