#include "joanna/entities/entity.h"

Entity::Entity(
    const jo::FloatRect& box, const jo::Texture& texture,
    const std::optional<jo::FloatRect>& collisionBox, Direction direction
)
    : id(NEXT_ID++), boundingBox(box), texture(&texture), direction(direction),
      collisionBox(collisionBox) {

    sprite = std::make_unique<jo::Sprite>(texture);
    sprite->setPosition(box.position);
}

void Entity::render(jo::RenderTarget& target) const {
    target.draw(*sprite);
}

uint32_t Entity::getId() const {
    return id;
}

void Entity::setTexture(const jo::Texture& newTexture) {
    texture = &newTexture;
    sprite->setTexture(*texture);
}

void Entity::setFrame(const jo::IntRect& textureRect) {
    currentTextureRect = textureRect;
    jo::IntRect rect = textureRect;
    if (direction == Direction::Left) {
        rect.position.x += rect.size.x;
        rect.size.x = -rect.size.x;
    }
    sprite->setTextureRect(rect);
}

std::optional<jo::FloatRect> Entity::getCollisionBox() const {
    return collisionBox;
}

std::optional<jo::Vector2f> Entity::getCollisionBoxCenter() const {
    if (!collisionBox) {
        return std::nullopt;
    }

    const auto& cb = *collisionBox;
    return jo::Vector2f{ cb.position.x + (cb.size.x / 2),
                         cb.position.y + (cb.size.y / 2) };
}

jo::Vector2f Entity::getPosition() const {
    const auto& boundingBox = getBoundingBox();
    return { boundingBox.position.x + (boundingBox.size.x / 2),
             boundingBox.position.y + (boundingBox.size.y / 2) };
}

void Entity::setPosition(const jo::Vector2f& position) {

    const jo::Vector2f delta = position - getPosition();
    sprite->setPosition(getBoundingBox().position + delta);
    if (!collisionBox) { // if entity has no collision, we don't want to set it
        return;
    }
    this->collisionBox =
        jo::FloatRect({ collisionBox->position + delta }, collisionBox->size);
}

jo::FloatRect Entity::getBoundingBox() const {
    return { sprite->getPosition(), boundingBox.size };
}

void Entity::setFacing(Direction newDirection) {
    this->direction = newDirection;
    // re-apply current frame to update flipping
    setFrame(currentTextureRect);
}

Direction Entity::getFacing() const {
    return direction.value_or(Direction::Right);
}

void Entity::setScale(const jo::Vector2f& scale) {
    currentScale = scale;
    sprite->setScale(scale);
    // re-apply facing to update scale and origin
    setFacing(getFacing());
}

jo::Vector2f Entity::getScale() const {
    return currentScale;
}