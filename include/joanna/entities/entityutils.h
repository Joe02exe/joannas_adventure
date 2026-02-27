#pragma once

#include "joanna/core/graphics.h"
#include <string>
#include <vector>

enum class Direction { Left, Right };

enum class State {
    Idle,
    Walking,
    Running,
    Attack,
    Roll,
    Hurt,
    Dead,
    Mining,
    Counter
};

struct Animation {
    const jo::Texture* texture;
    std::vector<jo::IntRect> frames;
    static constexpr float frameTime = 0.12f;
    // Removed static frameCount

    Animation();
    explicit Animation(
        const std::string& path, const jo::Vector2i& frameSize, int frameCount
    );
};

#include <cmath>
#include <vector>

// Collision helpers
inline bool
isColliding(const jo::FloatRect& nextBox, const jo::FloatRect& box) {
    const bool AIsRightToB = nextBox.position.x >= box.position.x + box.size.x;
    const bool AIsLeftToB =
        nextBox.position.x + nextBox.size.x <= box.position.x;
    const bool AIsBelowB = nextBox.position.y >= box.position.y + box.size.y;
    const bool AIsAboveB =
        nextBox.position.y + nextBox.size.y <= box.position.y;
    return !(AIsRightToB || AIsLeftToB || AIsBelowB || AIsAboveB);
}

inline jo::Vector2f moveWithCollisions(
    const jo::Vector2f& dir, const jo::FloatRect& entityBox,
    const std::vector<jo::FloatRect>& collisions
) {
    jo::Vector2f result = dir;
    jo::FloatRect nextX = entityBox;
    nextX.position.x += dir.x;
    jo::FloatRect nextY = entityBox;
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
