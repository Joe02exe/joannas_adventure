#include "./character.h"
#include <spdlog/spdlog.h>

#include "./character.h"
#include <spdlog/spdlog.h>

Character::Character(
    const std::string& idlePath, const std::string& walkPath,
    const sf::Vector2f& startPos
) {
    // Load Idle animation
    Animation idleAnim;
    if (!idleAnim.texture.loadFromFile(idlePath))
        spdlog::error("Failed to load {}", idlePath);
    for (int i = 0; i < 8; i++)
        idleAnim.frames.emplace_back(sf::IntRect({ i * 96, 0 }, { 96, 64 }));
    animations[State::Idle] = std::move(idleAnim);

    // Load Walking animation
    Animation walkAnim;
    if (!walkAnim.texture.loadFromFile(walkPath))
        spdlog::error("Failed to load {}", walkPath);
    for (int i = 0; i < 8; i++)
        walkAnim.frames.emplace_back(sf::IntRect({ i * 96, 0 }, { 96, 64 }));
    animations[State::Walking] = std::move(walkAnim);

    //  Construct sprite dynamically
    sprite = std::make_unique<sf::Sprite>(animations[State::Idle].texture);
    sprite->setTextureRect(animations[State::Idle].frames[0]);
    sprite->setPosition(startPos);
}

void Character::update(float dt, bool moving, bool facingLeft) {
    // Determine state and facing
    facing = facingLeft ? Direction::Left : Direction::Right;

    if (moving) {
        switchState(State::Walking);
    } else {
        switchState(State::Idle);
    }
    // Update animation frame
    frameTimer += dt;
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime; // keep leftover time
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }

    flipIfNeeded();
}

void Character::applyFrame() {
    const auto& anim = animations[currentState];
    sprite->setTexture(anim.texture);
    sprite->setTextureRect(anim.frames[currentFrame]);
}

void Character::flipIfNeeded() {
    if (facing == Direction::Left) {
        sprite->setScale({ -1.f, 1.f });
        sprite->setOrigin({ 96.f, 0.f });
    } else {
        sprite->setScale({ 1.f, 1.f });
        sprite->setOrigin({ 0.f, 0.f });
    }
}

void Character::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Character::draw(sf::RenderWindow& window) {
    window.draw(*sprite);
}

void Character::setPosition(const sf::Vector2f& pos) {
    sprite->setPosition(pos);
}

sf::Vector2f Character::getPosition() const {
    return sprite->getPosition();
}
