#pragma once

#include "../player/player.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class Controller {
  public:
    Controller(Player& player, sf::View& camera, sf::View& miniMapView);
    void getInput(float dt, sf::RenderWindow& window);
    sf::View& getCamera() { return *camera; }
    sf::View& getMiniMapView() { return *miniMapView; }
    Player& getPlayer() { return *player; }
  private:
    Player* player;
    sf::View* camera;
    sf::View* miniMapView;
    bool facingLeft = false;
};