#include "game.h"

#include "../entities/player/player.h"
#include "../ui/font_renderer.h"
#include "../core/logger.h"
#include "../entities/utils/controller.h"
#include "./postprocessing.h"
#include "./tilemanager.h"
#include "./windowmanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "menu.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Meadowlight");

    sf::RenderWindow& window = windowManager.getWindow();
    window.setFramerateLimit(60);
    Controller controller(windowManager);

    TileManager tileManager;
    PostProcessing postProc(900, 900);

    FontRenderer fontRenderer("assets/font/minecraft.ttf");
    if (!fontRenderer.isLoaded()) {
        Logger::error("Failed to load font for UI rendering");
    }
    
    sf::Clock clock;
    
    Menu menu(windowManager);
    menu.show();

    clock.reset();

    while (window.isOpen()) {

        // handle resizing events
        windowManager.pollEvents();

        float dt = clock.restart().asSeconds();

        bool resetClock = controller.getInput(dt, window, tileManager.getCollisionRects());
        if (resetClock) {
            clock.restart();
        }

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

            // target.setView(windowManager.getDefaultView());
            fontRenderer.drawTextUI(target, "Inventory UI []", { std::floor(target.getView().getCenter().x - 100.f), std::floor(target.getView().getCenter().y + target.getSize().y / 2.f - 50.f) }, 24);
            // sf::Text debugText(fontRenderer.getFont());
            // debugText.setString("Inventory UI");
            // debugText.setCharacterSize(24);
            // debugText.setFillColor(sf::Color::White);
            // debugText.setStyle(sf::Text::Bold);
            // debugText.setPosition(
            //     { std::floor(target.getView().getCenter().x -
            //           debugText.getLocalBounds().size.x / 2.f),
            //       std::floor(target.getView().getCenter().y + 
            //           target.getSize().y / 2.f - 50.f )});

            // target.draw(debugText);

        }, nullptr);

        postProc.apply(window, clock.getElapsedTime().asSeconds());

        window.display();
    }
}