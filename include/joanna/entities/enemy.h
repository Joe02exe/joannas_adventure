#pragma once

#include "joanna/core/combattypes.h"
#include "joanna/entities/entity.h"
#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include "joanna/entities/entityutils.h"
#include <unordered_map>
#include <vector>

class Enemy: public Entity {
  public:
    enum class EnemyType { Goblin, Skeleton };
    Enemy(const sf::Vector2f& startPos, EnemyType type);

    void update(float dt, State state);
    void draw(sf::RenderTarget& target) const;

    void takeDamage(int amount);

    int getHealth() const {
        return health;
    }

    bool isDead() const {
        return health <= 0;
    }

    void resetHealth() {
        health = maxHealth;
    }

    const std::vector<Attack>& getAttacks() const {
        return attacks;
    }

    EnemyType getType() const {
        return type;
    }

    enum class OverworldState { Idle, Pursuing };
    bool updateOverworld(float dt, Player& player, TileManager& tileManager);

  private:
    void switchState(State newState);
    void applyFrame();

    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    float frameTimer = 0.f;
    int currentFrame = 0;

    std::vector<Attack> attacks;
    int health = 10;
    int maxHealth = 10;

    // AI movement variables
    OverworldState aiState = OverworldState::Idle;
    sf::Vector2f homePoint;
    sf::Vector2f patrolTarget;
    float patrolTimer = 0.f;
    float reactionTimer = 0.f;
    float speed = 38.f;
    EnemyType type;
};
