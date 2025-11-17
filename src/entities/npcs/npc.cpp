#include "joanna/entities/npc.h"

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& npcTexturePath,
    const std::string& buttonTexturePath
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          buttonTexturePath, npcTexturePath,
          sf::FloatRect({ startPos.x - 6, startPos.y - 5 }, { 12, 12 }),
          Player::Direction::Left
      ) {
    animations[Player::State::Idle] = Animation(npcTexturePath, { 96, 64 });
}

void NPC::interact() {
    Logger::info("NPC {} says: {}", getId(), "Hello there!");
}

void NPC::update(
    float dt, Player::State state, bool facingLeft,
    const sf::Vector2f& playerPos
) {
    frameTimer += dt;

    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime;
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
    const Player::Direction direction =
        this->getCollisionBox().has_value() &&
                this->getCollisionBox()->position.x < playerPos.x
            ? Player::Direction::Left
            : Player::Direction::Right;
    flipFace(direction);
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}
