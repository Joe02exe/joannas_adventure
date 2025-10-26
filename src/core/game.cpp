#include "game.h"
#include "../entities/player/player.h"
#include "./tilemanager.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "spdlog/spdlog.h"

#define LOGGING_ENABLED true

Game::Game() = default;

void Game::run() {

    sf::Vector2u windowSize(1200, 900);
    sf::RenderWindow window(
        sf::VideoMode({ windowSize.x, windowSize.y }), "RPG Game"
    );

    sf::Vector2f playerScreenPos(560.f, 850.f);
    Player player(
        "assets/player/main/idle.png", "assets/player/main/walk.png",
        "assets/player/main/run.png", playerScreenPos
    );

    sf::View camera = window.getDefaultView();
    camera.setCenter(playerScreenPos);
    camera.zoom(0.25f);

    TileManager tileManager;
    sf::Clock clock;

    bool facingLeft = false;
    int factor = 3;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        Player::State state = Player::State::Idle;
        float speedMul = 1.f;

        sf::Vector2f dir{ 0.f, 0.f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            dir.x -= 1.f * factor;
            facingLeft = true;
            state = Player::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1.f * factor;
            facingLeft = false;
            state = Player::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            dir.y -= 1.f * factor;
            state = Player::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            dir.y += 1.f * factor;
            state = Player::State::Walking;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            speedMul = 2.f;
            state = Player::State::Running;
        }

        // normalize the diagonal movement
        if (dir.x != 0.f && dir.y != 0.f) {
            dir *= 0.7071f; // approx 1/sqrt(2)
        }

        camera.move(dir * speedMul);
        window.setView(camera);
        player.setPosition(
            { camera.getCenter().x - 48.f, camera.getCenter().y - 32.f }
        ); // subtract half the size of character

        if (LOGGING_ENABLED) {
            spdlog::info(
                "Camera position: x={}, y={}", camera.getCenter().x,
                camera.getCenter().y
            );
        }

        window.clear();
        player.update(dt, state, facingLeft);
        tileManager.loadMap("./assets/environment/map/map.json", window);
        player.draw(window);
        window.display();
    }
}