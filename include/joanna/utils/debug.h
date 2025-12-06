#pragma once

#include "joanna/core/game_state.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include "joanna/systems/combat_system.h"

#include <SFML/Graphics/RenderWindow.hpp>

class DebugUI {
  public:
    static void init(sf::RenderWindow& window);
    void processEvent(const sf::Window& window, const sf::Event& event) const;
    void update(
        float dt, sf::RenderWindow& window, Player& player,
        GameState& gameState, CombatSystem& combatSystem, Enemy& testEnemy
    ) const;
    void render(sf::RenderWindow& window) const;
    static void shutdown();

    void toggle() { enabled = !enabled; }

  private:
    bool enabled = true;
};
