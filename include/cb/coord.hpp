#pragma once
#ifndef CBCOORD_HPP
#define CBCOORD_HPP

#include <iostream>
#include <string>

namespace Critterbits {

typedef struct CB_Point {
    int x;
    int y;

    CB_Point() : x(0), y(0){};
    CB_Point(int x, int y) : x(x), y(y){};

    inline std::string to_string() const { return "(" + std::to_string(x) + "," + std::to_string(y) + ")"; }

    static inline CB_Point CenterInside(int outer_w, int outer_h, int inner_w, int inner_h) {
        CB_Point point;
        point.x = outer_w / 2 - inner_w / 2;
        point.y = outer_h / 2 - inner_h / 2;
        return point;
    }
    inline bool operator==(const CB_Point & other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const CB_Point & other) const { return !(*this == other); }
    inline CB_Point operator+(const CB_Point & other) const { return {x + other.x, y + other.y}; }
    inline CB_Point operator-(const CB_Point & other) const { return {x - other.x, y - other.y}; }
} CB_Point;

inline CB_Point operator*(const CB_Point & point, float scalar) { return { static_cast<int>(point.x * scalar), static_cast<int>(point.y * scalar) }; }
inline CB_Point operator*(float scalar, const CB_Point & point) { return { static_cast<int>(point.x * scalar), static_cast<int>(point.y * scalar) }; }

typedef struct CB_Rect {
    int x;
    int y;
    int w;
    int h;

    CB_Rect() : x{0}, y{0}, w{0}, h{0} {};
    CB_Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h){};

    inline int bottom() const { return y + h; };
    inline int right() const { return x + w; };
    inline bool empty() const { return w < 1 && h < 1; };
    inline bool inside(const CB_Rect & rect) const {
        return x >= rect.x && right() <= rect.right() && y >= rect.y && bottom() <= rect.bottom();
    };
    inline bool intersects(const CB_Rect & rect) const {
        return !(rect.x > right() || rect.right() < x || rect.y > bottom() || rect.bottom() < y);
    };
    inline CB_Point xy() const { return CB_Point{x, y}; };
    inline void xy(const CB_Point & point) {
        x = point.x;
        y = point.y;
    };
    inline CB_Point wh() const { return CB_Point{w, h}; };
    inline void wh(const CB_Point & point) {
        w = point.x;
        h = point.y;
    };
    inline std::string to_string() const {
        return "[" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(w) + "," + std::to_string(h) +
               "]";
    }
    inline bool operator==(const CB_Rect & other) const {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }
    inline bool operator!=(const CB_Rect & other) const { return !(*this == other); }
} CB_Rect;

class FlexRect {
  public:
    static const int FLEX = -1;

    int top{0};
    int left{0};
    int bottom{0};
    int right{0};
    int width{0};
    int height{0};

    FlexRect(){};
    CB_Rect FlexBasedOn(const CB_Rect &) const;
};

enum class ZIndex { Background, Midground, Foreground, Gui };

typedef struct CB_ViewClippingInfo {
    CB_Rect source{}, dest{};
    ZIndex z_index{ZIndex::Midground};

    CB_ViewClippingInfo(){};
    CB_ViewClippingInfo(const CB_Rect & source, const CB_Rect & dest, const ZIndex & z_index)
        : source(source), dest(dest), z_index(z_index){};
} CB_ViewClippingInfo;
}

#endif