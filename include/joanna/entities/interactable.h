#pragma once

#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactionbutton.h"
#include "joanna/entities/player.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Interactable {

  public:
    Interactable(
        const sf::FloatRect& box, const std::string& buttonTexturePath,
        const std::string& spriteTexturePath,
        const std::optional<sf::FloatRect>& collisionBox = std::nullopt,
        Direction direction = Direction::Right
    );

    virtual ~Interactable() = default;

    virtual void interact() = 0;

    void render(sf::RenderTarget& target);

    void renderButton(sf::RenderTarget& target);

    bool canPlayerInteract(const sf::Vector2f& playerPos) const;

    uint32_t getId() const;

    void setFrame(const sf::IntRect& textureRect);

    void setCollisionBox(const sf::FloatRect& box);

    void flipFace(const Direction direction);

    std::optional<sf::FloatRect> getCollisionBox();

  private:
    static inline uint32_t NEXT_ID = 1;
    const uint32_t id;
    const sf::FloatRect box;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Texture texture;
    InteractionButton button;
    std::optional<sf::FloatRect> collisionBox;
    std::optional<Direction> direction;
    const float interactionDistance = 16.f;
};
