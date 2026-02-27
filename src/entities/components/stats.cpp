#include "joanna/entities/stats.h"
#include "joanna/core/graphics.h"
#include <string>

Stats::Stats() : attack(0), defense(0) {}

Stats::Stats(int attack, int defense) : attack(attack), defense(defense) {}

void Stats::draw(
    jo::RenderTarget& target, const jo::Font& font, int currentExp,
    int expToNextLevel
) const {
    // Match health bar alignment logic from view bounding
    const auto size = target.getView().getSize();
    const float startX =
        (-size.x / 2.f) + 40.f; // Base margin pad to align with health
    const float startY =
        (-size.y / 2.f) + 110.f; // Safely below doubled health height
    const float lineHeight = 40.f;
    const jo::Vector2f segmentSize(14.f, 28.f);
    const float gap = 4.f;

    auto drawRow = [&](int current, int max, jo::Color activeColor,
                       jo::Color emptyColor, float y) {
        float x = startX;
        int count = (max > 0) ? max : current;

        jo::RectangleShape rect(segmentSize);

        rect.setOutlineColor(jo::Color::Black);
        rect.setOutlineThickness(-1.f);

        for (int i = 0; i < count; ++i) {
            rect.setPosition({ x, y + 4.f });
            if (i < current) {
                rect.setFillColor(activeColor);
            } else {
                rect.setFillColor(emptyColor);
            }
            target.draw(rect);
            x += segmentSize.x + gap;
        }
    };

    drawRow(attack, 0, jo::Color(230, 50, 50), jo::Color::Transparent, startY);
    drawRow(
        defense, 0, jo::Color(50, 230, 50), jo::Color::Transparent,
        startY + lineHeight
    );
    drawRow(
        currentExp, expToNextLevel, jo::Color(50, 100, 255),
        jo::Color(60, 60, 60), startY + lineHeight * 2
    );
}