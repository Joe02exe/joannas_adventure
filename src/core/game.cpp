#include "joanna/core/game.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/interactables/chest.h"
#include "joanna/entities/interactables/stone.h"
#include "joanna/entities/npc.h"
#include "joanna/entities/player.h"
#include "joanna/systems/controller.h"
#include "joanna/systems/font_renderer.h"
#include "joanna/systems/gameover.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/world/tilemanager.h"

#include "joanna/core/graphics.h"
#include "joanna/utils/resourcemanager.h"
#include <fstream>
#include <spdlog/spdlog.h>
#if IMGUI_ENABLED
#include <imgui-SFML.h>
#include <imgui.h>
#endif

#ifdef MIYOO_BUILD
Game::Game()
    : windowManager(640, 480, "Joanna's Adventure"),
      tileManager(windowManager.getWindow()), postProc(640, 480),
      fontRenderer("assets/font/Pixellari.ttf") {
#else
Game::Game()
    : windowManager(900, 900, "Joanna's Adventure"),
      tileManager(windowManager.getWindow()), postProc(900, 900),
      fontRenderer("assets/font/Pixellari.ttf") {
#endif
    initialize();
}

void Game::initialize() {
    audioManager.set_current_music(currentMusicId);
    controller =
        std::make_unique<Controller>(windowManager, audioManager, *this);
    std::ifstream file("assets/dialog/dialog.json");
    NPC::jsonData = json::parse(file);
    sharedDialogueBox = std::make_shared<DialogueBox>(fontRenderer);

    gameOverScreen = std::make_unique<GameOver>(windowManager);
    gameOverScreen->setOnRestart([this]() { this->returnToMenu(); });

    resetEntities();

    controller->getPlayer().onLevelUp([this](int newLevel) {
        std::string msg1 = "You reached level " + std::to_string(newLevel) +
                           ".\n" + "Attack +2, Defense +1 and health restored";
        this->sharedDialogueBox->setDialogue({ msg1 });
        this->sharedDialogueBox->show();
    });

    menu = std::make_unique<Menu>(
        windowManager, *controller, tileManager, audioManager, entities, *this
    );
    menu->show(
        renderEngine, tileManager, entities, sharedDialogueBox, audioManager
    );

    spdlog::info("Game::initialize - Restarting clock before run");
    clock.restart();
    spdlog::info("Game::initialize - Complete.");
}

void Game::run() {
    spdlog::info("Game::run - Entering main loop");
    int frameCount = 0;
    float fpsTimer = 0.f;

#ifdef MIYOO_BUILD
    // Cap at 60 FPS on Miyoo — at 105fps each frame is ~0.28px of movement,
    // causing sub-pixel stutter especially when moving diagonally.
    // At 60fps each frame is ~0.5px, giving visibly smoother movement.
    static constexpr Uint32 FRAME_MS = 1000u / 60u; // ~16ms
    Uint32 lastTick = SDL_GetTicks();
#endif

    while (windowManager.getWindow().isOpen()) {
#ifdef MIYOO_BUILD
        // Frame-rate cap: sleep remaining time until next 60hz slot
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - lastTick;
        if (elapsed < FRAME_MS) {
            SDL_Delay(FRAME_MS - elapsed);
        }
        lastTick = SDL_GetTicks();
#endif

        handleInput();

        float dt = clock.restart().asSeconds();
        if (dt <= 0.0f) {
            dt = 0.0001f;
        }
        // Cap dt to avoid spiral of death on lag spikes
        if (dt > 0.1f)
            dt = 0.1f;

        update(dt);
        render(dt);

        // FPS logging: log once per second instead of every frame
        ++frameCount;
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) {
            float fps = static_cast<float>(frameCount) / fpsTimer;
            spdlog::info(
                "[PERF] FPS={:.1f}  dt_avg={:.2f}ms", fps,
                (fpsTimer / frameCount) * 1000.f
            );
            frameCount = 0;
            fpsTimer = 0.f;
        }
    }
    spdlog::info("Game::run - Exited main loop");

#if IMGUI_ENABLED
    ImGui::SFML::Shutdown();
#endif
    ResourceManager<jo::Font>::getInstance()->clear();
    ResourceManager<jo::Texture>::getInstance()->clear();
    ResourceManager<jo::SoundBuffer>::getInstance()->clear();
}

void Game::resetEntities() {
    spdlog::info("Game::resetEntities - Clearing entities");
    entities.clear();
    spdlog::info("Game::resetEntities - Pushing NPCs");
    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 220.f, 325.f }, "assets/player/npc/joe.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Joe"
    ));

    std::unique_ptr<Entity> enemy = std::make_unique<Enemy>(
        jo::Vector2f{ 710.f, 200.f }, Enemy::EnemyType::Goblin
    );
    enemyPtr = dynamic_cast<Enemy*>(enemy.get());
    entities.push_back(std::move(enemy));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 160.f, 110.f }, "assets/player/npc/Pirat.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Pirat"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 395.f, 270.f }, "assets/player/npc/guard1.png",
        "assets/player/npc/guard1_walking.png", "assets/buttons/interact_A.png",
        sharedDialogueBox, "Guard"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 375.f, 270.f }, "assets/player/npc/guard2.png",
        "assets/player/npc/guard2_walking.png", "assets/buttons/interact_A.png",
        sharedDialogueBox, "Guard"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 500.f, 300.f }, "assets/player/npc/boy1.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Boy"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 520.f, 430.f }, "assets/player/npc/miner.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Miner"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 135.f, 500.f }, "assets/player/npc/swimmer.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Swimmer"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 380.f, 455.f }, "assets/player/npc/girl1.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Girl1"
    ));

    entities.push_back(std::make_unique<NPC>(
        jo::Vector2f{ 105.f, 370.f }, "assets/player/npc/girl2.png",
        "assets/buttons/interact_A.png", sharedDialogueBox, "Girl2"
    ));

    spdlog::info("Game::resetEntities - Pushing interactables");
    entities.push_back(
        std::make_unique<Stone>(jo::Vector2f{ 527.f, 400.f }, "left")
    );
    entities.push_back(
        std::make_unique<Stone>(jo::Vector2f{ 545.f, 400.f }, "right")
    );

    entities.push_back(
        std::make_unique<Chest>(jo::Vector2f{ 652.f, 56.f }, "chest")
    );

    spdlog::info("Game::resetEntities - Wiring Guard callbacks");
    for (auto& entity : entities) {
        if (auto* npc = dynamic_cast<NPC*>(entity.get())) {
            if (npc->getDialogId() == "Guard") {
                npc->setOnAction([this, npc](const std::string& actionId) {
                    for (auto& otherEntity : entities) {
                        if (auto* otherNpc =
                                dynamic_cast<NPC*>(otherEntity.get())) {
                            if (otherNpc != npc &&
                                otherNpc->getDialogId() == "Guard") {
                                otherNpc->triggerMove(actionId);
                                if (this->controller) {
                                    this->controller->getPlayer()
                                        .addInteraction(
                                            otherNpc->getUniqueSpriteId() +
                                            "_" + actionId
                                        );
                                }
                            }
                        }
                    }
                });
            }
        }
    }
}

void Game::handleInput() {
    jo::RenderWindow& window = windowManager.getWindow();
    while (auto event = window.pollEvent()) {
#if IMGUI_ENABLED
        windowManager.getDebugUI().processEvent(window, *event);
#endif
        if (const auto* closed = event->getIf<jo::Event::Closed>()) {
            window.close();
        }
        if (const auto* resized = event->getIf<jo::Event::Resized>()) {
            Game::resize(
                { resized->size.x, resized->size.y }, 1.0f,
                windowManager.getMainView(), window, postProc
            );
            windowManager.handleResizeEvent({ resized->size.x, resized->size.y }
            );
        }
        if (gameStatus == GameStatus::Combat) {
            combatSystem.handleInput(*event);
        } else if (gameStatus == GameStatus::GameOver) {
            gameOverScreen->handleInput(*event);
        }
        if (const auto* keyEvent = event->getIf<jo::Event::KeyPressed>()) {
            if (keyEvent->code == jo::Keyboard::Key::F1) {
                windowManager.getDebugUI().toggle();
            }
        }
    }
}

void Game::update(float dt) {
    // Music logic
    const auto getRegionMusic = [](const jo::Vector2f& pos) -> MusicId {
        if (pos.x > 540.f) {
            return MusicId::Underworld;
        }
        if (pos.y < 215.f) {
            return MusicId::Beach;
        }
        return MusicId::Overworld;
    };

    MusicId targetMusic = MusicId::Overworld;
    if (gameStatus == GameStatus::GameOver) {
        targetMusic = MusicId::GameOver;
    } else if (gameStatus == GameStatus::Combat) {
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

#if IMGUI_ENABLED
    jo::RenderWindow& window = windowManager.getWindow();
    ImGui::SFML::Update(window, jo::seconds(dt));
    if (controller && (enemyPtr != nullptr)) {
        windowManager.getDebugUI().update(
            dt, window, controller->getPlayer(), gameStatus, combatSystem,
            *enemyPtr, *controller
        );
    }
#endif

    if (gameStatus == GameStatus::Overworld) {
        updateOverworld(dt);
    } else if (gameStatus == GameStatus::Combat) {
        updateCombat(dt);
    } else if (gameStatus == GameStatus::GameOver) {
        updateGameOver(dt);
    }
}

void Game::updateOverworld(float dt) {
    if (!controller) {
        return;
    }

    // Static tile collisions never change — rebuild only once, then reuse.
    if (m_tileCollisionsDirty) {
        m_cachedTileCollisions = tileManager.getCollisionRects();
        m_tileCollisionsDirty = false;
    }

    // Gather all possible collisions
    std::vector<jo::FloatRect> frameCollisions;
    frameCollisions.reserve(m_cachedTileCollisions.size() + entities.size());
    frameCollisions.insert(
        frameCollisions.end(), m_cachedTileCollisions.begin(),
        m_cachedTileCollisions.end()
    );
    for (const auto& entity : entities) {
        if (auto box = entity->getCollisionBox()) {
            frameCollisions.push_back(*box);
        }
    }

    // Optimization: Only check collisions for things near the player (within
    // 200 pixels)
    jo::Vector2f playerPos = controller->getPlayer().getPosition();
    float proximityLimit = 200.f;
    // Filter in-place: partition into nearby first (keeps allocation)
    std::vector<jo::FloatRect> nearCollisions;
    nearCollisions.reserve(64);
    for (const auto& rect : frameCollisions) {
        float dx = rect.position.x + rect.size.x / 2.f - playerPos.x;
        float dy = rect.position.y + rect.size.y / 2.f - playerPos.y;
        if (std::abs(dx) < proximityLimit && std::abs(dy) < proximityLimit) {
            nearCollisions.push_back(rect);
        }
    }

    bool resetClock = controller->updateStep(
        dt, windowManager.getWindow(), nearCollisions, entities,
        sharedDialogueBox, tileManager, renderEngine
    );

    if (sharedDialogueBox && sharedDialogueBox->isActive() &&
        sharedDialogueBox->getOwner() == nullptr) {
        sharedDialogueBox->update(dt, controller->getPlayer().getPosition());
    }

    if (resetClock) {
        clock.restart();
    }

    // Goblin interaction
    if ((enemyPtr != nullptr) &&
        enemyPtr->updateOverworld(dt, controller->getPlayer(), tileManager) ==
            COMBAT_TRIGGERED) {
        gameStatus = GameStatus::Combat;
        Logger::info("Goblin fight");
        combatSystem.startCombat(controller->getPlayer(), *enemyPtr);
    }

    // Skeleton Logic
    if (controller->getPlayer().getInventory().hasItemByName("piratToken") &&
        !controller->getPlayer().getInventory().hasItemByName("counterAttack"
        )) {
        if (skeletonPtr == nullptr) {
            auto skeleton = std::make_unique<Enemy>(
                jo::Vector2f{ 100.f, 110.f }, Enemy::EnemyType::Skeleton
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

    if (skeletonSpawnTimer > 0.0f) {
        skeletonSpawnTimer -= dt;
    }

    if (controller->getPlayer().getPosition().y < 200.f &&
        controller->getPlayer().getPosition().x > 200.f &&
        controller->getPlayer().getPosition().x < 400.f) {

        if (randomSkeletonPtr == nullptr && skeletonSpawnTimer <= 0.f &&
            (std::rand() % 3000 < 5)) {
            auto randomSkeleton = std::make_unique<Enemy>(
                jo::Vector2f{ controller->getPlayer().getPosition().x + 15.f,
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
                skeletonSpawnTimer = 10.0f;
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

        if (combatSystem.getState() == CombatState::Defeat) {
            gameStatus = GameStatus::GameOver;
            return;
        }

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
                    Logger::info("Goblin dead");
                    controller->getPlayer().addInteraction("goblinDead");
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
    } else if (gameStatus == GameStatus::GameOver) {
        renderGameOver();
    }

    if constexpr (IMGUI_ENABLED) {
        windowManager.render();
        windowManager.getDebugUI().render(windowManager.getWindow());
    }
    windowManager.getWindow().display();
}

void Game::renderOverworld(float dt) {
    if (!controller) {
        return;
    }

    // High performance rendering strategy:
    // 1. Render the visible game world to a low-resolution texture (matching
    // zoom).
    // 2. Clear only once.
    // 3. Stretch-blit the texture to the screen in a single pass.

    jo::Vector2f viewSize = controller->isMapOverviewActive()
                                ? windowManager.getMapOverviewView().getSize()
                                : controller->getPlayerView().getSize();
    unsigned int pw = static_cast<unsigned int>(viewSize.x);
    unsigned int ph = static_cast<unsigned int>(viewSize.y);

    // Ensure our intermediate buffer matches the needed resolution
    if (postProc.getRenderTexture().getSize().x != pw ||
        postProc.getRenderTexture().getSize().y != ph) {
        postProc.resize(pw, ph);
    }

    // The RenderEngine will now target the low-res texture
    postProc.drawScene(
        [&](jo::RenderTarget& target, const jo::View& view) {
            if (controller->isMapOverviewActive()) {
                jo::View& mapView = windowManager.getMapOverviewView();
                target.setView(mapView);
            } else {
                target.setView(controller->getPlayerView());
            }

            // We must set the viewport of the target view to full (0,0,1,1)
            // since we are drawing to an offscreen buffer that is precisely the
            // right size.
            jo::View currentView = target.getView();
            currentView.setViewport(jo::FloatRect({ 0.f, 0.f }, { 1.f, 1.f }));
            target.setView(currentView);

            renderEngine.render(
                target, controller->getPlayer(), tileManager, entities,
                sharedDialogueBox, dt
            );
        },
        nullptr
    );

    jo::RenderWindow& window = windowManager.getWindow();

    // Reset window to default view for stretching the result back to full
    // screen
    jo::View stretchView(jo::FloatRect(
        { 0.f, 0.f }, { static_cast<float>(pw), static_cast<float>(ph) }
    ));
    window.setView(stretchView);
    postProc.apply(window, clock.getElapsedTime().asSeconds());

    // UI elements should be drawn directly to the window at full resolution
    window.setView(windowManager.getUiView());

    // Draw Health Bar
    controller->getPlayer().displayHealthBar(window, tileManager);

    // Draw Stats
    controller->getPlayer().getStats().draw(
        window, fontRenderer.getFont(), controller->getPlayer().getCurrentExp(),
        controller->getPlayer().getExpToNextLevel()
    );

    if (controller->renderInventory()) {
        controller->getPlayer().getInventory().displayInventory(
            window, tileManager
        );
    }

    if (sharedDialogueBox && sharedDialogueBox->isActive()) {
        sharedDialogueBox->render(window);
    }

    // Minimap is currently disabled to boost FPS (it triggered heavy scaling
    // operations)
    /*
    if (!controller->isMapOverviewActive()) {
        window.setView(windowManager.getMiniMapView());
        renderEngine.render(
            window, controller->getPlayer(), tileManager, entities,
            sharedDialogueBox, dt
        );
    }
    */
}

void Game::resize(
    const jo::Vector2u size, float targetAspectRatio, jo::View& camera,
    jo::RenderWindow& window, PostProcessing& postProc
) {
    // Current aspect ratio
    const float as = static_cast<float>(size.x) / static_cast<float>(size.y);

    if (as >= targetAspectRatio) {
        // window is wider than target
        const float width = targetAspectRatio / as;
        const float x = (1.f - width) / 2.f;
        camera.setViewport(jo::FloatRect({ x, 0.f }, { width, 1.f }));
    } else {
        // window is taller than target
        const float height = as / targetAspectRatio;
        const float y = (1.f - height) / 2.f;
        camera.setViewport(jo::FloatRect({ 0.f, y }, { 1.f, height }));
    }

#if IMGUI_ENABLED
    // update ImGui if needed (handled by processEvent mostly)
#endif

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
    jo::View combatView(jo::FloatRect({ 0.f, 0.f }, { 900.f, 900.f }));
    combatView.setViewport(windowManager.getMainView().getViewport());

    jo::RenderWindow& window = windowManager.getWindow();
    jo::RenderTarget& target = window;

    target.setView(combatView);
    combatSystem.render(target, tileManager, fontRenderer.getFont());

    // ui
    target.setView(windowManager.getUiView());
    controller->getPlayer().displayHealthBar(target, tileManager);
    controller->getPlayer().getStats().draw(
        target, fontRenderer.getFont(), controller->getPlayer().getCurrentExp(),
        controller->getPlayer().getExpToNextLevel()
    );
}

void Game::updateGameOver(float dt) {
    gameOverScreen->update(dt);
}

void Game::renderGameOver() {
    gameOverScreen->render();
}

void Game::returnToMenu() {
    gameStatus = GameStatus::Overworld;
    resetEntities();
    controller->getPlayer().setHealth(controller->getPlayer().getMaxHealth());
    // Reset player position to spawn
    controller->getPlayer().setPosition({ 150.f, 400.f });
    controller->getPlayerView().setCenter({ 150.f, 400.f });
    menu->show(
        renderEngine, tileManager, entities, sharedDialogueBox, audioManager
    );
    clock.restart();
}
