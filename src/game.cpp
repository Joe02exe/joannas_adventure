#include "game.h"
#include "./sprites/character.h"
#include "tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "spdlog/spdlog.h"

Game::Game() = default;

void Game::run() {

    sf::Vector2u windowSize(1200, 900);
    sf::RenderWindow window(
        sf::VideoMode({ windowSize.x, windowSize.y }), "RPG Game"
    );

    sf::Vector2f playerScreenPos(560.f, 850.f);
    Character player(
        "assets/idle.png", "assets/walk.png", "assets/run.png", playerScreenPos
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

        bool printLog = false;
        Character::State state = Character::State::Idle;
        float speedMul = 1.f;

        sf::Vector2f dir{ 0.f, 0.f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            dir.x -= 1.f * factor;
            printLog = true;
            facingLeft = true;
            state = Character::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1.f * factor;
            printLog = true;
            facingLeft = false;
            state = Character::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            dir.y -= 1.f * factor;
            printLog = true;
            state = Character::State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            dir.y += 1.f * factor;
            printLog = true;
            state = Character::State::Walking;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            speedMul = 2.f;
            state = Character::State::Running;
        }

        camera.move(dir * speedMul);
        window.setView(camera);
        player.setPosition(
            { camera.getCenter().x - 48.f, camera.getCenter().y - 32.f }
        ); // subtract half the size of character

        if (printLog) {
            spdlog::info(
                "Camera position: x={}, y={}", camera.getCenter().x,
                camera.getCenter().y
            );
            printLog = false;
        }

        window.clear();
        player.update(dt, state, facingLeft);
        tileManager.loadMap("./assets/map.json", window);
        player.draw(window);
        window.display();
    }
}