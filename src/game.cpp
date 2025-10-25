#include "game.h"
#include "tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "spdlog/spdlog.h"

Game::Game() = default;

void Game::run() {

    sf::RenderWindow window(
        sf::VideoMode({ 900, 600 }), "RPG Game"
    );
    
    TileManager tileManager;

    sf::View camera = window.getDefaultView();
    camera.setCenter(sf::Vector2f(560.f, 850.f));
    camera.zoom(.5f);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        bool printLog = false;
        // WASD movement
        sf::Vector2f dir{ 0.f, 0.f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            dir.x -= 1.f;
            printLog = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1.f;
            printLog = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            dir.y -= 1.f;
            printLog = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            dir.y += 1.f;
            printLog = true;
        }

        float speedMul =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ? 2.f : 1.f;

        camera.move(dir * speedMul);
        window.setView(camera);

        if (printLog) {
            spdlog::info(
                "Camera position: x={}, y={}", camera.getCenter().x,
                camera.getCenter().y
            );
            printLog = false;
        }

        window.clear();
        tileManager.loadMap("map.json", window);
        window.display();
    }
}