#pragma once

#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <string>

class InteractionButton {
  public:
    InteractionButton(const sf::FloatRect& box, const std::string& texturePath);

    void render(sf::RenderTarget& target);

    void setTexture(const std::string& texturePath);

    sf::Vector2f getPosition() const;

  private:
    const sf::FloatRect box;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Texture texture;
};
