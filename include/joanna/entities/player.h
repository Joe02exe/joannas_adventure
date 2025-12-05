#pragma once

#include "./entity.h"
#include "./entityutils.h"
#include "./inventory.h"
#include "joanna/systems/audiomanager.h"

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
    update(float dt, State state, bool movingRight, AudioManager& pManager);
    void draw(sf::RenderTarget& target) const;
    void addItemToInventory(const Item& item, std::uint32_t quantity = 1);

    Inventory& getInventory() {
        return inventory;
    }

  private:
    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    Inventory inventory = Inventory(20);

    float frameTimer = 0.f;
    int currentFrame = 0;

    void switchState(State newState);
    void applyFrame();
};
