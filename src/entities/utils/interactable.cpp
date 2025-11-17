#include "joanna/entities/interactable.h"

Interactable::Interactable(
    const sf::FloatRect& box, const std::optional<std::string>& name,
    const std::string& buttonTexturePath, const std::string& spriteTexturePath
)
    : id(NEXT_ID++), boundingBox(box), name(name),
      button(boundingBox, buttonTexturePath) {

    if (!texture.loadFromFile(spriteTexturePath)) {
        Logger::error(
            "Failed to load texture for Interactable {} from path: {}",
            name.value_or("Unnamed"), spriteTexturePath
        );
    }
    sprite = std::make_unique<sf::Sprite>(texture);
    sprite->setPosition({ box.position.x, box.position.y });
}

void Interactable::render(
    sf::RenderTarget& target, bool renderWithInteraction
) {
    target.draw(*sprite);
    if (renderWithInteraction) {
        button.render(target);
    }
}

bool Interactable::canPlayerInteract(const sf::Vector2f& playerPos) const {
    sf::Vector2f center(
        boundingBox.position.x + boundingBox.size.x * 0.5f,
        boundingBox.position.y + boundingBox.size.y * 0.5f
    );

    float dx = playerPos.x - center.x;
    float dy = playerPos.y - center.y;
    return dx * dx + dy * dy <= interactionDistance * interactionDistance;
}

uint32_t Interactable::getId() const {
    return id;
}

std::optional<std::string> Interactable::getName() const {
    return name;
}

sf::FloatRect Interactable::getBoundingBox() const {
    return boundingBox;
}

void Interactable::setName(const std::optional<std::string>& newName) {
    name = newName;
}

void Interactable::setFrame(const sf::IntRect& textureRect) {
    sprite->setTextureRect(textureRect);
}
