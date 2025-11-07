#include "game.h"

#include "../entities/player/player.h"
#include "../entities/utils/controller.h"
// #include "./postprocessing.h"
#include "./tilemanager.h"
#include "./windowmanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900,900, "Game");

    sf::RenderWindow& window = windowManager.getWindow();

    Controller controller(windowManager);

    TileManager tileManager;

    // PostProcessing postProc(900, 900);

    sf::Clock clock;

    while (window.isOpen()) {

        // handle resizing events
        windowManager.pollEvents();
        
        float dt = clock.restart().asSeconds();
         
        controller.getInput(dt, window);
        
        windowManager.clear();
        controller.getPlayerView().setViewport(windowManager.getMainView().getViewport());

        windowManager.setView(controller.getPlayerView());

        // render map and player in main view
        tileManager.render(windowManager.getWindow());
        controller.getPlayer().draw(windowManager.getWindow());

        windowManager.setView(windowManager.getMiniMapView());

        // render minimap
        tileManager.render(windowManager.getWindow());
        controller.getPlayer().draw(windowManager.getWindow());

        windowManager.setView(windowManager.getDefaultView());
        
        // render text in default view
        // TODO: move to UI manager
        sf::Font font("assets/minecraft.ttf");
        sf::Text debugText(font);
        debugText.setString("Inventory UI");
        debugText.setCharacterSize(24);
        debugText.setFillColor(sf::Color::White);
        debugText.setStyle(sf::Text::Bold);
        debugText.setPosition({window.getView().getCenter().x - debugText.getLocalBounds().size.x / 2,
                              window.getView().getCenter().y + windowManager.getWindow().getSize().y / 2 - 50});
        
        windowManager.getWindow().draw(debugText);

        windowManager.display();
    }
}