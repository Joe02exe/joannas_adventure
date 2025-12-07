#pragma once

#include "joanna/core/combat_types.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include <SFML/Graphics.hpp>

struct EntityState {
    sf::Vector2f position;
    sf::Vector2f scale;
    Direction facing;
};

class CombatSystem {
  public:
    CombatSystem();

    void startCombat(Player& player, Enemy& enemy);
    void endCombat();
    void update(float dt);
    void render(sf::RenderTarget& target);
    void handleInput(sf::Event& event);

    CombatState getState() const {
        return currentState;
    }

  private:
    Player* player = nullptr;
    Enemy* enemy = nullptr;
    
    EntityState playerState;
    EntityState enemyState;

    sf::Texture backgroundTexture;
    sf::Texture attackButtonTexture;

    CombatState currentState = CombatState::PlayerTurn;

    float turnTimer = 0.0f;
    const float turnDuration = 1.0f; // Delay between turns for pacing

    void performEnemyAttack();
};
