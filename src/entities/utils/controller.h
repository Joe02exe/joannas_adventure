#pragma once

#include "../../core/windowmanager.h"
#include "../player/player.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class Controller {
  public:
    Controller(WindowManager& windowManager);

    void getInput(
        float dt, sf::RenderWindow& window,
        const std::vector<sf::FloatRect>& collisions
    );

    sf::View& getPlayerView() {
        return playerView;
    }

    sf::View& getMiniMapView() {
        return miniMapView;
    }

    Player& getPlayer() {
        return player;
    }

  private:
    WindowManager* windowManager;
    Player player;
    sf::View playerView;
    sf::View miniMapView;
    bool facingLeft = false;
};