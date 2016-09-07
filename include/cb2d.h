#pragma once
#ifndef CB2D_H
#define CB2D_H

namespace Critterbits {
    inline bool AabbCollision(const CB_Rect & rect1, const CB_Rect & rect2) {
        return (rect1.x < rect2.x + rect2.w &&
            rect1.x + rect1.w > rect2.x &&
            rect1.y < rect2.y + rect2.h &&
            rect1.h + rect1.y > rect2.y);
    }

}
#endif