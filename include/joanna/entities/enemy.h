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
    Enemy(const jo::Vector2f& startPos, EnemyType type);

    static bool shouldTriggerCombat(float distToPlayer);

    void update(float dt, State state);
    void draw(jo::RenderTarget& target) const;

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
    int updateOverworld(float dt, Player& player, TileManager& tileManager);

  private:
    void updateAIState(float dt, const jo::Vector2f& myPos, const jo::Vector2f& playerPos, float distToPlayer, TileManager& tileManager);
    State handleIdleBehavior(float dt, const jo::Vector2f& myPos);
    State handlePursuingBehavior(float dt, const jo::Vector2f& myPos, const jo::Vector2f& playerPos, float distToPlayer);
    void switchState(State newState);
    void applyFrame();

    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    float frameTimer = 0.f;
    int currentFrame = 0;

    std::vector<Attack> attacks;
    int health = 100;
    int maxHealth = 100;

    // AI movement variables
    static float getDistance(const jo::Vector2f& p1, const jo::Vector2f& p2);

    OverworldState aiState = OverworldState::Idle;
    jo::Vector2f homePoint;
    jo::Vector2f patrolTarget;
    float patrolTimer = 0.f;
    float reactionTimer = 0.f;
    float speed = 38.f;
    EnemyType type;
};
