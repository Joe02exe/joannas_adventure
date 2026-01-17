#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum class Direction { Left, Right };

enum class State { Idle, Walking, Running, Attack, Roll, Hurt, Dead, Mining };

struct Animation {
    sf::Texture texture;
    std::vector<sf::IntRect> frames;
    static constexpr float frameTime = 0.08f;
    // Removed static frameCount

    Animation();
    explicit Animation(
        const std::string& path, const sf::Vector2i& frameSize, int frameCount
    );
};

#include <vector>
#include <cmath>

// Collision helpers
inline bool isColliding(const sf::FloatRect& nextBox, const sf::FloatRect& box) {
    const bool AIsRightToB = nextBox.position.x >= box.position.x + box.size.x;
    const bool AIsLeftToB  = nextBox.position.x + nextBox.size.x <= box.position.x;
    const bool AIsBelowB = nextBox.position.y >= box.position.y + box.size.y;
    const bool AIsAboveB = nextBox.position.y + nextBox.size.y <= box.position.y;
    return !(AIsRightToB || AIsLeftToB || AIsBelowB || AIsAboveB);
}

inline sf::Vector2f moveWithCollisions(
    const sf::Vector2f& dir, const sf::FloatRect& entityBox,
    const std::vector<sf::FloatRect>& collisions
) {
    sf::Vector2f result = dir;
    sf::FloatRect nextX = entityBox;
    nextX.position.x += dir.x;
    sf::FloatRect nextY = entityBox;
    nextY.position.y += dir.y;

    for (const auto& box : collisions) {
        if (isColliding(nextX, box)) {
            result.x = 0.f;
        }
        if (isColliding(nextY, box)) {
            result.y = 0.f;
        }
    }
    return result;
}
