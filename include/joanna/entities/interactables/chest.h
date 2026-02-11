#pragma once

#include "joanna/entities/interactable.h"
#include "joanna/entities/inventory.h"

class Chest: public Interactable {
  public:
    Chest(const sf::Vector2f& position, std::string id);

    void interact(Player& player) override;

    std::string getChestId() const {
        return id;
    }

  private:
    bool isOpen = false;
    std::string id;
    AudioManager audioManager;
};
