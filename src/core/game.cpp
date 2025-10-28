#include "game.h"
#include "../entities/player/player.h"
#include "./tilemanager.h"
#include "./postprocessing.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "spdlog/spdlog.h"

Game::Game() = default;

void Game::run() {
    sf::Vector2u windowSize(900, 900);
    sf::RenderWindow window(
        sf::VideoMode({ windowSize.x, windowSize.y }), "RPG Game"
    );

    sf::Vector2f playerScreenPos(150.f, 165.f);
    Player player(
        "assets/player/main/idle.png", "assets/player/main/walk.png",
        "assets/player/main/run.png", playerScreenPos
    );

    sf::View camera = window.getDefaultView();
    camera.setCenter(playerScreenPos);
    camera.zoom(0.25f);

    const float targetAspectRatio = static_cast<float>(windowSize.x) / windowSize.y;
    sf::Vector2f viewSize = camera.getSize();

    // Load the map ONCE before the game loop
    TileManager tileManager;
    if (!tileManager.loadMap("./assets/environment/map/map_village.json")) {
        spdlog::error("Failed to load map!");
        return;
    }
    spdlog::info("Map loaded successfully");

    PostProcessing postProc(900, 900);

    sf::Clock clock;
    bool facingLeft = false;
    float factor = 60.0f * 0.5f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u newSize(resized->size.x, resized->size.y);
                float newAspectRatio = static_cast<float>(newSize.x) / static_cast<float>(newSize.y);

                sf::FloatRect viewport;

                if (newAspectRatio > targetAspectRatio) {
                    // Window is too wide - add letterboxing on sides
                    float width = targetAspectRatio / newAspectRatio;
                    viewport = sf::FloatRect(sf::Vector2f((1.f - width) / 2.f, 0.f), sf::Vector2f(width, 1.f));
                } else {
                    // Window is too tall - add letterboxing on top/bottom
                    float height = newAspectRatio / targetAspectRatio;
                    viewport = sf::FloatRect(sf::Vector2f(0.f, (1.f - height) / 2.f), sf::Vector2f(1.f, height));
                }

                camera.setViewport(viewport);
                window.setView(camera);

                postProc.resize(newSize.x, newSize.y);
            }
        }

        // TODO move to separate class.
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

        camera.move(dir);
        window.setView(camera);

        player.setPosition(
            { camera.getCenter().x - 48.f, camera.getCenter().y - 32.f }
        ); // subtract half the size of character

        // TODO don't use if around every log statement, find a better way.
        if (LOGGING_ENABLED) {
            spdlog::info(
                "Camera position: x={}, y={}", camera.getCenter().x,
                camera.getCenter().y
            );
        }

        // Update and draw the player AFTER (foreground)
        player.update(dt, state, facingLeft);

        // Rendering
        postProc.drawScene([&](sf::RenderTarget& rt) {
            tileManager.render(rt);
            player.draw(rt);
        });

        // Clean up.
        window.clear();
        postProc.apply(window, clock.getElapsedTime().asSeconds());
        window.display();
    }
}