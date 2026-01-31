#include "joanna/entities/interactables/stone.h"

Stone::Stone(const sf::Vector2f& position)
    : Interactable(
          sf::FloatRect(position, { 16.f, 16.f }),
          "assets/buttons/interact_T.png", "assets/interactables/stone.png",
          sf::FloatRect(position, { 16.f, 16.f }) // Collision box
      ) {
    // Initial frame
    setFrame(sf::IntRect({ 0, 0 }, { 16, 16 }));
}

void Stone::interact(Player& player) {
    if (!player.getInventory().hasItemByName("pickaxe")) {
        return;
    }
    if (waitingForHit) {
        return; // Already interacting
    }

    player.setFacing(Direction::Right);
    player.startMining();
    waitingForHit = true;
    hasHitThisAnimation = false;
}

void Stone::update(float dt, Player& player) {
    if (!waitingForHit) {
        return;
    }

    if (player.getState() != State::Mining) {
        waitingForHit = false;
        return;
    }

    int currentFrame = player.getCurrentFrame();

    if (currentFrame == miningHitFrame && !hasHitThisAnimation) {
        hasHitThisAnimation = true;
        stage++;

        if (stage < 3) {
            setFrame(sf::IntRect({ stage * 16, 0 }, { 16, 16 }));
        }
    }
}

bool Stone::shouldBeRemoved() const {
    return stage >= 3;
}
