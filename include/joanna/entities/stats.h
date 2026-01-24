#pragma once
#include <SFML/Graphics.hpp>

class Stats {
    public:
        Stats();

        Stats(int attack, int defense);
        int attack;
        int defense;

        void draw(sf::RenderTarget& target, const sf::Font& font) const;
};