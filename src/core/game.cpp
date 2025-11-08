#include "game.h"
#include "../entities/player/player.h"
#include "../entities/utils/controller.h"
#include "./postprocessing.h"
#include "./tilemanager.h"
#include "logger.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>

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

    sf::View camera(
        playerScreenPos,
        { static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) }
    );
    sf::View miniMapView(playerScreenPos, { 250.f, 250.f });
    miniMapView.setViewport(sf::FloatRect({ 0.75f, 0.f }, { 0.25f, 0.25f })
    ); // Top-right corner

    Controller controller(player, camera, miniMapView);

    camera.zoom(0.15f);

    const float targetAspectRatio =
        static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    sf::Vector2f viewSize = camera.getSize();

    // Load the map ONCE before the game loop
    TileManager tileManager;
    if (!tileManager.loadMap("./assets/environment/map/map_village3.json")) {
        Logger::error("Failed to load map!");
        return;
    }
    Logger::info("Map loaded successfully");

    // PostProcessing postProc(900, 900);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            //     sf::Vector2u newSize(resized->size.x, resized->size.y);
            //     float newAspectRatio = static_cast<float>(newSize.x) /
            //                            static_cast<float>(newSize.y);

            //     sf::FloatRect viewport;

            //     if (newAspectRatio > targetAspectRatio) {
            //         // Window is too wide - add letterboxing on sides
            //         float width = targetAspectRatio / newAspectRatio;
            //         viewport = sf::FloatRect(
            //             sf::Vector2f((1.f - width) / 2.f, 0.f),
            //             sf::Vector2f(width, 1.f)
            //         );
            //     } else {
            //         // Window is too tall - add letterboxing on top/bottom
            //         float height = newAspectRatio / targetAspectRatio;
            //         viewport = sf::FloatRect(
            //             sf::Vector2f(0.f, (1.f - height) / 2.f),
            //             sf::Vector2f(1.f, height)
            //         );
            //     }

            //     camera.setViewport(viewport);
            //     window.setView(camera);

            //     postProc.resize(newSize.x, newSize.y);
            // }
        }

        // TODO move to WindowManager
        controller.getInput(dt, window, tileManager.getCollisionRects());

        window.clear();

        window.setView(controller.getCamera());

        // render map and player in main view
        tileManager.render(window, controller.getPlayer());

        window.setView(miniMapView);

        // render minimap
        tileManager.render(window, controller.getPlayer());

        window.setView(window.getDefaultView());

        // render text in default view
        sf::Font font("assets/minecraft.ttf");
        sf::Text debugText(font);
        debugText.setString("Inventory UI");
        debugText.setCharacterSize(24);
        debugText.setFillColor(sf::Color::White);
        debugText.setStyle(sf::Text::Bold);
        debugText.setPosition({ window.getView().getCenter().x -
                                    debugText.getLocalBounds().size.x / 2,
                                window.getView().getCenter().y +
                                    windowSize.y / 2 - 50 });

        window.draw(debugText);

        window.display();
    }
}