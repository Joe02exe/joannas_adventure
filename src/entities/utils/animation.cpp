#include "animation.h"
#include <spdlog/spdlog.h>

Animation::Animation() = default;

Animation::Animation(const std::string& path, const sf::Vector2i& frameSize) {
    if (!texture.loadFromFile(path)) {
        spdlog::error("Failed to load animation texture: {}", path);
        return;
    }

    frames.reserve(frameCount);
    for (int i = 0; i < frameCount; ++i)
        frames.emplace_back(sf::IntRect({ i * frameSize.x, 0 }, frameSize));
}
