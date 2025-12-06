#include "joanna/systems/combat_system.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

CombatSystem::CombatSystem() = default;

void CombatSystem::startCombat(Player& p, Enemy* e) {
    player = &p;
    enemy = e;
    currentState = CombatState::PlayerTurn;
    turnTimer = 0.0f;
    std::cout << "Combat Started!\n";
}

void CombatSystem::update(float dt) {
    if (player == nullptr || enemy == nullptr)
        return;

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
    sf::RectangleShape background(target.getView().getSize());
    background.setFillColor(sf::Color(50, 50, 150));
    target.draw(background);

    if (player == nullptr || enemy == nullptr)
        return;

    // Placeholder rendering for scaffolding -> store plaayer and enemy pos later in the system
    sf::RectangleShape playerShape(sf::Vector2f(50.f, 50.f));
    playerShape.setFillColor(sf::Color::Green);
    playerShape.setPosition({ target.getView().getSize().x * 0.75f,
                              target.getView().getSize().y * 0.5f });
    target.draw(playerShape);

    // Draw Enemy (Left side)
    sf::RectangleShape enemyShape(sf::Vector2f(50.f, 50.f));
    enemyShape.setFillColor(sf::Color::Red);
    enemyShape.setPosition({ target.getView().getSize().x * 0.25f,
                             target.getView().getSize().y * 0.5f });
    target.draw(enemyShape);

    // Draw attack buttons for player later
    if (currentState == CombatState::PlayerTurn) {
        // Simple visual representation of buttons
        sf::RectangleShape button(sf::Vector2f(100.f, 40.f));
        button.setFillColor(sf::Color::White);
        button.setPosition({ target.getView().getSize().x * 0.5f -
                                 50.f,
                             target.getView().getSize().y * 0.8f });
        target.draw(button);
    }
}

void CombatSystem::handleInput(sf::Event& event) {
    if (currentState != CombatState::PlayerTurn)
        return;

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Space) {
            // TODO Player animation
            std::cout << "Player attacks!\n";
            enemy->takeDamage(10);
            if (enemy->getHealth() <= 0) {
                currentState = CombatState::Victory;
                std::cout << "Victory!\n";
            } else {
                currentState = CombatState::EnemyTurn;
            }
        }
    }
}

void CombatSystem::performEnemyAttack() {
    std::cout << "Enemy attacks!\n";
    player->takeDamage(5);
}
