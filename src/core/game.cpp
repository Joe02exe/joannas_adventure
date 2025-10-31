#include "game.h"
#include "../entities/player/player.h"
#include "../entities/utils/controller.h"
#include "./postprocessing.h"
#include "./tilemanager.h"
#include "logger.h"

#include "SFML/Graphics/RenderWindow.hpp"

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

    Controller controller(player, camera);

    camera.setCenter(playerScreenPos);
    camera.zoom(0.25f);

    const float targetAspectRatio =
        static_cast<float>(windowSize.x) / windowSize.y;
    sf::Vector2f viewSize = camera.getSize();

    // Load the map ONCE before the game loop
    TileManager tileManager;
    if (!tileManager.loadMap("./assets/environment/map/map_village.json")) {
        Logger::error("Failed to load map!");
        return;
    }
    Logger::info("Map loaded successfully");

    PostProcessing postProc(900, 900);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u newSize(resized->size.x, resized->size.y);
                float newAspectRatio = static_cast<float>(newSize.x) /
                                       static_cast<float>(newSize.y);

                sf::FloatRect viewport;

                if (newAspectRatio > targetAspectRatio) {
                    // Window is too wide - add letterboxing on sides
                    float width = targetAspectRatio / newAspectRatio;
                    viewport = sf::FloatRect(
                        sf::Vector2f((1.f - width) / 2.f, 0.f),
                        sf::Vector2f(width, 1.f)
                    );
                } else {
                    // Window is too tall - add letterboxing on top/bottom
                    float height = newAspectRatio / targetAspectRatio;
                    viewport = sf::FloatRect(
                        sf::Vector2f(0.f, (1.f - height) / 2.f),
                        sf::Vector2f(1.f, height)
                    );
                }

                camera.setViewport(viewport);
                window.setView(camera);

                postProc.resize(newSize.x, newSize.y);
            }
        }

        controller.getInput(dt, window);

        window.setView(controller.getCamera());

        // Rendering
        postProc.drawScene([&](sf::RenderTarget& rt) {
            tileManager.render(rt);
            controller.getPlayer().draw(rt);
        });

        // Clean up.
        window.clear();
        postProc.apply(window, clock.getElapsedTime().asSeconds());
        window.display();
    }
}