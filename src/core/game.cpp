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

    WindowManager windowManager(900, 900, "Joanna's Adventure");

    AudioManager audioManager;
    MusicId currentMusicId = MusicId::Overworld;
    //audioManager.set_current_music(currentMusicId);

    sf::RenderWindow& window = windowManager.getWindow();
    Controller controller(windowManager, audioManager);

    std::list<std::unique_ptr<Entity>> entities;

    FontRenderer fontRenderer("assets/font/Pixellari.ttf");

    std::ifstream file("assets/dialog/dialog.json");
    NPC::jsonData = json::parse(file);
    auto sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);
    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png", "assets/player/npc/guard1_walking.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Joe"
    ));
    std::unique_ptr<Entity> enemy = std::make_unique<Enemy>(
        sf::Vector2f{ 710.f, 200.f }, Enemy::EnemyType::Goblin
    );
    auto* enemyPtr = dynamic_cast<Enemy*>(enemy.get());
    entities.push_back(std::move(enemy));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 160.f, 110.f }, "assets/player/npc/Pirat.png", "assets/player/npc/guard1_walking.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Pirat"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 395.f, 270.f }, "assets/player/npc/guard1.png", "assets/player/npc/guard1_walking.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Guard"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 375.f, 270.f }, "assets/player/npc/guard2.png", "assets/player/npc/guard1_walking.png",
        "assets/buttons/talk_T.png", sharedDialogueBox, "Guard"
    ));

    TileManager tileManager(window);
    std::vector<sf::FloatRect>& collisions = tileManager.getCollisionRects();
    

    RenderEngine renderEngine;

    sf::Clock clock;

    Menu menu(windowManager, controller, tileManager, audioManager);
    menu.show(
        renderEngine, tileManager, entities, sharedDialogueBox, audioManager
    );

    clock.restart();

    CombatSystem combatSystem;
    GameStatus gameStatus = GameStatus::Overworld;

    const auto getRegionMusic = [](const sf::Vector2f& pos) -> MusicId {
        if (pos.x > 540.f) return MusicId::Underworld;
        if (pos.y < 215.f) return MusicId::Beach;
        return MusicId::Overworld;
    };

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
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::F1) {
                    windowManager.getDebugUI().toggle();
                }
            }
        }

        float dt = clock.restart().asSeconds();
        if (dt <= 0.0f) {
            dt = 0.0001f;
        }

        MusicId targetMusic = MusicId::Overworld;
        if (gameStatus == GameStatus::Combat) {
            targetMusic = MusicId::Combat;
        } else {
            targetMusic = getRegionMusic(controller.getPlayer().getPosition());
        }
        if (targetMusic != currentMusicId) {
            currentMusicId = targetMusic;
            audioManager.set_current_music(currentMusicId);
        }

        if constexpr (IMGUI_ENABLED) {
            ImGui::SFML::Update(window, sf::seconds(dt));
            windowManager.getDebugUI().update(
                dt, window, controller.getPlayer(), gameStatus, combatSystem,
                *enemyPtr, controller
            );
        }
        static Enemy* skeletonPtr = nullptr;

        if (gameStatus == GameStatus::Overworld) {
            std::vector<sf::FloatRect> frameCollisions = collisions; 
            for (const auto& entity : entities) {
                if (auto box = entity->getCollisionBox()) {
                    frameCollisions.push_back(*box);
                }
            }

            bool resetClock = controller.updateStep(
                dt, window, frameCollisions, entities, sharedDialogueBox,
                tileManager, renderEngine
            );
            if (resetClock) {
                clock.restart();
            }
            
            // Goblin interaction
            if (enemyPtr && enemyPtr->updateOverworld(dt, controller.getPlayer(), tileManager) == COMBAT_TRIGGERED) {
                gameStatus = GameStatus::Combat;
                combatSystem.startCombat(controller.getPlayer(), *enemyPtr);
            }

            // Skeleton Logic
            if (controller.getPlayer().getInventory().hasItem("3056") && 
                !controller.getPlayer().getInventory().hasItem("3055")) {
                
                if (!skeletonPtr) {
                    auto skeleton = std::make_unique<Enemy>(
                        sf::Vector2f{100.f, 110.f}, Enemy::EnemyType::Skeleton
                    );
                    skeletonPtr = skeleton.get();
                    entities.push_back(std::move(skeleton));
                }

                if (skeletonPtr->updateOverworld(dt, controller.getPlayer(), tileManager) == COMBAT_TRIGGERED) {
                    gameStatus = GameStatus::Combat;
                    combatSystem.startCombat(controller.getPlayer(), *skeletonPtr);
                }

                if (skeletonPtr->isDead()) {
                    controller.getPlayer().getInventory().addItem(Item("3055", "counterAttack"));
                    Logger::info("Skeleton defeated. Counter attack added to inventory.");
                }
            }
        } else if (gameStatus == GameStatus::Combat) {
            combatSystem.update(dt);
            if (combatSystem.battleFinished()) {
                combatSystem.endCombat();
                gameStatus = GameStatus::Overworld;

                if (skeletonPtr && skeletonPtr->isDead() &&
                    !controller.getPlayer().getInventory().hasItem("3055")) {
                    controller.getPlayer().getInventory().addItem(
                        Item("3055", "counterAttack")
                    );
                    Logger::info(
                        "Skeleton defeated. Counter attack added to inventory."
                    );
                }

                entities.remove_if([&](const std::unique_ptr<Entity>& entity) {
                    auto* enemy = dynamic_cast<Enemy*>(entity.get());
                    if (enemy && enemy->isDead()) {
                        if (enemy == enemyPtr)
                            enemyPtr = nullptr;
                        if (enemy == skeletonPtr)
                            skeletonPtr = nullptr;
                        return true;
                    }
                    return false;
                });
            }
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
                    controller.getPlayer().getStats().draw(
                        target, 
                        fontRenderer.getFont(),
                        controller.getPlayer().getCurrentExp(),
                        controller.getPlayer().getExpToNextLevel()
                    );
                },
                nullptr
            );
            postProc.apply(window, clock.getElapsedTime().asSeconds());

        } else if (gameStatus == GameStatus::Combat) {
            sf::View combatView(sf::FloatRect({ 0.f, 0.f }, { 900.f, 900.f }));
            combatView.setViewport(windowManager.getMainView().getViewport());
            window.setView(combatView);
            combatSystem.render(window, tileManager);

            // ui
            window.setView(windowManager.getUiView());
            controller.getPlayer().displayHealthBar(window, tileManager);
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