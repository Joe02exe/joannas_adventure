#include "joanna/entities/npc.h"
#include <algorithm>

json NPC::jsonData;

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& npcTexturePath,
    const std::string& buttonTexturePath,
    std::shared_ptr<DialogueBox> dialogueBox,
    std::string dialogId
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          buttonTexturePath, npcTexturePath,
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 10 }),
          Direction::Left
      ),
      dialogueBox(dialogueBox),
      dialogId(dialogId) {
    animations[State::Idle] = Animation(npcTexturePath, { 96, 64 });
}

void NPC::setDialogue(const std::vector<std::string>& messages) {
    if (dialogueBox) {
        dialogueBox->setDialogue(messages);
    }
}

void NPC::interact() {
    auto dialogList = jsonData[dialogId];
    std::sort(dialogList.begin(), dialogList.end(), 
        [](const nlohmann::json& a, const nlohmann::json& b) {
            return a["priority"] > b["priority"];
        }
    );

    for(auto& entry : dialogList){
        if(true) {
            dialogueBox->setDialogue(entry["text"]);
            dialogueBox->show();
            return;
        }
        Logger::info(entry["text"]);
    }
}

void NPC::update(
    float dt, State state, bool facingLeft, const sf::Vector2f& playerPos
) {
    frameTimer += dt;

    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime;
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
    const Direction direction = this->getPosition().x < playerPos.x
                                    ? Direction::Left
                                    : Direction::Right;
    flipFace(direction);

    if (dialogueBox && dialogueBox->isActive()) {
        dialogueBox->update(dt, getPosition());
    }
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}
