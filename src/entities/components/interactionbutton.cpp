#include "joanna/entities/interactionbutton.h"

#include "joanna/utils/resourcemanager.h"

InteractionButton::InteractionButton(
    const sf::FloatRect& box, const std::string& texturePath
)
    : box({ box.position.x + box.size.x / 2 + 3.f,
            box.position.y + box.size.y / 2 + 3.f },
          { 18.f, 19.f }),
      texture(ResourceManager<sf::Texture>::getInstance()->get(texturePath)) {

    sprite = std::make_unique<sf::Sprite>(texture);
    sprite->setScale({ 0.5f, 0.5f });
    sprite->setPosition({ this->box.position.x, this->box.position.y });
}

void InteractionButton::render(sf::RenderTarget& target) {
    target.draw(*sprite);
}

void InteractionButton::setTexture(const std::string& texturePath) {
    texture = ResourceManager<sf::Texture>::getInstance()->get(texturePath);
    sprite->setTexture(texture);
}

sf::Vector2f InteractionButton::getPosition() const {
    return sprite->getPosition();
}
