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
      health(200), maxHealth(200), inventory(20) {
    this->animations[State::Idle] = Animation(idlePath, { 96, 64 }, 9);
    this->animations[State::Walking] = Animation(walkPath, { 96, 64 }, 8);
    this->animations[State::Running] = Animation(runPath, { 96, 64 }, 8);

    // Load combat animations
    this->animations[State::Attack] =
        Animation("assets/player/main/attack.png", { 96, 64 }, 10);
    this->animations[State::Roll] =
        Animation("assets/player/main/roll.png", { 96, 64 }, 10);
    this->animations[State::Hurt] =
        Animation("assets/player/main/hurt.png", { 96, 64 }, 8);
    this->animations[State::Dead] =
        Animation("assets/player/main/dead.png", { 96, 64 }, 13);
    this->animations[State::Mining] =
        Animation("assets/player/main/mining.png", { 96, 64 }, 10);
    this->animations[State::Counter] =
        Animation("assets/player/main/punch.png", { 96, 64 }, 3);

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

        if ((this->currentState == State::Dead || this->currentState == State::Counter || this->currentState == State::Mining) &&
            this->currentFrame == anim.frames.size() - 1) {
            // Do not loop dead, counter, or mining animation
            if (this->currentState == State::Counter || this->currentState == State::Mining) {
                this->switchState(State::Idle);
                state = State::Idle; // Update external state
                if (this->currentState == State::Counter) {
                    std::cout << "Player finished counter animation, switching to Idle.\n";
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
    setTexture(anim.texture);
    setFrame(anim.frames[this->currentFrame]);
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

void Player::draw(sf::RenderTarget& target) const {
    this->render(target);
}

void Player::addItemToInventory(
    const Item& item, const std::uint32_t quantity
) {
    if(item.name == "shield") {
        this->stats.defense += 3;
    }
    else if (item.name == "sword") {
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
    sf::RenderTarget& target, TileManager& tileManager
) const {
    auto heartIcon = tileManager.getTextureById(3052);
    const auto size = target.getView().getSize();
    const sf::Vector2f startPos(
        (-size.x / 2) + heartIcon.getLocalBounds().size.x,
        (-size.y / 2) + heartIcon.getLocalBounds().size.y
    );
    for (int i = 0; i < this->health / 20; ++i) {
        heartIcon.setPosition({ startPos.x + (static_cast<float>(i) * 32.f),
                                startPos.y });
        heartIcon.setScale({ 3.f, 3.f });
        target.draw(heartIcon);
    }
}

bool Player::applyItem(const std::string& itemId) {
    if (itemId == "1330") {
        this->health += 5;
        this->inventory.removeItem(itemId, 1);
        return true;
    }
    if (itemId == "703") {
        for (auto& item : this->attacks) {
            if (item.name == "Attack") {
                item.damage += 5;
            }
        }
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