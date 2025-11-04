#pragma once

#include "../player/player.h"
#include <SFML/Graphics/RenderWindow.hpp>

class Controller {
  public:
    Controller(Player& player, sf::View& camera);
    void getInput(float dt, sf::RenderWindow& window);
    sf::View& getCamera() { return *camera; }
    Player& getPlayer() { return *player; }
  private:
    Player* player;
    sf::View* camera;
    bool facingLeft = false;
};