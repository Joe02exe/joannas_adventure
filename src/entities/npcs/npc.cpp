#include "joanna/entities/npc.h"

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& npcTexturePath,
    const std::string& buttonTexturePath,
    std::shared_ptr<DialogueBox> dialogueBox
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          buttonTexturePath, npcTexturePath,
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 10 }),
          Direction::Left
      ),
      dialogueBox(dialogueBox) {
    animations[State::Idle] = Animation(npcTexturePath, { 96, 64 }, 8);
}

void NPC::setDialogue(const std::vector<std::string>& messages) {
    if (dialogueBox) {
        dialogueBox->setDialogue(messages);
    }
}

void NPC::interact() {
    dialogueBox->setDialogue(
        { "Ostia, didn't see you there traveler!", "My name is Giovannino.",
          "I would like to show you around...",
          "... but someone stole my bike and I want to catch him." }
    );

    dialogueBox->show();
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
                                    ? Direction::Right
                                    : Direction::Left;
    setFacing(direction);

    if (dialogueBox && dialogueBox->isActive()) {
        dialogueBox->update(dt, getPosition());
    }
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}
