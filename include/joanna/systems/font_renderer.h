#pragma once

#include "joanna/core/graphics.h"
#include <cstdint>
#include <string>

class FontRenderer {
  public:
    // Text rendering options (bitfield)
    enum Options : uint32_t {
        NONE = 0,
        SHADOW = 1 << 0,     // Add drop shadow
        OUTLINE = 1 << 1,    // Add outline
        CENTERED_X = 1 << 2, // Center horizontally at position
        CENTERED_Y = 1 << 3  // Center vertically at position
    };

    // Constructor: loads font from file
    explicit FontRenderer(const std::string& fontPath);

    // Check if font loaded successfully
    bool isLoaded() const;

    // Draw text to any render target (window, texture, etc.)
    void drawText(
        jo::RenderTarget& target, const std::string& text,
        const jo::Vector2f& position, unsigned int size,
        const jo::Color& color = jo::Color::White, uint32_t options = NONE
    );

    // Draw text in screen space (UI) - temporarily sets default view
    void drawTextUI(
        jo::RenderTarget& target, const std::string& text,
        const jo::Vector2f& position, unsigned int size,
        const jo::Color& color = jo::Color::White, uint32_t options = NONE
    );

    // Get the font for direct access if needed
    const jo::Font& getFont() const;

  private:
    const jo::Font* font;
    bool loaded;
    static constexpr float letterSpacing = 1.5f;

    // Helper to apply centering
    void applyAlignment(
        jo::Text& text, const jo::Vector2f& position, uint32_t options
    );

    // Internal draw implementation
    void drawTextImpl(
        jo::RenderTarget& target, const std::string& text,
        const jo::Vector2f& position, unsigned int size, const jo::Color& color,
        uint32_t options
    );
};

// Bitwise operators for Options enum
inline FontRenderer::Options
operator|(FontRenderer::Options a, FontRenderer::Options b) {
    return static_cast<FontRenderer::Options>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline uint32_t operator|(uint32_t a, FontRenderer::Options b) {
    return a | static_cast<uint32_t>(b);
}

inline uint32_t operator|(FontRenderer::Options a, uint32_t b) {
    return static_cast<uint32_t>(a) | b;
}