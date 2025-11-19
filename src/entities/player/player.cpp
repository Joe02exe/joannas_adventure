#include "joanna/entities/player.h"
#include "joanna/systems/audiomanager.h"

Player::Player(
    const std::string& idlePath, const std::string& walkPath,
    const std::string& runPath, const sf::Vector2f& startPos
){
    animations[State::Idle] = Animation(idlePath, { 96, 64 });
    animations[State::Walking] = Animation(walkPath, { 96, 64 });
    animations[State::Running] = Animation(runPath, { 96, 64 });

    //  Construct sprite dynamically
    sprite = std::make_unique<sf::Sprite>(animations[State::Idle].texture);
    sprite->setTextureRect(animations[State::Idle].frames[0]);
    sprite->setPosition(startPos);
}

void Player::update(
    float dt, State state, bool movingRight, AudioManager& pManager
) {
    facing = movingRight ? Direction::Left : Direction::Right;
    switchState(state);

    frameTimer += dt;
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        if(state != State::Idle && currentFrame % 4 == 0) {
            pManager.play_sfx(SfxId::Footstep);
        }

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

void Player::draw(sf::RenderTarget& target) const {
    target.draw(*sprite);
    inventory.draw(target);
}

void Player::setPosition(const sf::Vector2f& pos) const {
    sprite->setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite->getPosition();
}

void Player::addItemToInventory(const Item& item, const std::uint32_t quantity) {
    inventory.addItem(item, quantity);
}