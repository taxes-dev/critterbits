#pragma once
#ifndef CBGUI_HPP
#define CBGUI_HPP

#include <memory>
#include <vector>

#include "entity.hpp"

namespace Critterbits {
namespace Gui {

class GuiPanel : public Entity {
  public:
    std::string panel_name;

    GuiPanel() : Entity(){};
    EntityType GetEntityType() const { return EntityType::GuiPanel; };

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
};

class GuiManager {
  public:
    std::vector<std::shared_ptr<GuiPanel>> panels;
};
}
}
#endif