#pragma once

#include "./entity.h"
#include "./entityutils.h"
#include "./inventory.h"
#include "joanna/core/combattypes.h"
#include "joanna/systems/audiomanager.h"
#include "joanna/world/tilemanager.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class Player: public Entity {
  public:
    Player(
        const std::string& idlePath, const std::string& walkPath,
        const std::string& runPath, const sf::Vector2f& startPos
    );

    void
    update(float dt, State& state, bool facingLeft, AudioManager& pManager);
    void draw(sf::RenderTarget& target) const;
    void addItemToInventory(const Item& item, std::uint32_t quantity = 1);
    void takeDamage(int amount);
    bool applyItem(const std::string& itemId);
    void
    displayHealthBar(sf::RenderTarget& target, TileManager& tileManager) const;

    int getHealth() const {
        return health;
    }

    void setHealth(int newHealth);

    Inventory& getInventory() {
        return inventory;
    }

  private:
    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    Inventory inventory = Inventory(20);
    std::vector<Attack> attacks;

    float frameTimer = 0.f;
    int currentFrame = 0;
    int health;
    int maxHealth;

    void switchState(State newState);
    void applyFrame();
};
