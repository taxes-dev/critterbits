#pragma once
#ifndef CBCOORD_H
#define CBCOORD_H

namespace Critterbits {

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
}
#endif