#pragma once

#include "./entity.h"
#include "./entityutils.h"
#include "./inventory.h"
#include "./stats.h"
#include "joanna/core/combattypes.h"
#include "joanna/systems/audiomanager.h"
#include "joanna/world/tilemanager.h"

#include <SFML/Graphics.hpp>
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
    void startMining();

    State getState() const {
        return currentState;
    }

    int getCurrentFrame() const {
        return currentFrame;
    }

    void draw(sf::RenderTarget& target) const;
    void addItemToInventory(const Item& item, std::uint32_t quantity = 1);
    void takeDamage(int amount);
    bool applyItem(const std::string& itemId);
    void
    displayHealthBar(sf::RenderTarget& target, TileManager& tileManager) const;

    int getHealth() const {
        return health;
    }

    int getMaxHealth() const {
        return maxHealth;
    }

    void setHealth(int newHealth);

    Inventory& getInventory() {
        return inventory;
    }

    Stats& getStats() {
        return stats;
    }

    bool hasInteraction(const std::string& id) const {
        return visitedInteractions.find(id) != visitedInteractions.end();
    }

    void addInteraction(const std::string& id) {
        visitedInteractions.insert(id);
    }

    void gainExp(int amount);
    void levelUp();

    int getLevel() const {
        return level;
    }

    void setLevel(int newLevel) {
        level = newLevel;
    }

    int getCurrentExp() const {
        return currentExp;
    }

    void setCurrentExp(int newExp) {
        currentExp = newExp;
    }

    int getExpToNextLevel() const {
        return expToNextLevel;
    }

    void setExpToNextLevel(int newExpToNextLevel) {
        expToNextLevel = newExpToNextLevel;
    }

    std::unordered_set<std::string> getVisitedInteractions() const {
        return visitedInteractions;
    }

    void resetInteractions() {
        visitedInteractions.clear();
    }

    void resetStats();

    void setInteractions(const std::unordered_set<std::string>& interactions) {
        visitedInteractions = interactions;
    }

    using LevelUpListener = std::function<void(int)>;

    void onLevelUp(const LevelUpListener& listener) {
        levelUpListener = listener;
    }

  private:
    LevelUpListener levelUpListener;
    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    Inventory inventory = Inventory(20);
    Stats stats = Stats(10, 10);
    std::vector<Attack> attacks;

    float frameTimer = 0.f;
    int currentFrame = 0;
    int health;
    int maxHealth;
    int level = 1;
    int currentExp = 0;
    int expToNextLevel = 10;

    void switchState(State newState);
    void applyFrame();
    std::unordered_set<std::string> visitedInteractions;
};
