#include "joanna/entities/player.h"
#include <iostream>

#include "joanna/systems/audiomanager.h"
#include "joanna/utils/resourcemanager.h"
#include "joanna/world/tilemanager.h"
#include <spdlog/spdlog.h>

Player::Player(
    const std::string& idlePath, const std::string& walkPath,
    const std::string& runPath, const jo::Vector2f& startPos
)
    : Entity(
          jo::FloatRect({ startPos.x - 28, startPos.y - 16 }, { 56, 32 }),
          ResourceManager<jo::Texture>::getInstance()->get(idlePath),
          jo::FloatRect({ startPos.x - 5.f, startPos.y - 2.f }, { 9.6f, 6.5f }),
          Direction::Right
      ),
      health(200), maxHealth(200), inventory(20) {
    this->animations[State::Idle] = Animation(idlePath, { 56, 32 }, 9);
    this->animations[State::Walking] = Animation(walkPath, { 56, 32 }, 8);
    this->animations[State::Running] = Animation(runPath, { 56, 32 }, 8);

    // Load combat animations
    this->animations[State::Attack] =
        Animation("assets/player/main/attack.png", { 56, 32 }, 10);
    this->animations[State::Roll] =
        Animation("assets/player/main/roll.png", { 56, 32 }, 10);
    this->animations[State::Hurt] =
        Animation("assets/player/main/hurt.png", { 56, 32 }, 8);
    this->animations[State::Dead] =
        Animation("assets/player/main/dead.png", { 56, 32 }, 13);
    this->animations[State::Mining] =
        Animation("assets/player/main/mining.png", { 56, 32 }, 10);
    this->animations[State::Counter] =
        Animation("assets/player/main/punch.png", { 56, 32 }, 3);

    // Initialize attacks
    attacks.push_back({ "Attack", 10, State::Attack });
    attacks.push_back({ "Roll", 15, State::Roll });
}

void Player::update(
    float dt, State& state, bool facingLeft, AudioManager& pManager
) {
    setFacing(facingLeft ? Direction::Left : Direction::Right);

    // Only switch state if we are not in a blocking animation
    if (this->currentState != State::Mining) {
        switchState(state);
    }

    this->frameTimer += dt;
    const auto& anim = this->animations[this->currentState];
    if (this->frameTimer >= anim.frameTime) {
        if ((state == State::Running || state == State::Walking) &&
            this->currentFrame % 4 == 0) {
            pManager.play_sfx(SfxId::Footstep);
        }

        this->frameTimer -= anim.frameTime; // keep leftover time

        if ((this->currentState == State::Dead ||
             this->currentState == State::Counter ||
             this->currentState == State::Mining) &&
            this->currentFrame == anim.frames.size() - 1) {
            // Do not loop dead, counter, or mining animation
            if (this->currentState == State::Counter ||
                this->currentState == State::Mining) {
                this->switchState(State::Idle);
                state = State::Idle; // Update external state
                if (this->currentState == State::Counter) {
                    std::cout << "Player finished counter animation, switching "
                                 "to Idle.\n";
                }
            }
        } else {
            this->currentFrame =
                (this->currentFrame + 1) % static_cast<int>(anim.frames.size());
        }

        this->applyFrame();
    }
}

void Player::applyFrame() {
    const auto& anim = animations[this->currentState];
    setTexture(*anim.texture);
    const auto& rect = anim.frames[this->currentFrame];
    setFrame(rect);
}

void Player::switchState(State newState) {
    if (this->currentState != newState) {
        this->currentState = newState;
        this->currentFrame = 0;
        this->frameTimer = 0.f;
        this->applyFrame();
    }
}

void Player::startMining() {
    switchState(State::Mining);
}

void Player::draw(jo::RenderTarget& target) const {
    this->render(target);
}

void Player::addItemToInventory(
    const Item& item, const std::uint32_t quantity
) {
    if (item.name == "shield") {
        this->stats.defense += 3;
    } else if (item.name == "sword") {
        this->stats.attack += 3;
    }
    this->inventory.addItem(item, quantity);
}

void Player::takeDamage(int amount) {
    this->health -= amount;
    this->health = std::max(this->health, 0);
    std::cout << "Player took " << amount << " damage. Health: " << this->health
              << "\n";
}

void Player::displayHealthBar(
    jo::RenderTarget& target, TileManager& tileManager
) const {
    // Cache heart sprites — look up tile IDs only once
    if (!m_heartCached) {
        m_heartSprite = tileManager.getTextureById(3052);
        m_halfHeartSprite = tileManager.getTextureById(3054);
        m_heartCached = true;
    }
    auto heartIcon = *m_heartSprite;
    auto halfHeartIcon = *m_halfHeartSprite;

    const auto size = target.getView().getSize();
    const jo::Vector2f startPos(
        (-size.x / 2) + heartIcon.getLocalBounds().size.x,
        (-size.y / 2) + heartIcon.getLocalBounds().size.y
    );
    int fullHearts = this->health / 20;
    bool showHalfHeart =
        (this->health % 20) >= 10 || (this->health > 0 && fullHearts == 0);

    for (int i = 0; i < fullHearts; ++i) {
        heartIcon.setPosition({ startPos.x + (static_cast<float>(i) * 60.f),
                                startPos.y });
        heartIcon.setScale({ 6.f, 6.f });
        target.draw(heartIcon);
    }

    if (showHalfHeart) {
        halfHeartIcon.setPosition(
            { startPos.x + (static_cast<float>(fullHearts) * 60.f), startPos.y }
        );
        halfHeartIcon.setScale({ 6.f, 6.f });
        target.draw(halfHeartIcon);
    }
}

bool Player::applyItem(const std::string& itemId) {
    if (itemId == "1330") {
        this->health += 50;
        this->inventory.removeItem(itemId, 1);
        return true;
    }
    if (itemId == "703") {
        this->stats.attack += 3;
        this->inventory.removeItem(itemId, 1);
        return true;
    }
    return false;
}

void Player::setHealth(const int newHealth) {
    if (newHealth < 0) {
        this->health = 0;
        return;
    }
    this->health = std::min(newHealth, this->maxHealth);
}

void Player::gainExp(int amount) {
    this->currentExp += amount;

    while (this->currentExp >= this->expToNextLevel) {
        levelUp();
    }
}

void Player::levelUp() {
    this->currentExp -= this->expToNextLevel;

    this->level++;

    this->expToNextLevel = static_cast<int>((float)this->expToNextLevel * 1.2f);

    this->stats.attack += 2;
    this->stats.defense += 1;
    this->health = maxHealth;

    if (levelUpListener) {
        levelUpListener(this->level);
    }
}

void Player::resetStats() {
    stats = Stats(10, 10);
    level = 1;
    currentExp = 0;
    expToNextLevel = 10;
}
