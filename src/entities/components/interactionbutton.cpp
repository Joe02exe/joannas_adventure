#include "joanna/entities/interactionbutton.h"

InteractionButton::InteractionButton(const sf::FloatRect& box,
                                     const std::string& texturePath)
    : box(box), texture(texturePath), sprite(texture) {
    sprite.setPosition({
        box.position.x,
        box.position.y - box.size.y * 0.5f
    });
}

void InteractionButton::render(sf::RenderTarget& target) {
    target.draw(sprite);
}

void InteractionButton::setTexture(const std::string& texturePath) {
    if (!texture.loadFromFile(texturePath)) {
        Logger::error("Failed to load interaction button texture from {}", texturePath);
    }
    sprite.setTexture(texture);
}

sf::Vector2f InteractionButton::getPosition() const {
    return sprite.getPosition();
}
