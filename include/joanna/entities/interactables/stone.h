#pragma once

#include "joanna/entities/interactable.h"

class Stone : public Interactable {
  public:
    Stone(const sf::Vector2f& position);

    void interact(Player& player) override;
    void update(float dt, Player& player);

    bool shouldBeRemoved() const;

  private:
    int stage = 0;
    bool waitingForHit = false;
    bool hasHitThisAnimation = false;
    
    // Config
    const int miningHitFrame = 7; // Adjust based on animation
};
