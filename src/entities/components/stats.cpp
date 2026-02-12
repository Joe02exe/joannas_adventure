#include "joanna/entities/stats.h"
#include <string>
#include <SFML/Graphics.hpp>

Stats::Stats() : attack(0), defense(0) {}

Stats::Stats(int attack, int defense) : attack(attack), defense(defense) {}

void Stats::draw(sf::RenderTarget& target, const sf::Font& font, int currentExp, int expToNextLevel) const {
    const float startX = -440.f;
    const float startY = -415.f;
    const float lineHeight = 18.f;
    const float labelWidth = 40.f;
    const sf::Vector2f segmentSize(8.f, 12.f);
    const float gap = 0.f;
    auto drawRow = [&](const std::string& label, int current, int max, sf::Color activeColor, sf::Color emptyColor, float y) {
        sf::Text text(font);
        text.setString(label);
        text.setCharacterSize(15);
        text.setPosition({startX, y});
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1.f);
        target.draw(text);

        float x = startX + labelWidth;
        int count = (max > 0) ? max : current;

        sf::RectangleShape rect(segmentSize);

        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(-1.f);

        for (int i = 0; i < count; ++i) {
            rect.setPosition({x, y + 4.f});
            if (i < current) {
                rect.setFillColor(activeColor);
            } else {
                rect.setFillColor(emptyColor);
            }
            target.draw(rect);
            x += segmentSize.x + gap;
        }
    };

    drawRow("ATK", attack, 0, sf::Color(230, 50, 50), sf::Color::Transparent, startY);
    drawRow("DEF", defense, 0, sf::Color(50, 230, 50), sf::Color::Transparent, startY + lineHeight);
    drawRow("EXP", currentExp, expToNextLevel, sf::Color(50, 100, 255), sf::Color(60, 60, 60), startY + lineHeight * 2);
}