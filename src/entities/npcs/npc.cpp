#include "joanna/entities/npc.h"

NPC::NPC(const sf::FloatRect& box,
         const std::string& name,
         const std::string& buttonTexturePath)
    : Interactable(box, name, buttonTexturePath,
                   "assets/player/npc/joe_base.png") 
{
    animations[Player::State::Idle] =
        Animation("assets/player/npc/joe_stages.png", {96, 64});
}

void NPC::onInteract() {
    Logger::info("NPC {} says: {}", getName().value_or("Unnamed NPC"));
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
    setTexture(anim.texture);
    // sprite->setTextureRect(anim.frames[currentFrame]);
}
