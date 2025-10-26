#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Character {
  public:
    enum class State { Idle, Walking, Running };
    enum class Direction { Left, Right };

    Character(
        const std::string& idlePath, const std::string& walkPath,
        const std::string& runPath, const sf::Vector2f& startPos
    );

    void update(float dt, State state, bool movingRight);
    void draw(sf::RenderWindow& window);

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

  private:
    struct Animation {
        sf::Texture texture;
        std::vector<sf::IntRect> frames;
        float frameTime = 0.05f;
    };

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
