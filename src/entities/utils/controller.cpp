#pragma once

#include "controller.h"
#include "../../core/logger.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

Controller::Controller(Player& player, sf::View& camera, sf::View& miniMapView)
    : player(&player), camera(&camera), miniMapView(&miniMapView) {}

// clang-format off
const bool isColliding(const sf::FloatRect& nextPlayerBox, const sf::FloatRect& box) {
    const bool AIsRightToB = nextPlayerBox.position.x - nextPlayerBox.size.x/2.f >= box.position.x + box.size.x/2.f;
    const bool AIsLeftToB  = nextPlayerBox.position.x + nextPlayerBox.size.x/2.f <= box.position.x - box.size.x/2.f;
    const bool AIsBelowB   = nextPlayerBox.position.y - nextPlayerBox.size.y/2.f >= box.position.y + box.size.y/2.f;
    const bool AIsAboveB   = nextPlayerBox.position.y + nextPlayerBox.size.y/2.f <= box.position.y - box.size.y/2.f;
    return !(AIsRightToB || AIsLeftToB || AIsBelowB || AIsAboveB);
}


// Axis-separated collision movement
sf::Vector2f moveWithCollisions(
    const sf::Vector2f& dir, const sf::FloatRect& playerBox,
    const std::vector<sf::FloatRect>& collisions
) {

    sf::Vector2f result = dir;
    sf::FloatRect nextPlayerBox = playerBox;
    nextPlayerBox.position.x += dir.x;
    bool blockedX = false;
    for (const auto& box : collisions) {

        if (isColliding(nextPlayerBox, box)) {
            // if B.left- A.right = 0, only move along y axis or if B.right - A.left = 0
            if ((box.position.x + box.size.x/2.f - (nextPlayerBox.position.x - nextPlayerBox.size.x/2.f) < 0.01f) ||
                (box.position.x - box.size.x/2.f - (nextPlayerBox.position.x + nextPlayerBox.size.x/2.f ) < 0.01f)) {
                Logger::info(
                    "Collision on X axis detected at object x={}, y={}",
                    box.position.x, box.position.y
                );
                result.x = 0.f;
            }
            // if B.above - A.bottom = 0, only move along x axis or if B.bottom - A.top = 0
            if ((box.position.y + box.size.y/2.f - (nextPlayerBox.position.y - nextPlayerBox.size.y/2.f) < 0.01f) ||
                (box.position.y - box.size.y/2.f - (nextPlayerBox.position.y + nextPlayerBox.size.y/2.f) < 0.01f)) {
                Logger::info(
                    "Collision on Y axis detected at object x={}, y={}",
                    box.position.x, box.position.y
                );
                result.y = 0.f;
            }
        }
    }
    return result;
}

// clang-format on

void Controller::getInput(
    float dt, sf::RenderWindow& window,
    const std::vector<sf::FloatRect>& collisions
) {
    float factor = 30.0f;

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

    // maybe in the future make this less static
    sf::FloatRect playerBox(
        { camera->getCenter().x - 48.f, camera->getCenter().y - 32.f },
        { 96.f, 64.f }
    );

    sf::Vector2f allowedMove = moveWithCollisions(dir, playerBox, collisions);
    camera->move(allowedMove);
    miniMapView->move(allowedMove);

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
