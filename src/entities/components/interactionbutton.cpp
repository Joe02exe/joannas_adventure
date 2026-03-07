#include "joanna/entities/interactionbutton.h"

#include "joanna/utils/resourcemanager.h"

InteractionButton::InteractionButton(
    const jo::FloatRect& box, const std::string& texturePath
)
    : box({ box.position.x + box.size.x / 2 + 3.f,
            box.position.y + box.size.y / 2 + 3.f },
          { 18.f, 19.f }),
      texture(&ResourceManager<jo::Texture>::getInstance()->get(texturePath)) {

    sprite = std::make_unique<jo::Sprite>(*texture);
    const_cast<jo::Texture*>(texture)->setSmooth(false
    ); // Ensure pixel art remains sharp when scaled
    sprite->setPosition({ this->box.position.x, this->box.position.y });
}

void InteractionButton::render(jo::RenderTarget& target) {
    target.draw(*sprite);
}

void InteractionButton::setTexture(const std::string& texturePath) {
    texture = &ResourceManager<jo::Texture>::getInstance()->get(texturePath);
    sprite->setTexture(*texture);
}

jo::Vector2f InteractionButton::getPosition() const {
    return sprite->getPosition();
}

void InteractionButton::setPosition(const jo::Vector2f& pos) {
    sprite->setPosition(pos);
}
