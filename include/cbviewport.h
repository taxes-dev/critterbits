#pragma once
#ifndef CBVIEWPORT_H
#define CBVIEWPORT_H

#include "cbcoord.h"
#include "cbentity.h"

namespace Critterbits {

class Viewport : public Entity {
  public:
    Viewport(){};
    CB_ViewClippingInfo GetViewableRect(CB_Rect &) const;
};
}

#endif