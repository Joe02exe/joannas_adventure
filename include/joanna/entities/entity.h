#pragma once

#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactionbutton.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Entity {

  public:
    Entity(
        const sf::FloatRect& box, const sf::Texture& texture,
        const std::optional<sf::FloatRect>& collisionBox = std::nullopt,
        Direction direction = Direction::Right
    );

    virtual ~Entity() = default;

    void render(sf::RenderTarget& target) const;

    uint32_t getId() const;

    void setTexture(const sf::Texture& texture);

    void setFrame(const sf::IntRect& textureRect);

    std::optional<sf::FloatRect> getCollisionBox() const;

    std::optional<sf::Vector2f> getCollisionBoxCenter() const;

    sf::Vector2f getPosition() const;

    void setPosition(const sf::Vector2f& position);

    sf::FloatRect getBoundingBox() const;

    void setFacing(Direction newDirection);

    Direction getFacing() const;

    void setScale(const sf::Vector2f& scale);
    sf::Vector2f getScale() const;

  private:
    static inline uint32_t NEXT_ID = 1;
    const uint32_t id;
    sf::FloatRect boundingBox;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Texture texture;
    std::optional<sf::FloatRect> collisionBox;
    std::optional<Direction> direction;
    sf::Vector2f currentScale = { 1.f, 1.f };
};
