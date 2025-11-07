#include "./player.h"

#include <spdlog/spdlog.h>

Player::Player(
    const std::string& idlePath, const std::string& walkPath,
    const std::string& runPath, const sf::Vector2f& startPos
) {
    animations[State::Idle] = Animation(idlePath, { 96, 64 });
    animations[State::Walking] = Animation(walkPath, { 96, 64 });
    animations[State::Running] = Animation(runPath, { 96, 64 });

    //  Construct sprite dynamically
    sprite = std::make_unique<sf::Sprite>(animations[State::Idle].texture);
    sprite->setTextureRect(animations[State::Idle].frames[0]);
    sprite->setPosition(startPos);
}

void Player::update(float dt, State state, bool facingLeft) {
    facing = facingLeft ? Direction::Left : Direction::Right;
    switchState(state);

    frameTimer += dt;
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime; // keep leftover time
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
    flipIfNeeded();
}

void Player::applyFrame() {
    const auto& anim = animations[currentState];
    sprite->setTexture(anim.texture);
    sprite->setTextureRect(anim.frames[currentFrame]);
}

void Player::flipIfNeeded() {
    if (facing == Direction::Left) {
        sprite->setScale({ -1.f, 1.f });
        sprite->setOrigin({ 96.f, 0.f });
    } else {
        sprite->setScale({ 1.f, 1.f });
        sprite->setOrigin({ 0.f, 0.f });
    }
}

void Player::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Player::draw(sf::RenderTarget& target) {
    target.draw(*sprite);
}

void Player::setPosition(const sf::Vector2f& pos) {
    sprite->setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite->getPosition();
}
