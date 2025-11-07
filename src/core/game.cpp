#include "game.h"
#include "../entities/player/player.h"
#include "../entities/utils/controller.h"
#include "./postprocessing.h"
#include "./tilemanager.h"
#include "./windowmanager.h"
#include "logger.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900,900, "Game");

    sf::Vector2f playerScreenPos(150.f, 165.f);

    windowManager.setCenter(playerScreenPos);
    
    sf::RenderWindow& window = windowManager.getWindow();

    sf::Vector2u windowSize = window.getSize();

    Player player(
        "assets/player/main/idle.png", "assets/player/main/walk.png",
        "assets/player/main/run.png", playerScreenPos
    );

    sf::View camera = windowManager.getMainView();
    sf::View miniMapView = windowManager.getMiniMapView();

    Controller controller(player, camera, miniMapView);

    const float targetAspectRatio =
        static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    sf::Vector2f viewSize = camera.getSize();

    // Load the map ONCE before the game loop
    TileManager tileManager;
    if (!tileManager.loadMap("./assets/environment/map/map_village.json")) {
        Logger::error("Failed to load map!");
        return;
    }
    Logger::info("Map loaded successfully");

    // PostProcessing postProc(900, 900);

    sf::Clock clock;

    while (window.isOpen()) {
        // handle resizing events
        windowManager.pollEvents();
        float dt = clock.restart().asSeconds();
         
        // TODO move to WindowManager
        controller.getInput(dt, windowManager.getWindow());
        windowManager.getWindow().clear();
        controller.getCamera().setViewport(windowManager.getMainView().getViewport());

        windowManager.getWindow().setView(controller.getCamera());

        // render map and player in main view
        tileManager.render(windowManager.getWindow());
        controller.getPlayer().draw(windowManager.getWindow());

        auto miniMapView = windowManager.getMiniMapView();
        miniMapView.setCenter(
            controller.getCamera().getCenter()
        ); // center minimap on player

        windowManager.getWindow().setView(miniMapView);

        // render minimap
        tileManager.render(windowManager.getWindow());
        controller.getPlayer().draw(windowManager.getWindow());

        windowManager.getWindow().setView(windowManager.getDefaultView());
        
        // render text in default view
        sf::Font font("assets/minecraft.ttf");
        sf::Text debugText(font);
        debugText.setString("Inventory UI");
        debugText.setCharacterSize(24);
        debugText.setFillColor(sf::Color::White);
        debugText.setStyle(sf::Text::Bold);
        debugText.setPosition({window.getView().getCenter().x - debugText.getLocalBounds().size.x / 2,
                              window.getView().getCenter().y + windowSize.y / 2 - 50});
        
        windowManager.getWindow().draw(debugText);

        windowManager.getWindow().display();
    }
}