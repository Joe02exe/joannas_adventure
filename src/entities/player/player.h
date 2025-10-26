#pragma once

#include "../utils/animation.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Player {
  public:
    enum class State { Idle, Walking, Running };
    enum class Direction { Left, Right };

    Player(
        const std::string& idlePath, const std::string& walkPath,
        const std::string& runPath, const sf::Vector2f& startPos
    );

    void update(float dt, State state, bool movingRight);
    void draw(sf::RenderWindow& window);

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

  private:
    std::unique_ptr<sf::Sprite> sprite; // use pointer to avoid initialization
    std::unordered_map<State, Animation> animations;
    State currentState = State::Idle;
    Direction facing = Direction::Right;

    float frameTimer = 0.f;
    int currentFrame = 0;

    void switchState(State newState);
    void applyFrame();
    void flipIfNeeded();
};
