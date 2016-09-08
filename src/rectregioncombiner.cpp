#include <algorithm>

#include <critterbits.h>

namespace Critterbits {
    void RectRegionCombiner::Combine() {
        LOG_INFO("RectRegionCombiner::Combine starting with " + std::to_string(this->regions.size()) + " regions to combine");
        int combines;
        std::vector<CB_Rect> new_regions;
        CB_Rect new_rect;

        // remove duplicates
        this->regions.erase(
            std::unique(this->regions.begin(), this->regions.end(), [](const CB_Rect & rect1, const CB_Rect & rect2) {
                return rect1 == rect2 || rect1.inside(rect2) || rect2.inside(rect1); 
            }),
            this->regions.end());

        // brute force!
        do {
            combines = 0;
            for (auto & rect1 : this->regions) {
                if (rect1.empty()) continue;
                for (auto & rect2 : this->regions) {
                    if (rect2.empty()) continue;
                    // regions horizontally adjacent
                    if (rect1.y == rect2.y &&
                        rect1.h == rect2.h &&
                        (rect1.x == rect2.right() ||
                        rect1.right() == rect2.x)) {
                            new_rect.x = std::min(rect1.x, rect2.x);
                            new_rect.y = rect1.y;
                            new_rect.w = rect1.w + rect2.w;
                            new_rect.h = rect1.h;
                            new_regions.push_back(new_rect);
                            rect1.w = 0; rect1.h = 0;
                            rect2.w = 0; rect2.h = 0;
                            combines++;
                    }
                    // regions vertically adjacent
                    if (rect1.x == rect2.x &&
                        rect1.w == rect2.w &&
                        (rect1.y == rect2.bottom() ||
                        rect1.bottom() == rect2.y)) {
                            new_rect.x = rect1.x;
                            new_rect.y = std::min(rect1.y, rect2.y);
                            new_rect.w = rect1.w;
                            new_rect.h = rect1.h + rect2.h;
                            new_regions.push_back(new_rect);
                            rect1.w = 0; rect1.h = 0;
                            rect2.w = 0; rect2.h = 0;
                            combines++;
                        }
                } // for rect2
                if (!rect1.empty()) {
                    new_regions.push_back(rect1);
                }
            } // for rect1
            this->regions = new_regions;
            new_regions.clear();
        } while(combines > 0);

        this->regions.shrink_to_fit();
        LOG_INFO("RectRegionCombiner::Combine combined down to " + std::to_string(this->regions.size()) + " regions");
    }
}