#include "font_renderer.h"
#include "../core/logger.h"
#include <SFML/Graphics/RenderTarget.hpp>

FontRenderer::FontRenderer(const std::string& fontPath) : loaded(false) {
    if (font.openFromFile(fontPath)) {
        loaded = true;
        Logger::info("Font loaded successfully from: {}", fontPath);
    } else {
        Logger::error("Failed to load font from: {}", fontPath);
    }
}

bool FontRenderer::isLoaded() const {
    return loaded;
}

void FontRenderer::drawText(
    sf::RenderTarget& target,
    const std::string& text,
    const sf::Vector2f& position,
    unsigned int size,
    const sf::Color& color,
    uint32_t options
) {
    drawTextImpl(target, text, position, size, color, options);
}

void FontRenderer::drawTextUI(
    sf::RenderTarget& target,
    const std::string& text,
    const sf::Vector2f& position,
    unsigned int size,
    const sf::Color& color,
    uint32_t options
) {
    // Save current view
    sf::View oldView = target.getView();

    // Set to default view (screen space)
    target.setView(target.getDefaultView());

    // Draw text
    drawTextImpl(target, text, position, size, color, options);

    // Restore original view
    target.setView(oldView);
}

void FontRenderer::drawTextImpl(
    sf::RenderTarget& target,
    const std::string& text,
    const sf::Vector2f& position,
    unsigned int size,
    const sf::Color& color,
    uint32_t options
) {
    if (!loaded) {
        Logger::error("Attempting to draw text with unloaded font");
        return;
    }

    sf::Text textObj(font, text);
    textObj.setCharacterSize(size);
    textObj.setFillColor(color);
    textObj.setLetterSpacing(letterSpacing);

    // Apply outline if requested
    if (options & OUTLINE) {
        textObj.setOutlineColor(sf::Color::Black);
        textObj.setOutlineThickness(2.0f);
    }

    // Apply alignment before drawing shadow (so shadow is properly positioned)
    applyAlignment(textObj, position, options);

    // Draw shadow first (behind main text)
    if (options & SHADOW) {
        sf::Text shadowText(font, text);
        shadowText.setCharacterSize(size);
        shadowText.setLetterSpacing(letterSpacing);
        shadowText.setFillColor(sf::Color(0, 0, 0, 128)); // Semi-transparent black
        shadowText.setPosition(textObj.getPosition() + sf::Vector2f(2.f, 2.f));
        applyAlignment(shadowText, shadowText.getPosition(), options);
        target.draw(shadowText);
    }

    // Draw main text
    target.draw(textObj);
}

const sf::Font& FontRenderer::getFont() const {
    return font;
}

void FontRenderer::applyAlignment(sf::Text& text, const sf::Vector2f& position, uint32_t options) {
    sf::FloatRect bounds = text.getLocalBounds();
    sf::Vector2f origin(0.f, 0.f);

    if (options & CENTERED_X) {
        origin.x = bounds.size.x / 2.f;
    }

    if (options & CENTERED_Y) {
        origin.y = bounds.size.y / 2.f;
    }

    text.setOrigin(origin);
    text.setPosition(position);
}