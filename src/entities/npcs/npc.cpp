#include "joanna/entities/npc.h"

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& npcTexturePath,
    const std::string& buttonTexturePath,
    std::shared_ptr<DialogueBox> dialogueBox
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          buttonTexturePath, npcTexturePath,
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 12 }),
          Direction::Left
      ),
      dialogueBox(dialogueBox) {
    animations[State::Idle] = Animation(npcTexturePath, { 96, 64 });
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
    const Direction direction =
        this->getCollisionBox().has_value() &&
                this->getCollisionBox()->position.x < playerPos.x
            ? Direction::Left
            : Direction::Right;
    flipFace(direction);

    if (dialogueBox && dialogueBox->isActive()) {
        if (auto collisionBox = getCollisionBox()) {
            sf::Vector2f npcCenter(
                collisionBox->position.x + collisionBox->size.x / 2,
                collisionBox->position.y
            );
            dialogueBox->update(dt, npcCenter);
        } // This updates typewriter and bubble geometry
    }
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}
