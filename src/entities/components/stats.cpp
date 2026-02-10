#include "joanna/entities/stats.h"
#include <string>
#include <SFML/Graphics.hpp>

Stats::Stats() : attack(0), defense(0) {}

Stats::Stats(int attack, int defense) : attack(attack), defense(defense) {}

void Stats::draw(sf::RenderTarget& target, const sf::Font& font, int currentExp, int expToNextLevel) const {
    sf::Text text(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    
    std::string info = "Atk: " + std::to_string(attack) + " | Def: " + std::to_string(defense) +
                       " | EXP: " + std::to_string(currentExp) + " / " + std::to_string(expToNextLevel);
    text.setString(info);

    text.setPosition(sf::Vector2f(-445.f, -420.f));

    target.draw(text);
}