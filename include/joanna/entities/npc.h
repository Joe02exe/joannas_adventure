#pragma once

#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"
#include "nlohmann/json.hpp"
#include "joanna/entities/inventory.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <unordered_map>
#include <deque>

using json = nlohmann::json;

class NPC: public Interactable {
  public:
    static json jsonData;
    NPC(const sf::Vector2f& startPos, const std::string& npcIdlePath,
        const std::string& npcWalkingPath,
        const std::string& buttonTexturePath,
        std::shared_ptr<DialogueBox> dialogueBox,
        std::string dialogId);

    void interact(Player& player) override;

    void update(
        float dt, Player& player
    );

    void setDialogue(const std::vector<std::string>& messages);

    std::shared_ptr<DialogueBox> getDialogueBox() {
        return dialogueBox;
    }

    void applyFrame();

    void move(const json& moveData);

  private:
    State currentState = State::Idle;
    float frameTimer = 0.f;
    int currentFrame = 0;
    sf::Vector2f targetPosition;
    bool isMoving = false;
    float moveSpeed = 40.0f;
    void switchState(State newState);
    std::string uniqueSpriteId;
    std::deque<sf::Vector2f> movementQueue;
    std::unordered_map<State, Animation> animations;
    std::shared_ptr<DialogueBox> dialogueBox;
    std::string dialogId;
    std::optional<Item> pendingReward;
    std::vector<nlohmann::json> sortedDialogue;
};