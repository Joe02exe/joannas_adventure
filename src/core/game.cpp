#include "joanna/core/game.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/npc.h"
#include "joanna/entities/player.h"
#include "joanna/systems/controller.h"
#include "joanna/systems/font_renderer.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/world/tilemanager.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "joanna/systems/audiomanager.h"
#include "joanna/utils/resourcemanager.h"

#include <SFML/System/Vector2.hpp>
#include <fstream>
#include <imgui-SFML.h>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Joanna's Farm");

    AudioManager audioManager;
    audioManager.set_current_music(MusicId::Overworld);

    sf::RenderWindow& window = windowManager.getWindow();
    Controller controller(windowManager, audioManager);

    std::list<std::unique_ptr<Entity>> entities;

    FontRenderer fontRenderer("assets/font/Pixellari.ttf");

    std::ifstream file("assets/dialog/dialog.json");
    NPC::jsonData = json::parse(file);
    auto sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);
    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Joe"
    ));
    std::unique_ptr<Entity> enemy = std::make_unique<Enemy>(
        sf::Vector2f{ 710.f, 200.f }, "assets/player/enemies/goblin/idle.png"
    );
    auto* enemyPtr = dynamic_cast<Enemy*>(enemy.get());
    entities.push_back(std::move(enemy));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 160.f, 110.f }, "assets/player/npc/Pirat.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Pirat"
    ));

    TileManager tileManager(window);
    std::vector<sf::FloatRect>& collisions = tileManager.getCollisionRects();
    for (auto& entity : entities) {
        if (auto box = entity->getCollisionBox()) {
            collisions.push_back(*box);
        }
    }

    RenderEngine renderEngine;

    sf::Clock clock;

    Menu menu(windowManager, controller, tileManager, audioManager);
    menu.show(
        renderEngine, tileManager, entities, sharedDialogueBox, audioManager
    );

    clock.restart();

    CombatSystem combatSystem;
    GameStatus gameStatus = GameStatus::Overworld;

    PostProcessing postProc(900, 900);
    while (window.isOpen()) {

        // handle resizing events
        while (auto event = window.pollEvent()) {
            if constexpr (IMGUI_ENABLED) {
                windowManager.getDebugUI().processEvent(window, *event);
            }
            if (const auto* closed = event->getIf<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                windowManager.handleResizeEvent({ resized->size.x,
                                                  resized->size.y });
            }
            if (gameStatus == GameStatus::Combat) {
                combatSystem.handleInput(*event);
            }
        }

        float dt = clock.restart().asSeconds();
        if (dt <= 0.0f) {
            dt = 0.0001f;
        }

        if constexpr (IMGUI_ENABLED) {
            ImGui::SFML::Update(window, sf::seconds(dt));
        }
        if (gameStatus == GameStatus::Overworld) {
            bool resetClock = controller.updateStep(
                dt, window, collisions, entities, sharedDialogueBox,
                tileManager, renderEngine
            );
            if (resetClock) {
                clock.restart();
            }
            if (enemyPtr->updateOverworld(dt, controller.getPlayer(), tileManager)) {
                gameStatus = GameStatus::Combat;
                combatSystem.startCombat(controller.getPlayer(), *enemyPtr);
            }
        } else if (gameStatus == GameStatus::Combat) {
            combatSystem.update(dt);
        }
        if constexpr (IMGUI_ENABLED) {
            windowManager.getDebugUI().update(
                dt, window, controller.getPlayer(), gameStatus, combatSystem,
                *enemyPtr, controller
            );
        }
        windowManager.clear();

        if (gameStatus == GameStatus::Overworld) {
            controller.getPlayerView().setViewport(
                windowManager.getMainView().getViewport()
            );

            postProc.drawScene(
                [&](sf::RenderTarget& target, const sf::View& view) {
                    // world view
                    target.setView(controller.getPlayerView());
                    renderEngine.render(
                        target, controller.getPlayer(), tileManager, entities,
                        sharedDialogueBox, dt
                    );

                    // minimap
                    target.setView(windowManager.getMiniMapView());
                    renderEngine.render(
                        target, controller.getPlayer(), tileManager, entities,
                        sharedDialogueBox, dt
                    );

                    // ui
                    target.setView(windowManager.getUiView());
                    if (controller.renderInventory()) {
                        controller.getPlayer().getInventory().displayInventory(
                            target, tileManager
                        );
                    }
                    controller.getPlayer().displayHealthBar(
                        target, tileManager
                    );
                },
                nullptr
            );
            postProc.apply(window, clock.getElapsedTime().asSeconds());

        } else if (gameStatus == GameStatus::Combat) {
            sf::View combatView(sf::FloatRect({ 0.f, 0.f }, { 900.f, 900.f }));
            combatView.setViewport(windowManager.getMainView().getViewport());
            window.setView(combatView);
            combatSystem.render(window);
        }

        if constexpr (IMGUI_ENABLED) {
            windowManager.render();
            windowManager.getDebugUI().render(window);
        }
        window.display();
    }
    if constexpr (IMGUI_ENABLED) {
        ImGui::SFML::Shutdown();
    }
    ResourceManager<sf::Font>::getInstance()->clear();
    ResourceManager<sf::Texture>::getInstance()->clear();
    ResourceManager<sf::SoundBuffer>::getInstance()->clear();
}