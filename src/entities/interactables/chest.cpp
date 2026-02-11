#include "joanna/entities/interactables/chest.h"
#include "joanna/utils/logger.h"

Chest::Chest(const sf::Vector2f& position)
    : Interactable(
          sf::FloatRect(position, { 16.f, 22.f }),
          "assets/buttons/interact_T.png", "assets/interactables/chest.png",
          sf::FloatRect(position, { 16.f, 22.f }) // Collision box
      ) {
    setFrame(sf::IntRect({ 0, 0 }, { 16, 22 }));
}

void Chest::interact(Player& player) {
    if (!isOpen) {
        isOpen = true;
        setFrame(sf::IntRect({ 16, 0 }, { 16, 22 }));
        player.addItemToInventory(Item("630", "Grade"),1);
    } else {
        Logger::info("Chest is already open.");
    }
}
