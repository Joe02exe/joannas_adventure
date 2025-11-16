#pragma once

#include "joanna/entities/player.h"

#include <SFML/Graphics/RenderWindow.hpp>

class DebugUI {
public:
    static void init(sf::RenderWindow& window);
    void processEvent(const sf::Window& window, const sf::Event& event) const;
    void update(float dt, sf::RenderWindow& window, Player& player) const;
    void render(sf::RenderWindow& window) const;
    static void shutdown();

    void toggle() { enabled = !enabled; }

private:
    bool enabled = true;
};
