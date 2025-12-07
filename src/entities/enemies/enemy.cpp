#include "joanna/entities/enemy.h"
#include "joanna/utils/resourcemanager.h"

#include <algorithm>
#include <cstdlib>  // for rand
#include <iostream> // for std::cout

Enemy::Enemy(const sf::Vector2f& startPos, const std::string& idlePath)
    : Entity(
          sf::FloatRect(
              { startPos.x - 48.f, startPos.y - 32.f }, { 96.f, 64.f }
          ),
          ResourceManager<sf::Texture>::getInstance()->get(idlePath),
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 10 }),
          Direction::Right
      ) {

    animations[State::Idle] = Animation(idlePath, { 96, 64 }, 8);
    animations[State::Walking] =
        Animation("assets/player/enemies/goblin/run.png", { 96, 64 }, 8);
    animations[State::Running] =
        Animation("assets/player/enemies/goblin/run.png", { 96, 64 }, 8);

    animations[State::Attack] =
        Animation("assets/player/enemies/goblin/attack.png", { 96, 64 }, 9);
    animations[State::Roll] =
        Animation("assets/player/enemies/goblin/roll.png", { 96, 64 }, 10);
    animations[State::Hurt] =
        Animation("assets/player/enemies/goblin/hurt.png", { 96, 64 }, 8);
    animations[State::Dead] =
        Animation("assets/player/enemies/goblin/dead.png", { 96, 64 }, 9);
    animations[State::Mining] =
        Animation("assets/player/enemies/goblin/mining.png", { 96, 64 }, 10);

    // Set initial texture
    applyFrame();

    // Initialize some dummy attacks
    attacks.push_back({ "Scratch", 5 });
    attacks.push_back({ "Bite", 10 });
}

void Enemy::update(float dt, State state) {
    if (currentState != state) {
        switchState(state);
    }

    frameTimer += dt;
    const auto& anim = animations[currentState];
    // Use frameCount from animation if we had it exposed, or just size of
    // frames
    float duration = anim.frames.size() * Animation::frameTime;

    if (frameTimer >= Animation::frameTime) {
        frameTimer -= Animation::frameTime;

        if (currentState == State::Dead &&
            currentFrame == anim.frames.size() - 1) {
            // Do not loop dead animation
        } else {
            currentFrame++;
            if (currentFrame >= anim.frames.size()) {
                currentFrame = 0;
            }
        }
        applyFrame();
    }
}

void Enemy::draw(sf::RenderTarget& target) const {
    Entity::render(target);
}

void Enemy::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Enemy::applyFrame() {
    const auto& anim = animations[currentState];
    setTexture(anim.texture);
    setFrame(anim.frames[currentFrame]);
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    health = std::max(health, 0);
}
