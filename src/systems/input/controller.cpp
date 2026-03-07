#include "joanna/systems/controller.h"

#include "joanna/core/windowmanager.h"
#include "joanna/entities/entityutils.h"
#include "joanna/entities/inventory.h"
#include "joanna/systems/audiomanager.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/logger.h"

#include "joanna/core/game.h"
#include "joanna/core/graphics.h"
#include "joanna/entities/interactables/stone.h"
#include <algorithm>
#include <joanna/entities/npc.h>
#include <limits>

Controller::Controller(
    WindowManager& windowManager, AudioManager& audioManager, Game& game
)
    : windowManager(windowManager), audioManager(audioManager), game(game),
      player(
          "assets/player/main/idle.png", "assets/player/main/walk.png",
          "assets/player/main/run.png", jo::Vector2f{ 150.f, 400.f }
      ),
      playerView(windowManager.getMainView()),
      miniMapView(windowManager.getMiniMapView()) {
    playerView.setCenter(player.getPosition());
    miniMapView.setCenter(player.getPosition());
}

// clang-format on

bool Controller::getInput(
    float dt, jo::RenderWindow& window,
    const std::vector<jo::FloatRect>& collisions,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& sharedDialogueBox,
    TileManager& tileManager, RenderEngine& renderEngine
) {
    if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::M)) {
        if (!mPressed) {
            showMapOverview = !showMapOverview;
            jo::View& mapView = windowManager.getMapOverviewView();
            mapView.setCenter(this->getPlayer().getPosition());
            mPressed = true;
        }
    } else {
        mPressed = false;
    }

    float factor = 30.0f;

    State state = State::Idle;
    jo::Vector2f dir{ 0.f, 0.f };

    if (player.getState() != State::Mining) {
        if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::A)) {
            dir.x -= factor * dt;
            facingLeft = true;
            state = State::Walking;
        }
        if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::D)) {
            dir.x += factor * dt;
            facingLeft = false;
            state = State::Walking;
        }
        if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::W)) {
            dir.y -= factor * dt;
            state = State::Walking;
        }
        if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::S)) {
            dir.y += factor * dt;
            state = State::Walking;
        }
        if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::LShift)) {
            dir *= 1.5f;
            state = State::Running;
        }
    }

    if (isMapOverviewActive()) {
        windowManager.getMapOverviewView().move(dir);
        auto idleState = State::Idle;
        player.update(dt, idleState, true, audioManager);
        return false;
    }

    // Miyoo L1/R1 Inventory toggle (Debounced)
    bool lDown = jo::Keyboard::isKeyPressed(jo::Keyboard::Key::L);
    bool rDown = jo::Keyboard::isKeyPressed(jo::Keyboard::Key::R);
    if (lDown && !keyPressed) {
        player.getInventory().selectPrevious();
    }
    if (rDown && !keyPressed) {
        player.getInventory().selectNext();
    }

    bool eDown = (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::E));
    if (eDown && !keyPressed) {
        auto id = player.getInventory().getSelectedItemId();
        if (id != "") {
            auto hasApplied = player.applyItem(id);
            if (hasApplied) {
                audioManager.play_sfx(SfxId::Surprise);
            }
        }
    }

    bool spaceDown = jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Space) ||
                     jo::Keyboard::isKeyPressed(jo::Keyboard::Key::T);
    if (spaceDown && !keyPressed) {
        for (const auto& item : tileManager.getRenderObjects()) {
            auto playerPos = player.getPosition();
            auto itemPos = item.position;
            auto item_gid = item.gid;
            float dx = playerPos.x - static_cast<float>(itemPos.x) - 8.f;
            float dy = playerPos.y - static_cast<float>(itemPos.y) - 8.f;
            if (dx * dx + dy * dy <= 16.f * 16.f) {
                if (tileManager.removeObjectById(static_cast<int>(item.id))) {
                    auto map = player.getInventory().mapGidToName();
                    player.gainExp(5);
                    audioManager.play_sfx(SfxId::Collect);
                    player.addItemToInventory(Item(
                        std::to_string(item_gid),
                        map[static_cast<int>(item_gid)]
                    ));
                    break;
                }
            }
        }
    }
    if (spaceDown && !keyPressed && !sharedDialogueBox->isActive()) {
        Interactable* closestInteractable = nullptr;
        float minDistanceSq = std::numeric_limits<float>::max();
        jo::Vector2f playerPos = player.getPosition();

        for (auto& entity : entities) {
            if (auto* interactable =
                    dynamic_cast<Interactable*>(entity.get())) {
                if (interactable->canPlayerInteract(playerPos)) {
                    jo::Vector2f entityPos = entity->getPosition();
                    float dx = playerPos.x - entityPos.x;
                    float dy = playerPos.y - entityPos.y;
                    float distSq = dx * dx + dy * dy;

                    if (distSq < minDistanceSq) {
                        minDistanceSq = distSq;
                        closestInteractable = interactable;
                    }
                }
            }
        }

        if (closestInteractable) {
            closestInteractable->interact(player);
        }
    }

    bool pDown = jo::Keyboard::isKeyPressed(jo::Keyboard::Key::P) ||
                 jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Escape);
    if (pDown && !keyPressed) {
        Menu menu(
            windowManager, *this, tileManager, audioManager, entities, game
        );
        menu.setCanResume(true);
        menu.show(
            renderEngine, tileManager, entities, sharedDialogueBox, audioManager
        );
        return true;
    }

    if (spaceDown && !keyPressed) {
        if (sharedDialogueBox->isActive()) {
            sharedDialogueBox->nextLine();
        }
    }

    bool anyInteractionPoissible =
        std::any_of(entities.begin(), entities.end(), [this](const auto& obj) {
            if (auto* npc = dynamic_cast<NPC*>(obj.get())) {
                return npc->canPlayerInteract(player.getPosition());
            }
            return false;
        });

    if (sharedDialogueBox->isActive() && !anyInteractionPoissible &&
        sharedDialogueBox->getOwner() != nullptr) {
        sharedDialogueBox->hide();
    }

    // normalize the diagonal movement
    if (dir.x != 0.f && dir.y != 0.f) {
        dir *= 0.7071f; // approx 1/sqrt(2)
    }

    jo::Vector2f nextMove = moveWithCollisions(
        dir, player.getCollisionBox().value_or(jo::FloatRect{}), collisions
    );
    playerView.move(nextMove);
    windowManager.getMiniMapView().move(nextMove);
    player.setPosition(player.getPosition() + nextMove);

    player.update(dt, state, facingLeft, audioManager);
    keyPressed = spaceDown || eDown || pDown;
    return false;
}

bool Controller::updateStep(
    float dt, jo::RenderWindow& window, std::vector<jo::FloatRect>& collisions,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& sharedDialogueBox,
    TileManager& tileManager, RenderEngine& renderEngine
) {
    // This function can be used for fixed time step updates if needed in future
    for (auto& entity : entities) {
        if (auto* npc = dynamic_cast<NPC*>(entity.get())) {
            npc->update(dt, player);
        }
        if (auto* stone = dynamic_cast<Stone*>(entity.get())) {
            stone->update(dt, player);
        }
    }

    // Remove destroyed stones
    entities.remove_if([this](const std::unique_ptr<Entity>& e) {
        if (auto* stone = dynamic_cast<Stone*>(e.get())) {
            auto b = stone->shouldBeRemoved();
            if (b) {
                player.addInteraction(stone->getStoneId());
                audioManager.play_sfx(SfxId::Break);
            }
            return b;
        }
        return false;
    });
    return getInput(
        dt, window, collisions, entities, sharedDialogueBox, tileManager,
        renderEngine
    );
}