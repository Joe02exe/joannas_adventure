#include "joanna/entities/entity.h"
#include "joanna/utils/resourcemanager.h"

Entity::Entity(
    const sf::FloatRect& box, const sf::Texture& texture,
    const std::optional<sf::FloatRect>& collisionBox, Direction direction
)
    : id(NEXT_ID++), boundingBox(box), texture(texture), direction(direction),
      collisionBox(collisionBox) {

    sprite = std::make_unique<sf::Sprite>(texture);
    sprite->setPosition(box.position);
}

void Entity::render(sf::RenderTarget& target) const {
    target.draw(*sprite);
}

uint32_t Entity::getId() const {
    return id;
}

void Entity::setTexture(const sf::Texture& texture) {
    sprite->setTexture(texture);
}

void Entity::setFrame(const sf::IntRect& textureRect) {
    sprite->setTextureRect(textureRect);
}

std::optional<sf::FloatRect> Entity::getCollisionBox() const {
    return collisionBox;
}

std::optional<sf::Vector2f> Entity::getCollisionBoxCenter() const {
    if (!collisionBox)
        return std::nullopt;

    const auto& cb = *collisionBox;
    return sf::Vector2f{ cb.position.x + cb.size.x / 2,
                         cb.position.y + cb.size.y / 2 };
}

sf::Vector2f Entity::getPosition() const {
    const auto& boundingBox = getBoundingBox();
    return { boundingBox.position.x + boundingBox.size.x / 2,
             boundingBox.position.y + boundingBox.size.y / 2 };
}

void Entity::setPosition(const sf::Vector2f& position) {
    // Logger::info(
    //     "Setting entity {} position to ({}, {})", id, position.x, position.y
    // );
    const sf::Vector2f delta = position - getPosition();
    sprite->setPosition(getBoundingBox().position + delta);
    if (!collisionBox) // if entity has no collision, we don't want to set it
        return;
    this->collisionBox =
        sf::FloatRect({ collisionBox->position + delta }, collisionBox->size);
}

sf::FloatRect Entity::getBoundingBox() const {
    return { sprite->getPosition(), boundingBox.size };
}

void Entity::setFacing(Direction newDirection) {
    this->direction = newDirection;
    float width = sprite->getLocalBounds().size.x;
    if (newDirection == Direction::Left) {
        sprite->setScale({ -currentScale.x, currentScale.y });
        sprite->setOrigin({ width, 0.f });
    } else {
        sprite->setScale({ currentScale.x, currentScale.y });
        sprite->setOrigin({ 0.f, 0.f });
    }
}

Direction Entity::getFacing() const {
    return direction.value_or(Direction::Right);
}

void Entity::setScale(const sf::Vector2f& scale) {
    currentScale = scale;
    // Re-apply facing to update scale and origin
    setFacing(getFacing());
}

sf::Vector2f Entity::getScale() const {
    return currentScale;
}