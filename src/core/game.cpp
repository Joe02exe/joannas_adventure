#include "game.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/npc.h"
#include "joanna/entities/player.h"
#include "joanna/systems/controller.h"
#include "joanna/systems/font_renderer.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/logger.h"
#include "joanna/world/tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/System/Vector2.hpp>
#include <cmath>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Joanna's Farm");

    sf::RenderWindow& window = windowManager.getWindow();
    window.setFramerateLimit(60);
    Controller controller(windowManager);

    std::list<std::unique_ptr<Interactable>> interactables;

    interactables.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 100.f }, "player/npc/joe.png", "buttons/talk_T.png"
    ));
    TileManager tileManager;
    std::vector<sf::FloatRect>& collisions = tileManager.getCollisionRects();
    for (auto& entity : interactables) {
        if (entity->getCollisionBox().has_value()) {
            collisions.push_back(entity->getCollisionBox().value());
        }
    }

    RenderEngine renderEngine;
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

        bool resetClock =
            controller.updateStep(dt, window, collisions, interactables);
        if (resetClock) {
            clock.restart();
        }

        windowManager.clear();
        controller.getPlayerView().setViewport(
            windowManager.getMainView().getViewport()
        );

        postProc.drawScene(
            [&](sf::RenderTarget& target, const sf::View& view) {
                // world view
                target.setView(controller.getPlayerView());
                renderEngine.render(
                    target, controller.getPlayer(), tileManager, interactables
                );

                // keep for debugging player hitbox
                // const sf::FloatRect playerHitBox(
                //     { controller.getPlayer().getPosition().x + 48.f,
                //       controller.getPlayer().getPosition().y + 32.f },
                //     { 10.f, 8.f }
                // );
                // // render hitbox as red rectangle for debugging
                // sf::RectangleShape hitboxRect;
                // hitboxRect.setSize({ playerHitBox.size.x, playerHitBox.size.y
                // }
                // );
                // hitboxRect.setPosition({ playerHitBox.position.x,
                //                          playerHitBox.position.y });
                // hitboxRect.setFillColor(sf::Color(255, 0, 0, 50));
                // target.draw(hitboxRect);

                // minimap
                target.setView(windowManager.getMiniMapView());
                renderEngine.render(
                    target, controller.getPlayer(), tileManager, interactables
                );

                // ui
                target.setView(windowManager.getDefaultView());

                // target.setView(windowManager.getDefaultView());
                fontRenderer.drawTextUI(
                    target, "Inventory UI []",
                    { std::floor(target.getView().getCenter().x - 100.f),
                      std::floor(
                          target.getView().getCenter().y +
                          target.getSize().y / 2.f - 50.f
                      ) },
                    24
                );
            },
            nullptr
        );

        postProc.apply(window, clock.getElapsedTime().asSeconds());

        windowManager.getDebugUI().update(dt, window, controller.getPlayer());

        windowManager.render();

        window.display();
    }

    ImGui::SFML::Shutdown();
}