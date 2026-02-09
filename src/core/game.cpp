#include "joanna/core/game.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/interactables/chest.h"
#include "joanna/entities/interactables/stone.h"
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

Game::Game()
    : windowManager(900, 900, "Joanna's Adventure"),
      tileManager(windowManager.getWindow()), postProc(900, 900),
      fontRenderer("assets/font/Pixellari.ttf") {
    initialize();
}

void Game::initialize() {
    audioManager.set_current_music(currentMusicId);
    controller = std::make_unique<Controller>(windowManager, audioManager);
    std::ifstream file("assets/dialog/dialog.json");
    NPC::jsonData = json::parse(file);
    sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Joe"
    ));

    std::unique_ptr<Entity> enemy = std::make_unique<Enemy>(
        sf::Vector2f{ 710.f, 200.f }, Enemy::EnemyType::Goblin
    );
    enemyPtr = dynamic_cast<Enemy*>(enemy.get());
    entities.push_back(std::move(enemy));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 160.f, 110.f }, "assets/player/npc/Pirat.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Pirat"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 395.f, 270.f }, "assets/player/npc/guard1.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Guard"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 375.f, 270.f }, "assets/player/npc/guard2.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Guard"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 500.f, 300.f }, "assets/player/npc/boy1.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Boy"
    ));

    entities.push_back(std::make_unique<NPC>(
        sf::Vector2f{ 520.f, 430.f }, "assets/player/npc/miner.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_T.png",
        sharedDialogueBox, "Miner"
    ));

    entities.push_back(std::make_unique<Stone>(sf::Vector2f{ 527.f, 400.f }));
    entities.push_back(std::make_unique<Stone>(sf::Vector2f{ 545.f, 400.f }));

    entities.push_back(std::make_unique<Chest>(sf::Vector2f{ 652.f, 56.f }));

    menu = std::make_unique<Menu>(
        windowManager, *controller, tileManager, audioManager, entities
    );
    menu->show(
        renderEngine, tileManager, entities, sharedDialogueBox, audioManager
    );

    clock.restart();
}

void Game::run() {
    while (windowManager.getWindow().isOpen()) {
        handleInput();

        float dt = clock.restart().asSeconds();
        if (dt <= 0.0f) {
            dt = 0.0001f;
        }

        update(dt);
        render(dt);
    }

    if constexpr (IMGUI_ENABLED) {
        ImGui::SFML::Shutdown();
    }
    ResourceManager<sf::Font>::getInstance()->clear();
    ResourceManager<sf::Texture>::getInstance()->clear();
    ResourceManager<sf::SoundBuffer>::getInstance()->clear();
}

void Game::handleInput() {
    sf::RenderWindow& window = windowManager.getWindow();
    while (auto event = window.pollEvent()) {
        if constexpr (IMGUI_ENABLED) {
            windowManager.getDebugUI().processEvent(window, *event);
        }
        if (const auto* closed = event->getIf<sf::Event::Closed>()) {
            window.close();
        }
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            Game::resize(
                { resized->size.x, resized->size.y }, 1.0f,
                windowManager.getMainView(), window, postProc
            );
            windowManager.handleResizeEvent({ resized->size.x, resized->size.y }
            );
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
}

void Game::update(float dt) {
    // Music logic
    const auto getRegionMusic = [](const sf::Vector2f& pos) -> MusicId {
        if (pos.x > 540.f) {
            return MusicId::Underworld;
        }
        if (pos.y < 215.f) {
            return MusicId::Beach;
        }
        return MusicId::Overworld;
    };

    MusicId targetMusic = MusicId::Overworld;
    if (gameStatus == GameStatus::Combat) {
        targetMusic = MusicId::Combat;
    } else {
        if (controller) {
            targetMusic = getRegionMusic(controller->getPlayer().getPosition());
        }
    }

    if (targetMusic != currentMusicId) {
        currentMusicId = targetMusic;
        audioManager.set_current_music(currentMusicId);
    }

    if constexpr (IMGUI_ENABLED) {
        sf::RenderWindow& window = windowManager.getWindow();
        ImGui::SFML::Update(window, sf::seconds(dt));
        if (controller && (enemyPtr != nullptr)) {
            windowManager.getDebugUI().update(
                dt, window, controller->getPlayer(), gameStatus, combatSystem,
                *enemyPtr, *controller
            );
        }
    }

    if (gameStatus == GameStatus::Overworld) {
        updateOverworld(dt);
    } else if (gameStatus == GameStatus::Combat) {
        updateCombat(dt);
    }
}

void Game::updateOverworld(float dt) {
    if (!controller) {
        return;
    }

    std::vector<sf::FloatRect> frameCollisions =
        tileManager.getCollisionRects();
    for (const auto& entity : entities) {
        if (auto box = entity->getCollisionBox()) {
            frameCollisions.push_back(*box);
        }
    }

    bool resetClock = controller->updateStep(
        dt, windowManager.getWindow(), frameCollisions, entities,
        sharedDialogueBox, tileManager, renderEngine
    );
    if (resetClock) {
        clock.restart();
    }

    // Goblin interaction
    if ((enemyPtr != nullptr) &&
        enemyPtr->updateOverworld(dt, controller->getPlayer(), tileManager) ==
            COMBAT_TRIGGERED) {
        gameStatus = GameStatus::Combat;
        combatSystem.startCombat(controller->getPlayer(), *enemyPtr);
    }

    // Skeleton Logic
    if (controller->getPlayer().getInventory().hasItemByName("piratToken") &&
        !controller->getPlayer().getInventory().hasItemByName("counterAttack"
        )) {
        if (skeletonPtr == nullptr) {
            auto skeleton = std::make_unique<Enemy>(
                sf::Vector2f{ 100.f, 110.f }, Enemy::EnemyType::Skeleton
            );
            skeletonPtr = skeleton.get();
            entities.push_back(std::move(skeleton));
        }

        if (skeletonPtr->updateOverworld(
                dt, controller->getPlayer(), tileManager
            ) == COMBAT_TRIGGERED) {
            gameStatus = GameStatus::Combat;
            combatSystem.startCombat(controller->getPlayer(), *skeletonPtr);
        }

        if (skeletonPtr->isDead()) {
            controller->getPlayer().getInventory().addItem(
                Item("3055", "counterAttack")
            );
            Logger::info("Skeleton defeated. Counter attack added to inventory."
            );
        }
    }

    if (controller->getPlayer().getPosition().y < 200.f &&
        controller->getPlayer().getPosition().x > 200.f &&
        controller->getPlayer().getPosition().x < 400.f) {
        if (randomSkeletonPtr == nullptr && (std::rand() % 10000 < 5)) {
            auto randomSkeleton = std::make_unique<Enemy>(
                sf::Vector2f{ controller->getPlayer().getPosition().x + 15.f,
                              controller->getPlayer().getPosition().y },
                Enemy::EnemyType::Skeleton
            );

            randomSkeletonPtr = randomSkeleton.get();

            entities.push_back(std::move(randomSkeleton));
        }

        if (randomSkeletonPtr != nullptr) {
            bool stillExists = false;
            for (const auto& entity : entities) {
                if (entity.get() == randomSkeletonPtr) {
                    stillExists = true;
                    break;
                }
            }

            if (!stillExists) {
                randomSkeletonPtr = nullptr;
            }
        }

        if (randomSkeletonPtr != nullptr) {
            if (randomSkeletonPtr->updateOverworld(
                    dt, controller->getPlayer(), tileManager
                ) == COMBAT_TRIGGERED) {
                gameStatus = GameStatus::Combat;
                combatSystem.startCombat(
                    controller->getPlayer(), *randomSkeletonPtr
                );
            }
        }
    }
}

void Game::updateCombat(float dt) {
    combatSystem.update(dt);
    if (combatSystem.battleFinished()) {
        combatSystem.endCombat();
        gameStatus = GameStatus::Overworld;

        if ((skeletonPtr != nullptr) && skeletonPtr->isDead() && controller &&
            !controller->getPlayer().getInventory().hasItemByName(
                "counterAttack"
            )) {
            controller->getPlayer().getInventory().addItem(
                Item("3055", "counterAttack")
            );
            Logger::info("Skeleton defeated. Counter attack added to inventory."
            );
        } else if (randomSkeletonPtr != nullptr &&
                   randomSkeletonPtr->isDead()) {
            controller->getPlayer().getInventory().addItem(Item("628", "Bone"));
        }

        entities.remove_if([&](const std::unique_ptr<Entity>& entity) {
            auto* enemy = dynamic_cast<Enemy*>(entity.get());
            if (enemy && enemy->isDead()) {
                if (enemy == enemyPtr) {
                    enemyPtr = nullptr;
                }
                if (enemy == skeletonPtr) {
                    skeletonPtr = nullptr;
                }
                return true;
            }
            return false;
        });
    }
}

void Game::render(float dt) {
    windowManager.clear();

    if (gameStatus == GameStatus::Overworld) {
        renderOverworld(dt);
    } else if (gameStatus == GameStatus::Combat) {
        renderCombat();
    }

    if constexpr (IMGUI_ENABLED) {
        windowManager.render();
        windowManager.getDebugUI().render(windowManager.getWindow());
    }
    windowManager.getWindow().display();
}

void Game::renderOverworld(float dt) {
    // Lazy resize check
    if (postProc.getRenderTexture().getSize() !=
        windowManager.getWindow().getSize()) {
        postProc.resize(
            windowManager.getWindow().getSize().x,
            windowManager.getWindow().getSize().y
        );
    }

    if (!controller) {
        return;
    }

    controller->getPlayerView().setViewport(
        windowManager.getMainView().getViewport()
    );

    postProc.drawScene(
        [&](sf::RenderTarget& target, const sf::View& view) {
            if (controller->isMapOverviewActive()) {
                sf::View& mapView = windowManager.getMapOverviewView();
                // Center the map on the player
                mapView.setCenter(controller->getPlayer().getPosition());
                target.setView(mapView);
            } else {
                target.setView(controller->getPlayerView());
            }

            renderEngine.render(
                target, controller->getPlayer(), tileManager, entities,
                sharedDialogueBox, dt
            );

            // minimap
            if (!controller->isMapOverviewActive()) {
                target.setView(windowManager.getMiniMapView());
                renderEngine.render(
                    target, controller->getPlayer(), tileManager, entities,
                    sharedDialogueBox, dt
                );

                // ui
                target.setView(windowManager.getUiView());
                if (controller->renderInventory()) {
                    controller->getPlayer().getInventory().displayInventory(
                        target, tileManager
                    );
                }
                controller->getPlayer().displayHealthBar(target, tileManager);
                controller->getPlayer().getStats().draw(
                    target, fontRenderer.getFont(),
                    controller->getPlayer().getCurrentExp(),
                    controller->getPlayer().getExpToNextLevel()
                );
            }
        },
        nullptr
    );
    sf::View fullView(sf::FloatRect(
        { 0.f, 0.f },
        { static_cast<float>(windowManager.getWindow().getSize().x),
          static_cast<float>(windowManager.getWindow().getSize().y) }
    ));
    windowManager.getWindow().setView(fullView);
    postProc.apply(
        windowManager.getWindow(), clock.getElapsedTime().asSeconds()
    );
}

void Game::resize(
    const sf::Vector2u size, float targetAspectRatio, sf::View& camera,
    sf::RenderWindow& window, PostProcessing& postProc
) {
    // Current aspect ratio
    const float as = static_cast<float>(size.x) / static_cast<float>(size.y);

    if (as >= targetAspectRatio) {
        // window is wider than target
        const float width = targetAspectRatio / as;
        const float x = (1.f - width) / 2.f;
        camera.setViewport(sf::FloatRect({ x, 0.f }, { width, 1.f }));
    } else {
        // window is taller than target
        const float height = as / targetAspectRatio;
        const float y = (1.f - height) / 2.f;
        camera.setViewport(sf::FloatRect({ 0.f, y }, { 1.f, height }));
    }

    if constexpr (IMGUI_ENABLED) {
        // update ImGui if needed (handled by processEvent mostly)
    }

    postProc.resize(size.x, size.y);
    window.setView(camera
    ); // apply view immediately if needed, or MainView will be used later
}

void Game::renderCombat() {
    // Lazy resize check
    if (postProc.getRenderTexture().getSize() !=
        windowManager.getWindow().getSize()) {
        postProc.resize(
            windowManager.getWindow().getSize().x,
            windowManager.getWindow().getSize().y
        );
    }

    // combat view (using main view viewport for correct pillarboxing)
    sf::View combatView(sf::FloatRect({ 0.f, 0.f }, { 900.f, 900.f }));
    combatView.setViewport(windowManager.getMainView().getViewport());

    postProc.drawScene(
        [&](sf::RenderTarget& target, const sf::View& view) {
            target.setView(combatView);
            combatSystem.render(target, tileManager);

            // ui
            target.setView(windowManager.getUiView());
            controller->getPlayer().displayHealthBar(target, tileManager);
            controller->getPlayer().getStats().draw(
                target, fontRenderer.getFont(),
                controller->getPlayer().getCurrentExp(),
                controller->getPlayer().getExpToNextLevel()
            );
        },
        nullptr
    );

    sf::View fullView(sf::FloatRect(
        { 0.f, 0.f },
        { static_cast<float>(windowManager.getWindow().getSize().x),
          static_cast<float>(windowManager.getWindow().getSize().y) }
    ));
    windowManager.getWindow().setView(fullView);
    postProc.apply(
        windowManager.getWindow(), clock.getElapsedTime().asSeconds()
    );
}
