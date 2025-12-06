#include "joanna/core/game.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/npc.h"
#include "joanna/entities/player.h"
#include "joanna/systems/controller.h"
#include "joanna/systems/font_renderer.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/utils/logger.h"
#include "joanna/world/tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "joanna/core/savegamemanager.h"
#include "joanna/systems/audiomanager.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <imgui-SFML.h>
#include <imgui.h>
#include <fstream>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Joanna's Farm");

    // temporal loading screen (font is still not the same as in the final
    // version)
    sf::Texture loadingTexture;
    if (loadingTexture.loadFromFile("assets/images/loading.gif")) {
        sf::Sprite loadingSprite(loadingTexture);

        // Scale sprite to fit window size
        sf::Vector2u windowSize = windowManager.getWindow().getSize();
        sf::Vector2u textureSize = loadingTexture.getSize();

        float scaleX = static_cast<float>(windowSize.x) /
                       static_cast<float>(textureSize.x);
        float scaleY = static_cast<float>(windowSize.y) /
                       static_cast<float>(textureSize.y);

        loadingSprite.setScale({ scaleX, scaleY });
        loadingSprite.setPosition({ 0.f, 0.f });

        windowManager.getWindow().clear();
        windowManager.getWindow().draw(loadingSprite);
        windowManager.getWindow().display();
    }

    AudioManager audioManager;
    audioManager.set_current_music(MusicId::Overworld);

    sf::RenderWindow& window = windowManager.getWindow();
    Controller controller(windowManager, audioManager);

    std::list<std::unique_ptr<Interactable>> interactables;

    FontRenderer fontRenderer("assets/font/Pixellari.ttf");

    std::ifstream file("assets/dialog/dialog.json");
    NPC::jsonData = json::parse(file);
    auto sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);
    interactables.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Joe"
    ));

    interactables.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 160.f, 110.f },
        "assets/player/npc/Pirat.png",
        "assets/buttons/talk_T.png", 
        sharedDialogueBox,
        "Pirat"
    ));

    TileManager tileManager;
    std::vector<sf::FloatRect>& collisions = tileManager.getCollisionRects();
    for (auto& entity : interactables) {
        if (auto box = entity->getCollisionBox()) {
            collisions.push_back(*box);
        }
    }

    RenderEngine renderEngine;
    PostProcessing postProc(900, 900);

    sf::Clock clock;

    Menu menu(windowManager, controller);
    menu.show(
        renderEngine, tileManager, interactables, sharedDialogueBox,
        audioManager
    );

    SaveGameManager manager;
    GameState state = manager.loadGame();
    Logger::info("Load game");
    Logger::info("Player x: {}", state.player.x);
    Logger::info("Player y: {}", state.player.y);
    controller.getPlayer().setPosition(
        sf::Vector2f(state.player.x, state.player.y)
    );
    controller.getPlayerView().setCenter(
        sf::Vector2f(state.player.x, state.player.y)
    );
    controller.getMiniMapView().setCenter(
        sf::Vector2f(state.player.x, state.player.y)
    );

    Logger::info("Player X {}", controller.getPlayer().getPosition().x);
    Logger::info("Player Y {}", controller.getPlayer().getPosition().y);

    clock.reset();

    while (window.isOpen()) {

        // handle resizing events
        windowManager.pollEvents();

        float dt = clock.restart().asSeconds();

        bool resetClock = controller.updateStep(
            dt, window, collisions, interactables, sharedDialogueBox,
            tileManager, renderEngine
        );
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
                    target, controller.getPlayer(), tileManager, interactables,
                    sharedDialogueBox
                );
                // also draw a blue dot for the player position

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
                    target, controller.getPlayer(), tileManager, interactables,
                    sharedDialogueBox
                );

                // inventory
                target.setView(windowManager.getDefaultView());
                if (controller.renderInventory()) {
                    controller.getPlayer().getInventory().displayInventory(
                        target, tileManager
                    );
                }

                // ui
                fontRenderer.drawTextUI(
                    target, "Inventory UI []",
                    { std::floor(target.getView().getCenter().x - 100.f),
                      std::floor(
                          target.getView().getCenter().y +
                          (static_cast<float>(target.getSize().y) / 2.f) - 50.f
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