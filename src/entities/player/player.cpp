#include "joanna/entities/player.h"
#include <iostream>

void Player::takeDamage(int amount) {
    health -= amount;
    if (health < 0)
        health = 0;
    std::cout << "Player took " << amount << " damage. Health: " << health
              << "\n";
    // Trigger Hurt animation logic if needed, but CombatSystem handles state
    // switch
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
      health(100), maxHealth(100), inventory(20) {
    animations[State::Idle] = Animation(idlePath, { 96, 64 }, 9);
    animations[State::Walking] = Animation(walkPath, { 96, 64 }, 8);
    animations[State::Running] = Animation(runPath, { 96, 64 }, 8);

    // Load combat animations
    animations[State::Attack] =
        Animation("assets/player/main/attack.png", { 96, 64 }, 10);
    animations[State::Roll] =
        Animation("assets/player/main/roll.png", { 96, 64 }, 10);
    animations[State::Hurt] =
        Animation("assets/player/main/hurt.png", { 96, 64 }, 8);
    animations[State::Dead] =
        Animation("assets/player/main/dead.png", { 96, 64 }, 13);

    // Initialize attacks
    attacks.push_back({ "Attack", 10, State::Attack });
    attacks.push_back({ "Roll", 15, State::Roll });
}

void Player::update(
    float dt, State state, bool facingLeft, AudioManager& pManager
) {
    setFacing(facingLeft ? Direction::Left : Direction::Right);
    switchState(state);

    frameTimer += dt;
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        if ((state == State::Running || state == State::Walking) &&
            currentFrame % 4 == 0) {
            pManager.play_sfx(SfxId::Footstep);
        }

        frameTimer -= anim.frameTime; // keep leftover time
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
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