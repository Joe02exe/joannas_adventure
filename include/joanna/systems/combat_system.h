#pragma once

#include "joanna/core/combat_types.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include <SFML/Graphics.hpp>

class CombatSystem {
  public:
    CombatSystem();

    void startCombat(Player& player, Enemy* enemy);
    void update(float dt);
    void render(sf::RenderTarget& target);
    void handleInput(sf::Event& event);

    CombatState getState() const {
        return currentState;
    }

  private:
    Player* player = nullptr;
    Enemy* enemy = nullptr;
    CombatState currentState = CombatState::PlayerTurn;

    float turnTimer = 0.0f;
    const float turnDuration = 1.0f; // Delay between turns for pacing

    void performEnemyAttack();
};
