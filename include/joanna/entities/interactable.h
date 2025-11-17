#pragma once

#include "joanna/entities/interactionbutton.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Interactable {

  public:
    Interactable(
        const sf::FloatRect& box, const std::optional<std::string>& name,
        const std::string& buttonTexturePath,
        const std::string& spriteTexturePath
    );

    virtual ~Interactable() = default;

    virtual void onInteract() = 0;

    void render(sf::RenderTarget& target, bool renderWithInteraction);

    bool canPlayerInteract(const sf::Vector2f& playerPos) const;

    uint32_t getId() const;

    std::optional<std::string> getName() const;

    sf::FloatRect getBoundingBox() const;

    void setName(const std::optional<std::string>& newName);

    void setFrame(const sf::IntRect& textureRect);

  private:
    static inline uint32_t NEXT_ID = 1;
    const uint32_t id;
    const sf::FloatRect boundingBox;
    std::optional<std::string> name;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Texture texture;
    InteractionButton button;
    const float interactionDistance = 32.f;
};
