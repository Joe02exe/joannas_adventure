#pragma once

#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include <SFML/Graphics/RenderWindow.hpp>

class RenderEngine {
  public:
    RenderEngine();

    void
    render(sf::RenderTarget& target, Player& player, TileManager& tileManager);
};