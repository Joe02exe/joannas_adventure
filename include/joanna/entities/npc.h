#pragma once

#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/inventory.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"
#include "nlohmann/json.hpp"
#include <SFML/Graphics.hpp>
#include <deque>
#include <optional>
#include <unordered_map>

using json = nlohmann::json;

class NPC: public Interactable {
  public:
    static json jsonData;
    NPC(const sf::Vector2f& startPos, const std::string& npcIdlePath,
        const std::string& npcWalkingPath, const std::string& buttonTexturePath,
        std::shared_ptr<DialogueBox> dialogueBox, std::string dialogId);

    void interact(Player& player) override;

    void update(float dt, Player& player);

    void setDialogue(const std::vector<std::string>& messages);

    std::shared_ptr<DialogueBox> getDialogueBox() {
        return dialogueBox;
    }

    std::string getUniqueSpriteId() const {
        return uniqueSpriteId;
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
    std::optional<json> pendingMove;
    std::string pendingActionId;
    std::vector<nlohmann::json> sortedDialogue;
    std::function<void(const std::string&)> onAction;

  public:
    void setOnAction(std::function<void(const std::string&)> callback) {
        onAction = std::move(callback);
    }

    void triggerMove(const std::string& actionId);

    std::string getDialogId() const {
        return dialogId;
    }
};