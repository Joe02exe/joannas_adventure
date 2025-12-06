#pragma once

#include "joanna/core/combat_types.h"
#include "joanna/entities/entity.h"
#include <vector>

class Enemy: public Entity {
  public:
    Enemy(const sf::Vector2f& startPos, const std::string& texturePath);

    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    void takeDamage(int amount);

    int getHealth() const {
        return health;
    }

    const Attack& chooseAttack();

  private:
    std::vector<Attack> attacks;
    int health = 100;
    int maxHealth = 100;
};
