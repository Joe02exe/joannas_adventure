#pragma once

#include "joanna/entities/entity.h"
#include "joanna/entities/player.h"
#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactionbutton.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Interactable: public Entity {

  public:
    Interactable(
        const sf::FloatRect& box, const std::string& buttonTexturePath,
        const std::string& spriteTexturePath,
        const std::optional<sf::FloatRect>& collisionBox = std::nullopt,
        Direction direction = Direction::Right
    );

    virtual ~Interactable() = default;

    virtual void interact(Player& player) = 0;

    void renderButton(sf::RenderTarget& target);

    bool canPlayerInteract(const sf::Vector2f& playerPos);

  private:
    InteractionButton button;
    const float interactionDistance = 16.f;
};
