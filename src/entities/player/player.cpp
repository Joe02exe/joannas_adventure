#include "joanna/entities/player.h"
#include <iostream>

void Player::takeDamage(int amount) {
    //TODO also subtract health from player
    std::cout << "Player took " << amount << " damage\n";
}

#include "joanna/systems/audiomanager.h"
#include "joanna/utils/resourcemanager.h"

Player::Player(
    const std::string& idlePath, const std::string& walkPath,
    const std::string& runPath, const sf::Vector2f& startPos
)
    : Entity(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          ResourceManager<sf::Texture>::getInstance()->get(idlePath),
          sf::FloatRect({ startPos.x, startPos.y }, { 10, 8 }), Direction::Right
      ),
      inventory(20) {
    animations[State::Idle] = Animation(idlePath, { 96, 64 });
    animations[State::Walking] = Animation(walkPath, { 96, 64 });
    animations[State::Running] = Animation(runPath, { 96, 64 });
}

void Player::update(
    float dt, State state, bool movingRight, AudioManager& pManager
) {
    setFacing(movingRight ? Direction::Right : Direction::Left);
    switchState(state);

    frameTimer += dt;
    Logger::info("dt: " + std::to_string(dt));
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        if (state != State::Idle && currentFrame % 4 == 0) {
            pManager.play_sfx(SfxId::Footstep);
        }

        frameTimer -= anim.frameTime; // keep leftover time
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
    flipFace(getFacing());
}

void Player::applyFrame() {
    const auto& anim = animations[currentState];
    setTexture(anim.texture);
    setFrame(anim.frames[currentFrame]);
}

void Player::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Player::draw(sf::RenderTarget& target) const {
    Entity::render(target);
    inventory.draw(target);
}

void Player::addItemToInventory(
    const Item& item, const std::uint32_t quantity
) {
    inventory.addItem(item, quantity);
}