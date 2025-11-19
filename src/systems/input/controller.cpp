#include "joanna/systems/controller.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/entityutils.h"
#include "joanna/entities/inventory.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/logger.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>
#include <joanna/entities/npc.h>

Controller::Controller(WindowManager& windowManager)
    : windowManager(&windowManager), playerView(windowManager.getMainView()),
      miniMapView(windowManager.getMiniMapView()),
      player(
          "assets/player/main/idle.png", "assets/player/main/walk.png",
          "assets/player/main/run.png", sf::Vector2f{ 150.f, 165.f }
      ) {}

// clang-format off
const bool isColliding(const sf::FloatRect& nextPlayerBox, const sf::FloatRect& box) {
    const bool AIsRightToB = nextPlayerBox.position.x - nextPlayerBox.size.x / 2.f >= box.position.x + box.size.x;
    const bool AIsLeftToB  = nextPlayerBox.position.x + nextPlayerBox.size.x / 2.f <= box.position.x;
     
    // create small illusion of depth (therefore we only user use box.size.y / 2.f once)
    const bool AIsBelowB = nextPlayerBox.position.y >= box.position.y + box.size.y;
    const bool AIsAboveB = nextPlayerBox.position.y + nextPlayerBox.size.y / 2.f <= box.position.y;
    return !(AIsRightToB || AIsLeftToB || AIsBelowB || AIsAboveB);
}


sf::Vector2f moveWithCollisions(
    const sf::Vector2f& dir, const sf::FloatRect& playerBox,
    const std::vector<sf::FloatRect>& collisions
) {

    sf::Vector2f result = dir;

    sf::FloatRect nextX = playerBox;
    nextX.position.x += dir.x;

    sf::FloatRect nextY = playerBox;
    nextY.position.y += dir.y;
    for (const auto& box : collisions) {
        if (isColliding(nextX, box)) {
            result.x = 0.f;
        }
        if (isColliding(nextY, box)) {
            result.y = 0.f;
        }
    }
    return result;
}

// clang-format on

bool Controller::getInput(
    float dt, sf::RenderWindow& window,
    const std::vector<sf::FloatRect>& collisions,
    std::list<std::unique_ptr<Interactable>>& interactables,
    std::shared_ptr<DialogueBox> sharedDialogueBox

) {
    float factor = 30.0f;

    State state = State::Idle;
    sf::Vector2f dir{ 0.f, 0.f };

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
    bool spaceDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (spaceDown && !keyPressed) {
        player.addItemToInventory(Item("test", "test"));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T)) {
        for (auto& entity : interactables) {
            if (entity->canPlayerInteract(player.getPosition())) {
                entity->interact();
            }
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        Menu menu(*windowManager);
        menu.show();
        return true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        if (sharedDialogueBox->isActive() && !sharedDialogueBox->isTyping()) {
            sharedDialogueBox->nextLine(
            ); // Advances to next line or closes dialogue
        }
    }

    bool anyInteractionPoissible = std::any_of(
        interactables.begin(), interactables.end(),
        [this](const auto& obj) {
            if (auto npc = dynamic_cast<NPC*>(obj.get())) {
                return npc->canPlayerInteract(player.getPosition());
            }
            return false;
        }
    );

    if (sharedDialogueBox->isActive() && !anyInteractionPoissible) {
        sharedDialogueBox->hide();
    }

    // normalize the diagonal movement
    if (dir.x != 0.f && dir.y != 0.f) {
        dir *= 0.7071f; // approx 1/sqrt(2)
    }

    // draw the player hitbox
    const sf::FloatRect playerHitBox(
        { player.getPosition().x + 48.f, player.getPosition().y + 32.f },
        { 10.f, 8.f }
    );

    sf::Vector2f nextMove = moveWithCollisions(dir, playerHitBox, collisions);
    playerView.move(nextMove);
    windowManager->getMiniMapView().move(nextMove);

    // subtract half the size of character
    player.setPosition({ playerView.getCenter().x - 48.f,
                         playerView.getCenter().y - 32.f });
    player.update(dt, state, facingLeft);
    keyPressed = spaceDown;
    return false;
}

bool Controller::updateStep(
    float dt, sf::RenderWindow& window, std::vector<sf::FloatRect>& collisions,
    std::list<std::unique_ptr<Interactable>>& interactables,
    std::shared_ptr<DialogueBox> sharedDialogueBox
) {
    // This function can be used for fixed time step updates if needed in future
    for (auto& entity : interactables) {
        if (NPC* npc = dynamic_cast<NPC*>(entity.get())) {
            npc->update(
                dt, State::Idle, false,
                { player.getPosition().x + 48.f, player.getPosition().y + 32.f }
            );
        }
    }
    return getInput(dt, window, collisions, interactables, sharedDialogueBox);
}