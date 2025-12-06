#include "joanna/entities/enemy.h"
#include "joanna/utils/resourcemanager.h"

#include <cstdlib> // for rand

Enemy::Enemy(const sf::Vector2f& startPos, const std::string& idlePath)
    : Entity(
          sf::FloatRect({ startPos.x - 48.f, startPos.y - 32.f }, { 96.f, 64.f }), ResourceManager<sf::Texture>::getInstance()->get(idlePath)
      ) { // Placeholder init

    // We don't need to load texture here manually if we use Animation struct
    // But for now let's keep it consistent or just use the animation map.
    
    // Initialize animations
    // Assuming goblin idle is 32x32 per frame? Or maybe larger?
    // User said "player/enemies/goblin/idle.png".
    // Let's assume standard size for now, maybe 32x32 or 64x64.
    // Player uses 96x64.
    // Let's try to load it and see size or assume 32x32 based on "newmap.json" or similar?
    // Actually, let's just use a reasonable default like 32x32 or 48x48.
    // If the file is a strip, Animation struct handles it.
    // Let's assume 32x32 for now as it's a common tile size.
    animations[State::Idle] = Animation(idlePath, { 96, 64 });
    
    // Set initial texture
    applyFrame();

    // Initialize some dummy attacks
    attacks.push_back({ "Scratch", 5 });
    attacks.push_back({ "Bite", 10 });
}

void Enemy::update(float dt) {
    frameTimer += dt;
    const auto& anim = animations[currentState];
    // Safety check if animation has frames
    if (!anim.frames.empty()) {
        if (frameTimer >= anim.frameTime) {
            frameTimer -= anim.frameTime;
            currentFrame = (currentFrame + 1) % anim.frames.size();
            applyFrame();
        }
    }
    flipFace(getFacing());
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
    if (anim.frames.empty()) return;
    setTexture(anim.texture);
    setFrame(anim.frames[currentFrame]);
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0)
        health = 0;
}

const Attack& Enemy::chooseAttack() {
    return attacks[std::rand() % attacks.size()];
}
