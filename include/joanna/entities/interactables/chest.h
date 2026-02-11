#pragma once

#include "joanna/entities/interactable.h"
#include "joanna/entities/inventory.h"

class Chest : public Interactable {
  public:
    Chest(const sf::Vector2f& position);

    void interact(Player& player) override;

  private:
    bool isOpen = false;
};
