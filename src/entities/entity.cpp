#include "joanna/entities/entity.h"
#include "joanna/entities/player.h"
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

void Entity::render(sf::RenderTarget& target) {
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

void Entity::flipFace(const Direction direction) {
    this->direction = direction;
    if (direction == Direction::Right) {
        sprite->setScale({ -1.f, 1.f });
        sprite->setOrigin({ 96.f, 0.f });
    } else {
        sprite->setScale({ 1.f, 1.f });
        sprite->setOrigin({ 0.f, 0.f });
    }
}

std::optional<sf::FloatRect> Entity::getCollisionBox() {
    return collisionBox;
}

std::optional<sf::Vector2f> Entity::getCollisionBoxCenter() {
    if (!collisionBox)
        return std::nullopt;

    const auto& cb = *collisionBox;
    return sf::Vector2f{ cb.position.x + cb.size.x / 2,
                         cb.position.y + cb.size.y / 2 };
}

sf::Vector2f Entity::getPosition() {
    return { boundingBox.position.x + boundingBox.size.x / 2,
             boundingBox.position.y + boundingBox.size.y / 2 };
}

void Entity::setPosition(sf::Vector2f position) {
    sf::Vector2f delta = position - getPosition();
    boundingBox.position += delta;
    sprite->setPosition(boundingBox.position);
    if (!collisionBox) // if entity has no collision, we don't want to set it
        return;
    const auto& cb = *collisionBox;
    collisionBox = sf::FloatRect{ cb.position + delta, cb.size };
}

sf::FloatRect Entity::getBoundingBox() {
    return boundingBox;
}