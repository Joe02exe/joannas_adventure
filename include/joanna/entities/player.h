#pragma once

#include "./animation.h"
#include "./inventory.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class Player {
  public:
    enum class State { Idle, Walking, Running };
    enum class Direction { Left, Right };

    Player(
        const std::string& idlePath, const std::string& walkPath,
        const std::string& runPath, const sf::Vector2f& startPos
    );

    void update(float dt, State state, bool movingRight);
    void draw(sf::RenderTarget& target) const;
    void addItemToInventory(const Item& item, std::uint32_t quantity = 1);

    void setPosition(const sf::Vector2f& pos) const;
    sf::Vector2f getPosition() const;

  private:
    std::unique_ptr<sf::Sprite> sprite; // use pointer to avoid initialization
    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    Direction facing = Direction::Right;
    Inventory inventory = Inventory(20);

    float frameTimer = 0.f;
    int currentFrame = 0;

    void switchState(State newState);
    void applyFrame();
    void flipIfNeeded();
};
