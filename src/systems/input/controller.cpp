#include "joanna/systems/controller.h"

#include "joanna/core/windowmanager.h"
#include "joanna/entities/entityutils.h"
#include "joanna/entities/inventory.h"
#include "joanna/systems/audiomanager.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/logger.h"

#include "joanna/entities/interactables/stone.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>
#include <joanna/entities/npc.h>

Controller::Controller(WindowManager& windowManager, AudioManager& audioManager)
    : windowManager(windowManager), audioManager(audioManager),
      player(
          "assets/player/main/idle.png", "assets/player/main/walk.png",
          "assets/player/main/run.png", sf::Vector2f{ 150.f, 400.f }
      ),
      playerView(windowManager.getMainView()),
      miniMapView(windowManager.getMiniMapView()) {
    playerView.setCenter(player.getPosition());
    miniMapView.setCenter(player.getPosition());
}

// clang-format on

bool Controller::getInput(
    float dt, sf::RenderWindow& window,
    const std::vector<sf::FloatRect>& collisions,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& sharedDialogueBox,
    TileManager& tileManager, RenderEngine& renderEngine

) {
    float factor = 30.0f;

    State state = State::Idle;
    sf::Vector2f dir{ 0.f, 0.f };

    if (player.getState() != State::Mining) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            dir.x -= 1.f * factor * dt;
            facingLeft = true;
            state = State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1.f * factor * dt;
            facingLeft = false;
            state = State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            dir.y -= 1.f * factor * dt;
            state = State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            dir.y += 1.f * factor * dt;
            state = State::Walking;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            dir *= 1.5f;
            state = State::Running;
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M)) {
        if (!mPressed) {
            showMapOverview = !showMapOverview;
            mPressed = true;
        }
    } else {
        mPressed = false;
    }

    // Inventory toggle
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
        player.getInventory().selectSlot(0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
        player.getInventory().selectSlot(1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
        player.getInventory().selectSlot(2);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) {
        player.getInventory().selectSlot(3);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5)) {
        player.getInventory().selectSlot(4);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6)) {
        player.getInventory().selectSlot(5);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7)) {
        player.getInventory().selectSlot(6);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8)) {
        player.getInventory().selectSlot(7);
    }

    bool eDown = (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E));
    if (eDown && !keyPressed) {
        auto id = player.getInventory().getSelectedItemId();
        if (id != "") {
            auto hasApplied = player.applyItem(id);
            if (hasApplied) {
                audioManager.play_sfx(SfxId::Surprise);
            }
        }
    }

    bool spaceDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (spaceDown && !keyPressed) {
        for (const auto& item : tileManager.getRenderObjects()) {
            auto playerPos = player.getPosition();
            auto itemPos = item.position;
            auto item_gid = item.gid;
            float dx = playerPos.x - static_cast<float>(itemPos.x);
            float dy = playerPos.y - static_cast<float>(itemPos.y);
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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T) &&
        !sharedDialogueBox->isActive()) {
        for (auto& entity : entities) {
            if (auto* interactable =
                    dynamic_cast<Interactable*>(entity.get())) {
                if (interactable->canPlayerInteract(player.getPosition())) {
                    interactable->interact(player);
                }
            }
        }
    }

    bool pDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P);
    if (pDown && !keyPressed) {
        Menu menu(windowManager, *this, tileManager, audioManager, entities);
        menu.show(
            renderEngine, tileManager, entities, sharedDialogueBox, audioManager
        );
        return true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        if (sharedDialogueBox->isActive() && !sharedDialogueBox->isTyping()) {
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

    if (sharedDialogueBox->isActive() && !anyInteractionPoissible) {
        sharedDialogueBox->hide();
    }

    // normalize the diagonal movement
    if (dir.x != 0.f && dir.y != 0.f) {
        dir *= 0.7071f; // approx 1/sqrt(2)
    }

    sf::Vector2f nextMove = moveWithCollisions(
        dir, player.getCollisionBox().value_or(sf::FloatRect{}), collisions
    );
    playerView.move(nextMove);
    windowManager.getMiniMapView().move(nextMove);
    player.setPosition(player.getPosition() + nextMove);

    player.update(dt, state, facingLeft, audioManager);
    keyPressed = spaceDown || eDown;
    return false;
}

bool Controller::updateStep(
    float dt, sf::RenderWindow& window, std::vector<sf::FloatRect>& collisions,
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
    entities.remove_if([](const std::unique_ptr<Entity>& e) {
        if (auto* stone = dynamic_cast<Stone*>(e.get())) {
            return stone->shouldBeRemoved();
        }
        return false;
    });
    return getInput(
        dt, window, collisions, entities, sharedDialogueBox, tileManager,
        renderEngine
    );
}