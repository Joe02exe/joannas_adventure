#include <utility>

#include "joanna/entities/interactables/chest.h"
#include "joanna/utils/logger.h"

Chest::Chest(const sf::Vector2f& position, std::string id)
    : Interactable(
          sf::FloatRect(position, { 16.f, 22.f }),
          "assets/buttons/interact_T.png", "assets/interactables/chest.png",
          sf::FloatRect(position, { 16.f, 22.f }) // Collision box
      ),
      id(std::move(id)) {
    setFrame(sf::IntRect({ 0, 0 }, { 16, 22 }));
    audioManager = AudioManager();
}

void Chest::interact(Player& player) {
    if (!isOpen) {
        isOpen = true;
        audioManager.play_sfx(SfxId::Chest);
        setFrame(sf::IntRect({ 16, 0 }, { 16, 22 }));
        player.addItemToInventory(Item("630", "Grade"), 1);
        player.addInteraction("chestOpened");
    } else {
        Logger::info("Chest is already open.");
    }
}

void Chest::setChestOpen(const bool open) {
    isOpen = open;
    if (isOpen) {
        setFrame(sf::IntRect({ 16, 0 }, { 16, 22 }));
    } else {
        setFrame(sf::IntRect({ 0, 0 }, { 16, 22 }));
    }
}
