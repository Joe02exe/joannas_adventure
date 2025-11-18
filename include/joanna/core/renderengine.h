#pragma once

#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <list>
#include "joanna/entities/interactable.h"

class RenderEngine {
  public:
    RenderEngine();

    void
    render(sf::RenderTarget& target, Player& player, TileManager& tileManager, std::list<std::unique_ptr<Interactable>>& interactables);
};