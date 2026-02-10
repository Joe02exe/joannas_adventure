#include "joanna/entities/npc.h"
#include <algorithm>
#include <cmath>

json NPC::jsonData;

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& npcIdlePath,
    const std::string& npcWalkingPath,
    const std::string& buttonTexturePath,
    std::shared_ptr<DialogueBox> dialogueBox,
    std::string dialogId
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          buttonTexturePath, npcIdlePath,
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 10 }),
          Direction::Left
      ),
      dialogueBox(dialogueBox),
      dialogId(dialogId) {
    this->uniqueSpriteId = npcIdlePath;
    animations[State::Idle] = Animation(npcIdlePath, { 96, 64 }, 8);
    animations[State::Walking] = Animation(npcWalkingPath, { 96, 64 }, 8);
    auto rawList = jsonData[dialogId];
    
    std::sort(rawList.begin(), rawList.end(), 
        [](const json& a, const json& b) {
            return a["priority"] > b["priority"];
        }
    );

    this->sortedDialogue = rawList;
}

void NPC::setDialogue(const std::vector<std::string>& messages) {
    if (dialogueBox) {
        dialogueBox->setDialogue(messages);
    }
}

std::string createKey(const nlohmann::json& entry, const Player& player, const std::string& uniqueSpriteId) {
    std::string result = "";
    if (entry.contains("id")) {
        result = uniqueSpriteId + "_" + std::string(entry["id"]); 
        if (player.hasInteraction(result)) {
            result = "alreadySeen";
        }
    }
    return result;
}

bool checkRequirements(nlohmann::json& entry, Player& player) {
    bool conditionMet = true;
    if(entry.contains("req") && !entry["req"].is_null()) {
        json req = entry["req"];
        std::string type = req["type"];
        if (type.find("ITEM") != std::string::npos) {
            std::string itemId = req["id"];
            int amount = req["amount"];
            if (player.getInventory().getQuantity(itemId) < amount) {
                conditionMet = false;
            }
            else if(type == "ITEM_REMOVE") {
                player.getInventory().removeItem(itemId, amount);
            }

        }
    }

    return conditionMet;
}

void NPC::interact(Player& player) {
    for(auto& entry : sortedDialogue){
        std::string uniqueKey = createKey(entry, player, this->uniqueSpriteId);
        if (uniqueKey == "alreadySeen") {
            continue;
        }

        bool conditionMet = checkRequirements(entry, player);

        if(conditionMet) {
            dialogueBox->setDialogue(entry["text"], this);
            dialogueBox->show();
            if (!uniqueKey.empty()) {
                player.addInteraction(uniqueKey);
            }
            if(!entry["reward"].is_null()) {
                json reward = entry["reward"];
                pendingReward = Item(reward["id"], reward["name"]);
            }
            move(entry["move"]);
            return;
        }
    }
}

void NPC::update(float dt, Player& player) {
    if (isMoving) {
        if (movementQueue.empty()) {
            isMoving = false;
            switchState(State::Idle);
            return;
        }

        sf::Vector2f target = movementQueue.front();
        
        sf::Vector2f currentPos = getPosition();
        sf::Vector2f diff = target - currentPos;
        float dist = std::hypot(diff.x, diff.y);

        if (dist < 2.0f) {
            setPosition(target);
            movementQueue.pop_front();
        } 
        else {
            sf::Vector2f dir = diff / dist;
            sf::Vector2f velocity = dir * moveSpeed * dt;
            
            setPosition(currentPos + velocity);
            switchState(State::Walking);

            if (velocity.x > 0) setFacing(Direction::Right);
            else if (velocity.x < 0) setFacing(Direction::Left);
        }
    }
    else {
        const Direction direction = this->getPosition().x < player.getPosition().x
                                        ? Direction::Right
                                        : Direction::Left;
        setFacing(direction);
        switchState(State::Idle);
    }
    frameTimer += dt;
    const auto& anim = animations[currentState];

    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime;
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }

    if (dialogueBox && dialogueBox->isActive() && dialogueBox->getOwner() == this) {
        dialogueBox->update(dt, getPosition());
    }

    if (pendingReward.has_value()) {
        if (dialogueBox->getOwner() == this) {
            if (!dialogueBox->isActive() && !dialogueBox->hasMoreLines()) {
                player.addItemToInventory(pendingReward.value(),1);
                pendingReward.reset();
            }
        } else {
            pendingReward.reset();
        }
    }
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}


void NPC::move(const json& moveData) {
    if (!moveData.is_array()) return;
    sf::Vector2f futurePos = getPosition();

    for (const auto& step : moveData) {
        float x = 0.f;
        float y = 0.f;

        if (step.contains("x")) {
            x = step["x"].get<float>();
        }

        if (step.contains("y")) {
            y = step["y"].get<float>();
        }

        futurePos += sf::Vector2f(x, y);
        movementQueue.push_back(futurePos);
    }
    isMoving = true;
}

void NPC::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}