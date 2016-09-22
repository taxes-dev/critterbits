#pragma once
#ifndef CBCOLOR_HPP
#define CBCOLOR_HPP

#include <SDL.h>

namespace Critterbits {
typedef struct CB_Color {
    int r;
    int g;
    int b;
    int a;

    CB_Color() : r(0), g(0), b(0), a(0){};
    CB_Color(int r, int g, int b) : r(r), g(g), b(b), a(255){};
    CB_Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a){};
    inline bool operator==(const CB_Color & other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    inline bool operator!=(const CB_Color & other) const { return !(*this == other); }
    explicit operator SDL_Color() const {
        SDL_Color color;
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
        return color;
    }
} CB_Color;
}
#endif