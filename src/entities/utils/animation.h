#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct Animation {
    sf::Texture texture;
    std::vector<sf::IntRect> frames;
    static constexpr float frameTime = 0.05f;
    static constexpr int frameCount = 8;

    Animation();
    explicit Animation(const std::string& path, const sf::Vector2i& frameSize);
};
