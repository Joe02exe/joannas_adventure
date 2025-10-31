#pragma once

#include "controller.h"
#include "../../core/logger.h"
#include "../../core/menu.h"

#include <SFML/Graphics/RenderWindow.hpp>

Controller::Controller(Player& player, sf::View& camera)
    : player(&player), camera(&camera) {}

void Controller::getInput(float dt, sf::RenderWindow& window) {
    float factor = 60.0f * 0.5f;

    Player::State state = Player::State::Idle;
    sf::Vector2f dir{ 0.f, 0.f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        dir.x -= 1.f * factor * dt;
        facingLeft = true;
        state = Player::State::Walking;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        dir.x += 1.f * factor * dt;
        facingLeft = false;
        state = Player::State::Walking;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        dir.y -= 1.f * factor * dt;
        state = Player::State::Walking;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        dir.y += 1.f * factor * dt;
        state = Player::State::Walking;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
        dir *= 1.5f;
        state = Player::State::Running;
    }

    // normalize the diagonal movement
    if (dir.x != 0.f && dir.y != 0.f) {
        dir *= 0.7071f; // approx 1/sqrt(2)
    }

    camera->move(dir);

    player->setPosition(
        { camera->getCenter().x - 48.f, camera->getCenter().y - 32.f }
    ); // subtract half the size of character

    if (dir.x != 0.f || dir.y != 0.f) {
        Logger::info(
            "Camera position: x={}, y={}", camera->getCenter().x,
            camera->getCenter().y
        );
    }

    // Update and draw the player AFTER (foreground)
    player->update(dt, state, facingLeft);
}