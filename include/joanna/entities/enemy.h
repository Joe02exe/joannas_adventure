#pragma once

#include "joanna/core/combattypes.h"
#include "joanna/entities/entity.h"
#include "joanna/entities/entityutils.h"
#include <unordered_map>
#include <vector>

class Enemy: public Entity {
  public:
    Enemy(const sf::Vector2f& startPos, const std::string& idlePath);

    void update(float dt, State state);
    void draw(sf::RenderTarget& target) const;

    void takeDamage(int amount);

    int getHealth() const {
        return health;
    }

  private:
    void switchState(State newState);
    void applyFrame();

    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    float frameTimer = 0.f;
    int currentFrame = 0;

    std::vector<Attack> attacks;
    int health = 100;
    int maxHealth = 100;
};
