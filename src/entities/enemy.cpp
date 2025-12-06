#include "joanna/entities/enemy.h"
#include <cstdlib> // for rand

Enemy::Enemy(const sf::Vector2f& startPos, const std::string& texturePath)
    : Entity(
          sf::FloatRect(startPos, { 32.f, 32.f }), sf::Texture()
      ) { // Placeholder init

    sf::Texture tex;
    if (tex.loadFromFile(texturePath)) {
        setTexture(tex);
        // Update box if needed, but Entity copies texture so we set it via
        // setTexture which might handle it. Actually Entity constructor takes
        // texture, let's try to load it first if possible or just use a dummy
        // one and set it later. Since we can't easily load before base init
        // without a static helper, let's use a dummy and set it in body.
    }

    // Initialize some dummy attacks
    attacks.push_back({ "Scratch", 5 });
    attacks.push_back({ "Bite", 10 });
}

void Enemy::update(float dt) {
    // Basic update
}

void Enemy::draw(sf::RenderTarget& target) const {
    // Draw logic
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0)
        health = 0;
}

const Attack& Enemy::chooseAttack() {
    return attacks[std::rand() % attacks.size()];
}
