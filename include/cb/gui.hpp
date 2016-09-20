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

    GuiPanel() : Entity(){ this->debug = true; };
    void Close() { this->state = EntityState::Inactive; };
    EntityType GetEntityType() const { return EntityType::GuiPanel; };
    void Open() { this->state = EntityState::Active; };

  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
};

class GuiManager {
  public:
    std::vector<std::shared_ptr<GuiPanel>> panels;

    bool ClosePanel(entity_id_t);
    entity_id_t OpenPanel(const std::string &, bool = false);
    void UnloadPanel(std::shared_ptr<GuiPanel>);
};
}
}
#endif