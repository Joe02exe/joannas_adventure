#include "joanna/entities/animation.h"

#include "joanna/utils/resourcemanager.h"

Animation::Animation() = default;

Animation::Animation(const std::string& path, const sf::Vector2i& frameSize)
    : texture(ResourceManager<sf::Texture>::getInstance()->get(path)) {
    frames.reserve(frameCount);
    for (int i = 0; i < frameCount; ++i)
        frames.emplace_back(sf::IntRect({ i * frameSize.x, 0 }, frameSize));
}
