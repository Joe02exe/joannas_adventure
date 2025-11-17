#include "joanna/entities/interactable.h"

Interactable::Interactable(
    const sf::FloatRect& box, const std::optional<std::string>& name,
    const std::string& buttonTexturePath, const std::string& spriteTexturePath,
    const std::optional<sf::FloatRect>& collisionBox

)
    : id(NEXT_ID++), box(box), name(name), button(box, buttonTexturePath),
      collisionBox(collisionBox) {

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
    float dx = playerPos.x - box.position.x;
    float dy = playerPos.y - box.position.y;
    return dx * dx + dy * dy <= interactionDistance * interactionDistance;
}

uint32_t Interactable::getId() const {
    return id;
}

std::optional<std::string> Interactable::getName() const {
    return name;
}

void Interactable::setName(const std::optional<std::string>& newName) {
    name = newName;
}

void Interactable::setFrame(const sf::IntRect& textureRect) {
    sprite->setTextureRect(textureRect);
}

void Interactable::setCollisionBox(const sf::FloatRect& box) {
    collisionBox = box;
}

std::optional<sf::FloatRect> Interactable::getCollisionBox() {
    return collisionBox;
}
