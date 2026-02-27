#pragma once

#include "joanna/core/graphics.h"
#include "joanna/entities/entityutils.h"
#include <optional>

class Entity {

  public:
    Entity(
        const jo::FloatRect& box, const jo::Texture& texture,
        const std::optional<jo::FloatRect>& collisionBox = std::nullopt,
        Direction direction = Direction::Right
    );

    virtual ~Entity() = default;

    void render(jo::RenderTarget& target) const;

    uint32_t getId() const;

    void setTexture(const jo::Texture& texture);

    void setFrame(const jo::IntRect& textureRect);

    std::optional<jo::FloatRect> getCollisionBox() const;

    std::optional<jo::Vector2f> getCollisionBoxCenter() const;

    jo::Vector2f getPosition() const;

    void setPosition(const jo::Vector2f& position);

    jo::FloatRect getBoundingBox() const;

    void setFacing(Direction newDirection);

    Direction getFacing() const;

    void setScale(const jo::Vector2f& scale);
    jo::Vector2f getScale() const;

  private:
    static inline uint32_t NEXT_ID = 1;
    const uint32_t id;
    jo::FloatRect boundingBox;
    std::unique_ptr<jo::Sprite> sprite;
    const jo::Texture* texture;
    std::optional<jo::FloatRect> collisionBox;
    std::optional<Direction> direction;
    jo::Vector2f currentScale = { 1.f, 1.f };
    jo::IntRect currentTextureRect;
};
