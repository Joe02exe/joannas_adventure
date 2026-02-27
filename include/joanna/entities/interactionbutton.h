#pragma once

#include "joanna/core/graphics.h"
#include <string>

class InteractionButton {
  public:
    InteractionButton(const jo::FloatRect& box, const std::string& texturePath);

    void render(jo::RenderTarget& target);

    void setTexture(const std::string& texturePath);

    jo::Vector2f getPosition() const;

    void setPosition(const jo::Vector2f& pos);

  private:
    jo::FloatRect box;
    std::unique_ptr<jo::Sprite> sprite;
    const jo::Texture* texture;
};
