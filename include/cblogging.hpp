#pragma once
#ifndef CBLOGGING_HPP
#define CBLOGGING_HPP

#include <SDL.h>
#include <iostream>

#if (NDEBUG && !WIN32)
#define LOG_INFO(m)
#else
#define LOG_INFO(m) Critterbits::LogInfo(std::cout, m)
#endif

#define LOG_ERR(m) Critterbits::LogError(std::cerr, m)
#define LOG_SDL_ERR(m) Critterbits::LogSdlError(std::cerr, m)

namespace Critterbits {

inline void LogInfo(std::ostream & os, const std::string & msg) { os << "[INFO] " << msg << std::endl; }

inline void LogError(std::ostream & os, const std::string & msg) { os << "[ERROR] " << msg << std::endl; }

inline void LogSdlError(std::ostream & os, const std::string & msg) {
    os << "[SDL ERROR] " << msg << " " << SDL_GetError() << std::endl;
}
}
#endif