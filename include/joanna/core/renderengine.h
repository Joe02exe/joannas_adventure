#pragma once

#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include "joanna/utils/dialogue_box.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <list>
#include "joanna/entities/interactable.h"
#include "joanna/entities/enemy.h"
#include "joanna/systems/combat_system.h"

class RenderEngine {
  public:
    RenderEngine();

    void
    render(sf::RenderTarget& target, Player& player, TileManager& tileManager, std::list<std::unique_ptr<Entity>>& entities, std::shared_ptr<DialogueBox> dialogueBox);
};