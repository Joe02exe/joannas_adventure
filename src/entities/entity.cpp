#include "joanna/entities/entity.h"
#include "joanna/entities/player.h"
#include "joanna/utils/resourcemanager.h"

Entity::Entity(
    const sf::FloatRect& box, const sf::Texture& texture,
    const std::optional<sf::FloatRect>& collisionBox,
    Player::Direction direction
)
    : id(NEXT_ID++), box(box), texture(texture), collisionBox(collisionBox),
      direction(direction) {

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

void Entity::flipFace(const Player::Direction direction) {
    this->direction = direction;
    if (direction == Player::Direction::Right) {
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
    return sf::Vector2f{ cb.position.x + cb.size.x * 0.5f,
                         cb.position.y + cb.size.y * 0.5f };
}

sf::Vector2f Entity::getPosition() {
    return { box.position.x + box.size.x / 2, box.position.y + box.size.y / 2 };
}

void Entity::setPosition(sf::Vector2f position) {
    box = { { position.x - box.size.x / 2, position.y - box.size.y / 2 },
            box.size };
    sprite->setPosition(box.position);
    if (!collisionBox) // if entity has no collision, we don't want to set it
        return;
    const auto& cb = *collisionBox;
    collisionBox = { { position.x - cb.size.x / 2, position.y - cb.size.y / 2 },
                     collisionBox->size };
}

sf::FloatRect Entity::getBox() {
    return box;
}