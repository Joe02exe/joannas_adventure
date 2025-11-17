#pragma once

#include "joanna/core/windowmanager.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <list>

class Controller {
  public:
    Controller(WindowManager& windowManager);

    bool getInput(
        float dt, sf::RenderWindow& window,
        const std::vector<sf::FloatRect>& collisions,
        std::list<std::unique_ptr<Interactable>>& interactables

    );

    bool updateStep(
        float dt, sf::RenderWindow& window,
        std::vector<sf::FloatRect>& collisions,
        std::list<std::unique_ptr<Interactable>>& interactables

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