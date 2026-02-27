#pragma once

#include "joanna/core/graphics.h"
#include "joanna/core/windowmanager.h"
#include "joanna/utils/logger.h"
#include <functional>

class GameOver {
  public:
    GameOver(WindowManager& windowManager);

    void update(float dt);
    void render();
    void handleInput(const jo::Event& event);
    void setOnRestart(std::function<void()> callback);

  private:
    WindowManager& windowManager;
    const jo::Font* font;
    jo::Text gameText;
    jo::Text overText;
    std::function<void()> onRestart;
    float cooldown = 2.0f; // 2 seconds delay before input is accepted

    void alignText();
};
