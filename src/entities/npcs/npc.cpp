#include "joanna/entities/npc.h"

NPC::NPC(
    const sf::Vector2f& startPos, const std::string& name,
    const std::string& buttonTexturePath
)
    : Interactable(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }), name,
          buttonTexturePath, "assets/player/npc/joe_stages.png",
          sf::FloatRect({ startPos.x - 6, startPos.y - 4 }, { 12, 12 })
      ) {
    animations[Player::State::Idle] =
        Animation("assets/player/npc/joe_stages.png", { 96, 64 });
}

void NPC::interact() {
    Logger::info(
        "NPC {} says: {}", getName().value_or("Unnamed NPC"), "Hello there!"
    );
}

void NPC::update(float dt, Player::State state, bool facingLeft) {
    frameTimer += dt;

    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        frameTimer -= anim.frameTime;
        currentFrame = (currentFrame + 1) % anim.frames.size();
        applyFrame();
    }
}

void NPC::applyFrame() {
    const auto& anim = animations[currentState];
    setFrame(anim.frames[currentFrame]);
}
