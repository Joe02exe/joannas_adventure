#include "joanna/entities/player.h"
#include <iostream>

#include "joanna/systems/audiomanager.h"
#include "joanna/utils/resourcemanager.h"
#include "joanna/world/tilemanager.h"

Player::Player(
    const std::string& idlePath, const std::string& walkPath,
    const std::string& runPath, const sf::Vector2f& startPos
)
    : Entity(
          sf::FloatRect({ startPos.x - 48, startPos.y - 32 }, { 96, 64 }),
          ResourceManager<sf::Texture>::getInstance()->get(idlePath),
          sf::FloatRect({ startPos.x - 5.f, startPos.y - 2.f }, { 10.f, 9.f }),
          Direction::Right
      ),
      health(20), maxHealth(20), inventory(20) {
    animations[State::Idle] = Animation(idlePath, { 96, 64 }, 9);
    animations[State::Walking] = Animation(walkPath, { 96, 64 }, 8);
    animations[State::Running] = Animation(runPath, { 96, 64 }, 8);

    // Load combat animations
    animations[State::Attack] =
        Animation("assets/player/main/attack.png", { 96, 64 }, 10);
    animations[State::Roll] =
        Animation("assets/player/main/roll.png", { 96, 64 }, 10);
    animations[State::Hurt] =
        Animation("assets/player/main/hurt.png", { 96, 64 }, 8);
    animations[State::Dead] =
        Animation("assets/player/main/dead.png", { 96, 64 }, 13);
    animations[State::Counter] =
        Animation("assets/player/main/punch.png", { 96, 64 }, 3);

    // Initialize attacks
    attacks.push_back({ "Attack", 10, State::Attack });
    attacks.push_back({ "Roll", 15, State::Roll });
}

void Player::update(
    float dt, State& state, bool facingLeft, AudioManager& pManager
) {
    setFacing(facingLeft ? Direction::Left : Direction::Right);
    switchState(state);

    frameTimer += dt;
    const auto& anim = animations[currentState];
    if (frameTimer >= anim.frameTime) {
        if ((state == State::Running || state == State::Walking) &&
            currentFrame % 4 == 0) {
            pManager.play_sfx(SfxId::Footstep);
        }

        frameTimer -= anim.frameTime; // keep leftover time

        if ((currentState == State::Dead || currentState == State::Counter) &&
            currentFrame == anim.frames.size() - 1) {
            // Do not loop dead or counter animation
            if (currentState == State::Counter) {
                switchState(State::Idle);
                state = State::Idle; // Update external state
                std::cout << "Player finished counter animation, switching to "
                             "Idle.\n";
            }
        } else {
            currentFrame =
                (currentFrame + 1) % static_cast<int>(anim.frames.size());
        }

        applyFrame();
    }
}

void Player::applyFrame() {
    const auto& anim = animations[currentState];
    setTexture(anim.texture);
    setFrame(anim.frames[currentFrame]);
}

void Player::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Player::draw(sf::RenderTarget& target) const {
    render(target);
}

void Player::addItemToInventory(
    const Item& item, const std::uint32_t quantity
) {
    inventory.addItem(item, quantity);
}

void Player::takeDamage(int amount) {
    health -= amount;
    health = std::max(health, 0);
    std::cout << "Player took " << amount << " damage. Health: " << health
              << "\n";
}

void Player::displayHealthBar(
    sf::RenderTarget& target, TileManager& tileManager
) const {
    auto heartIcon = tileManager.getTextureById(3052);
    const auto size = target.getView().getSize();
    // const auto center = target.getView().getCenter();
    // const sf::Vector2f startPos(-300.f, 280.f);
    const sf::Vector2f startPos(
        -size.x / 2 + heartIcon.getLocalBounds().size.x,
        -size.y / 2 + heartIcon.getLocalBounds().size.y
    );
    for (int i = 0; i < health / 2; ++i) {
        heartIcon.setPosition({ startPos.x + static_cast<float>(i) * 32.f,
                                startPos.y });
        heartIcon.setScale({ 3.f, 3.f });
        target.draw(heartIcon);
    }
}

void Player::applyItem(std::string& itemId) {
    Logger::info("Applying item with ID: " + itemId);
    if (itemId == "1330") {
        health += 5;
        inventory.removeItem(itemId, 1);
    }
}
