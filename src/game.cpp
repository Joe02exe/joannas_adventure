#include "game.h"
#include "./sprites/character.h"
#include "tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "spdlog/spdlog.h"

Game::Game() = default;

void Game::run() {

    sf::RenderWindow window(sf::VideoMode({ 900, 600 }), "RPG Game");

    TileManager tileManager;
    sf::Vector2f playerScreenPos(500.f, 900.f);
    Character player("assets/idle.png", "assets/walk.png", playerScreenPos);

    sf::View camera = window.getDefaultView();
    camera.setCenter(playerScreenPos);
    camera.zoom(0.25f);

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
        bool moving = false;

        sf::Vector2f dir{ 0.f, 0.f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            dir.x -= 1.f * factor;
            printLog = true;
            facingLeft = true;
            moving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1.f * factor;
            printLog = true;
            facingLeft = false;
            moving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            dir.y -= 1.f * factor;
            printLog = true;
            moving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            dir.y += 1.f * factor;
            printLog = true;
            moving = true;
        }

        float speedMul =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ? 2.f : 1.f;

        camera.move(dir * speedMul);
        window.setView(camera);
        player.setPosition(camera.getCenter());

        if (printLog) {
            spdlog::info(
                "Camera position: x={}, y={}", camera.getCenter().x,
                camera.getCenter().y
            );
            printLog = false;
        }

        window.clear();
        player.update(dt, moving, facingLeft);
        tileManager.loadMap("./assets/map.json", window);
        player.draw(window);
        window.display();
    }
}