#include "joanna/systems/font_renderer.h"

#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include "joanna/core/graphics.h"

FontRenderer::FontRenderer(const std::string& fontPath) : loaded(false) {
    font = &ResourceManager<jo::Font>::getInstance()->get(fontPath);
    loaded = true;
}

bool FontRenderer::isLoaded() const {
    return loaded;
}

void FontRenderer::drawText(
    jo::RenderTarget& target, const std::string& text,
    const jo::Vector2f& position, unsigned int size, const jo::Color& color,
    uint32_t options
) {
    drawTextImpl(target, text, position, size, color, options);
}

void FontRenderer::drawTextUI(
    jo::RenderTarget& target, const std::string& text,
    const jo::Vector2f& position, unsigned int size, const jo::Color& color,
    uint32_t options
) {
    // Save current view
    jo::View oldView = target.getView();

    // Set to default view (screen space)
    target.setView(target.getDefaultView());

    // Draw text
    drawTextImpl(target, text, position, size, color, options);

    // Restore original view
    target.setView(oldView);
}

void FontRenderer::drawTextImpl(
    jo::RenderTarget& target, const std::string& text,
    const jo::Vector2f& position, unsigned int size, const jo::Color& color,
    uint32_t options
) {
    if (!loaded) {
        Logger::error("Attempting to draw text with unloaded font");
        return;
    }

    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos) {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    lines.push_back(text.substr(start));

    float currentY = position.y;

    for (const auto& line : lines) {
        if (line.empty()) {
            currentY += size + 5.0f;
            continue;
        }

        jo::Text textObj(*font, line);
        textObj.setCharacterSize(size);
        textObj.setFillColor(color);
        textObj.setLetterSpacing(letterSpacing);

        // Apply outline if requested
        if (options & OUTLINE) {
            textObj.setOutlineColor(jo::Color::Black);
            textObj.setOutlineThickness(2.0f);
        }

        jo::Vector2f linePos = { position.x, currentY };
        // Apply alignment before drawing shadow (so shadow is properly
        // positioned)
        applyAlignment(textObj, linePos, options);

        // Draw shadow first (behind main text)
        if (options & SHADOW) {
            jo::Text shadowText(*font, line);
            shadowText.setCharacterSize(size);
            shadowText.setLetterSpacing(letterSpacing);
            shadowText.setFillColor(jo::Color(0, 0, 0, 128)
            ); // Semi-transparent black
            shadowText.setPosition(
                textObj.getPosition() + jo::Vector2f(2.f, 2.f)
            );
            applyAlignment(shadowText, shadowText.getPosition(), options);
            target.draw(shadowText);
        }

        // Draw main text
        target.draw(textObj);

        currentY += size + 5.0f; // line spacing margin
    }
}

const jo::Font& FontRenderer::getFont() const {
    return *font;
}

void FontRenderer::applyAlignment(
    jo::Text& text, const jo::Vector2f& position, uint32_t options
) {
    jo::FloatRect bounds = text.getLocalBounds();
    jo::Vector2f origin(0.f, 0.f);

    if (options & CENTERED_X) {
        origin.x = bounds.size.x / 2.f;
    }

    if (options & CENTERED_Y) {
        origin.y = bounds.size.y / 2.f;
    }

    text.setOrigin(origin);
    text.setPosition(position);
}