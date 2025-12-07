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
#include "joanna/systems/audiomanager.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <imgui-SFML.h>
#include <imgui.h>

Game::Game() = default;

void Game::run() {

    WindowManager windowManager(900, 900, "Joanna's Farm");

    AudioManager audioManager;
    audioManager.set_current_music(MusicId::Overworld);

    sf::RenderWindow& window = windowManager.getWindow();
    Controller controller(windowManager, audioManager);

    std::list<std::unique_ptr<Entity>> entities;

    FontRenderer fontRenderer("assets/font/Pixellari.ttf");

    auto sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);
    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png",
        "assets/buttons/talk_T.png", sharedDialogueBox
    ));
    std::unique_ptr<Entity> enemy = std::make_unique<Enemy>(sf::Vector2f(720.f, 325.f), "assets/player/enemies/goblin/idle.png");
    Enemy* enemyPtr = dynamic_cast<Enemy*>(enemy.get());
    entities.push_back(std::move(enemy));
    TileManager tileManager;
    std::vector<sf::FloatRect>& collisions = tileManager.getCollisionRects();
    for (auto& entity : entities) {
        if (auto box = entity->getCollisionBox()) {
            collisions.push_back(*box);
        }
    }

    RenderEngine renderEngine;
    PostProcessing postProc(900, 900);

    sf::Clock clock;

    Menu menu(windowManager);
    menu.show();

    clock.reset();

    CombatSystem combatSystem;
    GameState gameState = GameState::Overworld;

    while (window.isOpen()) {

        // handle resizing events
        windowManager.pollEvents([&](const sf::Event& event) {
            if (const auto* closed = event.getIf<sf::Event::Closed>()) {
                window.close();
            }
            if (gameState == GameState::Combat) {
                sf::Event e = event;
                combatSystem.handleInput(e);
            }
        });

        float dt = clock.restart().asSeconds();

        if (gameState == GameState::Overworld) {
            bool resetClock = controller.updateStep(
                dt, window, collisions, entities, sharedDialogueBox
            );
            if (resetClock) {
                clock.restart();
            }
        } else if (gameState == GameState::Combat) {
            combatSystem.update(dt);
        }

        windowManager.clear();

        if (gameState == GameState::Overworld) {
            controller.getPlayerView().setViewport(
                windowManager.getMainView().getViewport()
            );

            postProc.drawScene(
                [&](sf::RenderTarget& target, const sf::View& view) {
                    // world view
                    target.setView(controller.getPlayerView());
                    renderEngine.render(
                        target, controller.getPlayer(), tileManager,
                        entities, sharedDialogueBox
                    );

                    // minimap
                    target.setView(windowManager.getMiniMapView());
                    renderEngine.render(
                        target, controller.getPlayer(), tileManager,
                        entities, sharedDialogueBox
                    );

                    // ui
                    target.setView(windowManager.getDefaultView());

                    fontRenderer.drawTextUI(
                        target, "Inventory UI []",
                        { std::floor(target.getView().getCenter().x - 100.f),
                          std::floor(
                              target.getView().getCenter().y +
                              (static_cast<float>(target.getSize().y) / 2.f) -
                              50.f
                          ) },
                        24
                    );
                },
                nullptr
            );
            postProc.apply(window, clock.getElapsedTime().asSeconds());
        } else if (gameState == GameState::Combat) {
            sf::View combatView(sf::FloatRect({ 0.f, 0.f }, { 900.f, 900.f }));
            combatView.setViewport(windowManager.getMainView().getViewport());
            window.setView(combatView);
            combatSystem.render(window);
        }

        windowManager.getDebugUI().update(
            dt, window, controller.getPlayer(), gameState, combatSystem, *enemyPtr
        );

        windowManager.render();

        window.display();
    }

    ImGui::SFML::Shutdown();
}