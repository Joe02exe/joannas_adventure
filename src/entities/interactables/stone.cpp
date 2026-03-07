#include <utility>

#include "joanna/entities/interactables/stone.h"

Stone::Stone(const jo::Vector2f& position, std::string id)
    : Interactable(
          jo::FloatRect(position, { 16.f, 16.f }),
          "assets/buttons/interact_A.png", "assets/interactables/stone.png",
          jo::FloatRect(position, { 16.f, 16.f }) // Collision box
      ),
      id(std::move(id)) {
    // Initial frame
    setFrame(jo::IntRect({ 0, 0 }, { 16, 16 }));
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
            setFrame(jo::IntRect({ stage * 16, 0 }, { 16, 16 }));
        }
    }
}

bool Stone::shouldBeRemoved() const {
    return stage >= 3;
}
