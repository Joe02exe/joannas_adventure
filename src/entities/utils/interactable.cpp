#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/resourcemanager.h"

Interactable::Interactable(
    const jo::FloatRect& box, const std::string& buttonTexturePath,
    const std::string& spriteTexturePath,
    const std::optional<jo::FloatRect>& collisionBox, Direction direction

)
    : Entity(
          box,
          ResourceManager<jo::Texture>::getInstance()->get(spriteTexturePath),
          collisionBox, direction
      ),
      button(box, buttonTexturePath) {}

void Interactable::renderButton(jo::RenderTarget& target) {
    jo::Vector2f currentPos = getPosition();
    // Offset the button to hover above the interactable entity
    button.setPosition(currentPos + jo::Vector2f(0.f, -16.f));
    button.render(target);
}

bool Interactable::canPlayerInteract(const jo::Vector2f& playerPos) {
    jo::Vector2f pos = this->getPosition();
    float dx = playerPos.x - pos.x;
    float dy = playerPos.y - pos.y;
    // Strict bounding radius (match zoomed camera scale bounds)
    return dx * dx + dy * dy <= 24.f * 24.f;
}
