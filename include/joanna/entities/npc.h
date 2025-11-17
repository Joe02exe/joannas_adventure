#include "joanna/entities/interactable.h"
#include <SFML/Graphics.hpp>
#include "./player.h"

class NPC : public Interactable {
public:

    NPC(const sf::FloatRect& box, const std::string& name, const std::string& buttonTexturePath)
        : Interactable(box, name, buttonTexturePath, "assets/player/npc/joe_base.png") {
        animations[Player::State::Idle] = Animation("assets/player/npc/joe_stages.png", {96, 64});
        }

    void onInteract() override {
        Logger::info("NPC {} says: {}", getName().value_or("Unnamed NPC"));
    }

    // void render(sf::RenderTarget& target, bool renderWithInteraction) override {
    //     if (renderWithInteraction) {
    //         target.draw(sprite);
    //     }
    // }

    void update(float dt, Player::State state, bool facingLeft) {
        frameTimer += dt;
        const auto& anim = animations[currentState];
        if (frameTimer >= anim.frameTime) {
            frameTimer -= anim.frameTime; // keep leftover time
            currentFrame = (currentFrame + 1) % anim.frames.size();
            applyFrame();
        }
    }

    void applyFrame() {
        const auto& anim = animations[currentState];
        setTexture(anim.texture);
        //sprite->setTextureRect(anim.frames[currentFrame]);
    }

private:
    Player::State currentState = Player::State::Idle;
    Player::Direction facing = Player::Direction::Right;
    float frameTimer = 0.f;
    int currentFrame = 0;
    std::unordered_map<Player::State, Animation> animations;
};