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
