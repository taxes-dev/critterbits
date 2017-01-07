#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Animation {
void TranslateAnimation::AnimateValues(std::shared_ptr<Entity> entity, float percent) {
    CB_Point xy{this->GetTransformedValue(this->start, this->end, percent)};
    if (entity->GetEntityType() == EntityType::Sprite) {
        std::dynamic_pointer_cast<Sprite>(entity)->SetPosition(xy.x, xy.y);
    } else {
        entity->dim.xy(xy);
    }
}
}
}