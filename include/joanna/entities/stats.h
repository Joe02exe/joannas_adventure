#pragma once
#include "joanna/core/graphics.h"

class Stats {
    public:
        Stats();

        Stats(int attack, int defense);
        int attack;
        int defense;

        void draw(jo::RenderTarget& target, const jo::Font& font, int currentExp, int expToNextLevel) const;
};