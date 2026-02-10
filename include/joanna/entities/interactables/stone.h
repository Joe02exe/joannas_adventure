#pragma once

#include "joanna/entities/interactable.h"

class Stone: public Interactable {
  public:
    Stone(const sf::Vector2f& position, std::string id);

    void interact(Player& player) override;
    void update(float dt, Player& player);

    bool shouldBeRemoved() const;

    std::string getStoneId() const {
        return id;
    }

  private:
    int stage = 0;
    bool waitingForHit = false;
    bool hasHitThisAnimation = false;
    std::string id;

    // Config
    const int miningHitFrame = 7; // Adjust based on animation
};
