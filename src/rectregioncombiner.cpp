#include <algorithm>

#include <cb/critterbits.hpp>

namespace Critterbits {
void RectRegionCombiner::Combine() {
    LOG_INFO("RectRegionCombiner::Combine starting with " + std::to_string(this->regions.size()) +
             " regions to combine");
    int combines;
    CB_Rect new_rect;

    // remove duplicates and completely overlapping rects
    this->regions.erase(std::unique(this->regions.begin(), this->regions.end(),
                                    [](const CB_Rect & rect1, const CB_Rect & rect2) {
                                        // NOTE: need to check this on other STL implementations, as we're relying on the
                                        // notion that rect1 will be overwritten with rect2 by std::unique. If it's the other
                                        // way around, the small rect would be preserved instead of the larger one.
                                        return rect1 == rect2 || rect1.inside(rect2);
                                    }),
                        this->regions.end());

    // brute force!
    do {
        combines = 0;
        for (auto & rect1 : this->regions) {
            if (rect1.empty())
                continue;
            for (auto & rect2 : this->regions) {
                if (rect2.empty())
                    continue;
                // regions horizontally adjacent
                if (rect1.y == rect2.y && rect1.h == rect2.h &&
                    (rect1.x == rect2.right() || rect1.right() == rect2.x)) {
                    new_rect.x = std::min(rect1.x, rect2.x);
                    new_rect.y = rect1.y;
                    new_rect.w = rect1.w + rect2.w;
                    new_rect.h = rect1.h;
                    this->regions.push_back(new_rect);
                    rect1.w = 0;
                    rect1.h = 0;
                    rect2.w = 0;
                    rect2.h = 0;
                    combines++;
                }
                // regions vertically adjacent
                if (rect1.x == rect2.x && rect1.w == rect2.w &&
                    (rect1.y == rect2.bottom() || rect1.bottom() == rect2.y)) {
                    new_rect.x = rect1.x;
                    new_rect.y = std::min(rect1.y, rect2.y);
                    new_rect.w = rect1.w;
                    new_rect.h = rect1.h + rect2.h;
                    this->regions.push_back(new_rect);
                    rect1.w = 0;
                    rect1.h = 0;
                    rect2.w = 0;
                    rect2.h = 0;
                    combines++;
                }
            } // for rect2
        }     // for rect1
    } while (combines > 0);

    this->regions.erase(
        std::remove_if(this->regions.begin(), this->regions.end(), [](const CB_Rect & rect) { return rect.empty(); }),
        this->regions.end());
    this->regions.shrink_to_fit();

    LOG_INFO("RectRegionCombiner::Combine combined down to " + std::to_string(this->regions.size()) + " regions");
}
}