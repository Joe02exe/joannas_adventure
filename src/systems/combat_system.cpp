#include "joanna/systems/combat_system.h"
#include "joanna/utils/resourcemanager.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

CombatSystem::CombatSystem() {
    backgroundTexture = ResourceManager<sf::Texture>::getInstance()->get(
        "assets/images/combat_background_cave.png"
    );
    attackButtonTexture = ResourceManager<sf::Texture>::getInstance()->get(
        "assets/buttons/attack.png"
    );
}

void CombatSystem::startCombat(Player& p, Enemy& e) {
    player = &p;
    enemy = &e;
    currentState = CombatState::PlayerTurn;
    turnTimer = 0.0f;
    std::cout << "Combat Started!\n";

    // Save state
    playerState.position = player->getPosition();
    playerState.scale = player->getScale();
    playerState.facing = player->getFacing();

    if (enemy) {
        enemyState.position = enemy->getPosition();
        enemyState.scale = enemy->getScale();
        enemyState.facing = enemy->getFacing();
    }

    // Position entities for combat
    // Assuming screen size 900x900 (logical view)
    // Player on Left, Enemy on Right
    sf::Vector2f playerPos(-100.f, 300);
    sf::Vector2f enemyPos(430.f, 300);

    player->setPosition(playerPos);
    enemy->setPosition(enemyPos);
    // Face each other
    player->setFacing(Direction::Left);
    enemy->setFacing(Direction::Right);

    // Scale them up
    player->setScale({ 7.f, 7.f });
    enemy->setScale({ 7.f, 7.f });
}

void CombatSystem::endCombat() {
    if (player != nullptr) {
        player->setPosition(playerState.position);
        player->setScale(playerState.scale);
        player->setFacing(playerState.facing);
        // Reset facing visual
        player->flipFace(playerState.facing);
    }
    if (enemy != nullptr) {
        enemy->setPosition(enemyState.position);
        enemy->setScale(enemyState.scale);
        enemy->setFacing(enemyState.facing);
        // Reset facing visual
        enemy->flipFace(enemyState.facing);
    }
}

void CombatSystem::update(float dt) {
    if (player == nullptr || enemy == nullptr)
        return;
    dt -= 0.007f; // just for visual purpose
    // Update animations
    static AudioManager dummyAudio;
    player->update(dt, State::Idle, false, dummyAudio);
    enemy->update(dt);

    if (currentState == CombatState::EnemyTurn) {
        turnTimer += dt;
        if (turnTimer >= turnDuration) {
            performEnemyAttack();
            turnTimer = 0.0f;
            if (player->getHealth() <= 0) {
                currentState = CombatState::Defeat;
                std::cout << "Player Defeated!\n";
            } else {
                currentState = CombatState::PlayerTurn;
            }
        }
    }
}

void CombatSystem::render(sf::RenderTarget& target) {
    // draw static background
    // draw static background
    sf::Sprite backgroundSprite(backgroundTexture);
    sf::Vector2f viewSize = target.getView().getSize();
    sf::Vector2u textureSize = backgroundTexture.getSize();

    float scaleX = viewSize.x / static_cast<float>(textureSize.x);
    float scaleY = viewSize.y / static_cast<float>(textureSize.y);

    backgroundSprite.setScale({ scaleX, scaleY });
    backgroundSprite.setPosition({ 0.f, 0.f });
    target.draw(backgroundSprite);

    if (player == nullptr || enemy == nullptr)
        return;

    // Draw Player (Right side)
    // Player draw uses its position, which we set in startCombat
    player->draw(target);

    // Draw Enemy (Left side)
    // Enemy draw uses its position, which we set in startCombat
    enemy->draw(target);

    // Draw attack buttons for player later
    if (currentState == CombatState::PlayerTurn) {
        sf::Sprite attackButtonSprite(attackButtonTexture);
        attackButtonSprite.setScale({ 3, 3 });
        attackButtonSprite.setPosition({ 95.f, 300.f });

        target.draw(attackButtonSprite);
    }
}

void CombatSystem::handleInput(sf::Event& event) {
    if (currentState != CombatState::PlayerTurn)
        return;

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::A) {
            // TODO Player animation
            std::cout << "Player attacks by pressing A!\n";
        } else if (keyEvent->code == sf::Keyboard::Key::D) {
            std::cout << "Player attacks by pressing D!\n";
        }
        enemy->takeDamage(10);
        if (enemy->getHealth() <= 0) {
            currentState = CombatState::Victory;
            std::cout << "Victory!\n";
        } else {
            currentState = CombatState::EnemyTurn;
        }
    }
}

void CombatSystem::performEnemyAttack() {
    std::cout << "Enemy attacks!\n";
    player->takeDamage(5);
}
