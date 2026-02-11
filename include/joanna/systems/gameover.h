#pragma once

#include "joanna/core/windowmanager.h"
#include "joanna/utils/logger.h"
#include <SFML/Graphics.hpp>
#include <functional>

class GameOver {
  public:
    GameOver(WindowManager& windowManager);

    void update(float dt);
    void render();
    void handleInput(const sf::Event& event);
    void setOnRestart(std::function<void()> callback);

  private:
    WindowManager& windowManager;
    sf::Font font;
    sf::Text gameText;
    sf::Text overText;
    std::function<void()> onRestart;
    float cooldown = 2.0f; // 2 seconds delay before input is accepted

    void alignText();
};
