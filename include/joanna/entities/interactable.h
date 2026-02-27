#pragma once

#include "joanna/entities/entity.h"
#include "joanna/entities/player.h"
#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactionbutton.h"
#include "joanna/core/graphics.h"
#include <optional>

class Interactable: public Entity {

  public:
    Interactable(
        const jo::FloatRect& box, const std::string& buttonTexturePath,
        const std::string& spriteTexturePath,
        const std::optional<jo::FloatRect>& collisionBox = std::nullopt,
        Direction direction = Direction::Right
    );

    virtual ~Interactable() = default;

    virtual void interact(Player& player) = 0;

    void renderButton(jo::RenderTarget& target);

    bool canPlayerInteract(const jo::Vector2f& playerPos);

  private:
    InteractionButton button;
    const float interactionDistance = 16.f;
};
