#pragma once

#include "joanna/entities/enemy.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/world/tilemanager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <list>

class RenderEngine {
  public:
    RenderEngine();

    void render(
        sf::RenderTarget& target, Player& player, TileManager& tileManager,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& dialogueBox, float dt
    );

  private:
    float offset = 0.f;
    float dir = 1.f;
    float frameTimer = 0.f;
};
