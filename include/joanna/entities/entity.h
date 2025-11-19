#pragma once

#include "joanna/entities/interactionbutton.h"
#include "joanna/entities/player.h"
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

    void render(sf::RenderTarget& target);

    uint32_t getId() const;

    void setTexture(const sf::Texture& texture);

    void setFrame(const sf::IntRect& textureRect);

    void flipFace(const Direction direction);

    std::optional<sf::FloatRect> getCollisionBox();

    std::optional<sf::Vector2f> getCollisionBoxCenter();

    sf::Vector2f getPosition();

    void setPosition(sf::Vector2f position);

    sf::FloatRect getBox();

  private:
    static inline uint32_t NEXT_ID = 1;
    const uint32_t id;
    sf::FloatRect box;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Texture texture;
    std::optional<sf::FloatRect> collisionBox;
    std::optional<Direction> direction;
};
