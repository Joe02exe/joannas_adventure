#pragma once

#include "joanna/core/postprocessing.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

class Game {
  public:
    Game();
    static void run();

  private:
    static void resize(
        const sf::Vector2u size, float targetAspectRatio, sf::View& camera,
        sf::RenderWindow& window, PostProcessing& postProc
    );
};