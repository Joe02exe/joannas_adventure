#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/resourcemanager.h"

Interactable::Interactable(
    const sf::FloatRect& box, const std::string& buttonTexturePath,
    const std::string& spriteTexturePath,
    const std::optional<sf::FloatRect>& collisionBox, Direction direction

)
    : Entity(
          box,
          ResourceManager<sf::Texture>::getInstance()->get(spriteTexturePath),
          collisionBox, direction
      ),
      button(box, buttonTexturePath) {}

void Interactable::renderButton(sf::RenderTarget& target) {
    button.render(target);
}

bool Interactable::canPlayerInteract(const sf::Vector2f& playerPos) {
    sf::Vector2f pos = this->getPosition();
    float dx = playerPos.x - pos.x;
    float dy = playerPos.y - pos.y;
    return dx * dx + dy * dy <= interactionDistance * interactionDistance;
}
