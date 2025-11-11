#include "game.h"

#include "../entities/player/player.h"
#include "../entities/utils/controller.h"
#include "./postprocessing.h"
#include "./tilemanager.h"
#include "./windowmanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Game");

    sf::RenderWindow& window = windowManager.getWindow();
    window.setFramerateLimit(60);
    Controller controller(windowManager);

    TileManager tileManager;
    PostProcessing postProc(900, 900);

    sf::Clock clock;

    while (window.isOpen()) {

        // handle resizing events
        windowManager.pollEvents();

        float dt = clock.restart().asSeconds();

        controller.getInput(dt, window, tileManager.getCollisionRects());

        windowManager.clear();
        controller.getPlayerView().setViewport(
            windowManager.getMainView().getViewport()
        );

        postProc.drawScene([&](sf::RenderTarget& target, const sf::View& view) {
            // world view
            target.setView(controller.getPlayerView());
            tileManager.render(target, controller.getPlayer());

            // minimap
            target.setView(windowManager.getMiniMapView());
            tileManager.render(target, controller.getPlayer());

            // ui
            target.setView(windowManager.getDefaultView());

            sf::Font font;
            if (!font.openFromFile("assets/minecraft.ttf")) {
                throw std::runtime_error("Failed to load font");
            }

            target.setView(windowManager.getDefaultView());

            sf::Text debugText(font);
            debugText.setString("Inventory UI");
            debugText.setCharacterSize(24);
            debugText.setFillColor(sf::Color::White);
            debugText.setStyle(sf::Text::Bold);
            debugText.setPosition(
                { std::floor(target.getView().getCenter().x -
                      debugText.getLocalBounds().size.x / 2.f),
                  std::floor(target.getView().getCenter().y + 
                      target.getSize().y / 2.f - 50.f )});

            target.draw(debugText);

        }, nullptr);

        postProc.apply(window, clock.getElapsedTime().asSeconds());

        window.display();
    }
}