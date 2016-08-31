#pragma once
#ifndef CBCOORD_H
#define CBCOORD_H

namespace Critterbits {

inline int Clamp(int value, int min, int max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

typedef struct CB_Point {
    int x;
    int y;

    static inline CB_Point CenterInside(int outer_w, int outer_h, int inner_w, int inner_h) {
        CB_Point point;
        point.x = outer_w / 2 - inner_w / 2;
        point.y = outer_h / 2 - inner_h / 2;
        return point;
    }

} CB_Point;

typedef struct CB_Rect {
    int x;
    int y;
    int w;
    int h;

    inline int bottom() { return y + h; };
    inline int right() { return x + w; };
    inline bool intersects(CB_Rect & rect) {
        return !(rect.x > right() || rect.right() < x || rect.y > bottom() || rect.bottom() < y);
    };
} CB_Rect;

typedef struct CB_ViewClippingInfo { CB_Rect source, dest; } CB_ViewClippingInfo;
}
#endif