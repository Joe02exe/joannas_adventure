#include "joanna/entities/interactionbutton.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Interactable {

    public:
        Interactable(const sf::FloatRect& box, const std::optional<std::string>& name,
          const std::string& buttonTexturePath, const std::string& spriteTexturePath
      )
          : id(NEXT_ID++), boundingBox(box), name(name),
            button(boundingBox, buttonTexturePath) {
                sf::Texture texture(spriteTexturePath);
                sprite->setTexture(texture);
                sprite->setPosition({ box.position.x, box.position.y });
            }

        virtual ~Interactable() = default;

        virtual void onInteract() = 0;
        void render(sf::RenderTarget& target, bool renderWithInteraction) {
            target.draw(*sprite);
            if (renderWithInteraction) {
                button.render(target);
            }
        }

        bool canPlayerInteract(const sf::Vector2f& playerPos) const {
            // assuming that playerpos is the center of the player
            sf::Vector2f center(
                boundingBox.position.x + boundingBox.size.x * 0.5f,
                boundingBox.position.y + boundingBox.size.y * 0.5f
            );

            float dx = playerPos.x - center.x;
            float dy = playerPos.y - center.y;
            return dx * dx + dy * dy <= interactionDistance * interactionDistance;
        }

        uint32_t getId() const {
            return id;
        }

        std::optional<std::string> getName() const {
            return name;
        }

        sf::FloatRect getBoundingBox() const {
            return boundingBox;
        }

        void setName(const std::optional<std::string>& newName) {
            name = newName;
        }

        void setTexture(const sf::Texture& texture) {
            sprite->setTexture(texture);
        }

    private:
        static inline uint32_t NEXT_ID = 1; // uuid --> uuids would require extra dependencies
        const uint32_t id;
        const sf::FloatRect boundingBox;
        std::optional<std::string> name;
        std::unique_ptr<sf::Sprite> sprite; // use pointer to avoid initialization
        InteractionButton button;
        const float interactionDistance = 32.f;
};
