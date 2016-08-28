#pragma once
#ifndef ERRORS_H
#define ERRORS_H

#include <SDL.h>
#include <iostream>

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
inline void logSDLError(std::ostream & os, const std::string & msg) {
    os << msg << " error: " << SDL_GetError() << std::endl;
}

#endif