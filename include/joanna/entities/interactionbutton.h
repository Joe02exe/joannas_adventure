#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>

class InteractionButton {
    public:
        InteractionButton(const sf::FloatRect& box, const std::string& texturePath)
            : box(box), texture(texturePath), sprite(texture) {
            sprite.setPosition({ box.position.x,
                                box.position.y - box.size.y * 0.5f });
        }

        void render(sf::RenderTarget& target) {
            target.draw(sprite);
        }

        void setTexture(const std::string& texturePath) {
            if (!texture.loadFromFile(texturePath)) {
                Logger::error(
                    "Failed to load interaction button texture from {}", texturePath
                );
            }
            sprite.setTexture(texture);
        }

        sf::Vector2f getPosition() const {
            return sprite.getPosition();
        }

    private:
        const sf::FloatRect box;
        sf::Sprite sprite;
        sf::Texture texture; // keep it alive
};
