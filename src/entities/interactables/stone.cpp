#include "joanna/entities/interactables/stone.h"

Stone::Stone(const sf::Vector2f& position)
    : Interactable(
        sf::FloatRect(position, {16.f, 16.f}), 
        "assets/buttons/interact_T.png",
        "assets/interactables/stone.png",
        sf::FloatRect(position, {18.f, 18.f}) // Collision box
      ) {
    // Initial frame
    setFrame(sf::IntRect({0, 0}, {16, 16}));
}

void Stone::interact(Player& player) {
    if (waitingForHit) { return; // Already interacting
}
    
    player.startMining();
    waitingForHit = true;
    hasHitThisAnimation = false;
}

void Stone::update(float dt, Player& player) {
    if (!waitingForHit) { return;
}

    // Check if player is still mining
    if (player.getState() != State::Mining) {
        waitingForHit = false;
        return;
    }

    // Check frame
    int currentFrame = player.getCurrentFrame();
    
    if (currentFrame == miningHitFrame && !hasHitThisAnimation) {
        // Hit!
        hasHitThisAnimation = true;
        stage++;

        // Update visuals
        if (stage < 3) {
            setFrame(sf::IntRect({stage * 16, 0}, {16, 16}));
        }
        
        // TODO: Play hit sound
    }
}

bool Stone::shouldBeRemoved() const {
    return stage >= 3;
}
