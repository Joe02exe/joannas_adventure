#include "joanna/systems/gameover.h"
#include "joanna/utils/resourcemanager.h"
#include <iostream>

GameOver::GameOver(WindowManager& windowManager)
    : windowManager(windowManager),
      font(&ResourceManager<jo::Font>::getInstance()->get(
          "assets/font/minecraft.ttf"
      )),
      gameText(*font), overText(*font) {

    gameText.setString("Game");
    gameText.setCharacterSize(180);
    gameText.setFillColor(jo::Color::Red);

    overText.setString("Over");
    overText.setCharacterSize(180);
    overText.setFillColor(jo::Color::Red);

    alignText();
}

void GameOver::update(float dt) {
    if (cooldown > 0.f) {
        cooldown -= dt;
    }
}

void GameOver::alignText() {
    jo::FloatRect gameBounds = gameText.getLocalBounds();
    jo::FloatRect overBounds = overText.getLocalBounds();

    gameText.setOrigin({ gameBounds.position.x + gameBounds.size.x / 2.0f,
                         gameBounds.position.y + gameBounds.size.y / 2.0f });
    overText.setOrigin({ overBounds.position.x + overBounds.size.x / 2.0f,
                         overBounds.position.y + overBounds.size.y / 2.0f });

    jo::Vector2f center = windowManager.getMainView().getCenter();
    gameText.setPosition({ center.x - 50.f, center.y - 50.f });
    overText.setPosition({ center.x - 50.f, center.y + 50.f });
}

void GameOver::render() {
    jo::RenderWindow& window = windowManager.getWindow();

    // Ensure view is set correctly for UI
    jo::View oldView = window.getView();
    jo::Vector2u size = window.getSize();
    jo::View view(jo::FloatRect(
        { 0.f, 0.f }, { static_cast<float>(size.x), static_cast<float>(size.y) }
    ));
    window.setView(view);

    jo::Vector2f center(size.x / 2.0f, size.y / 2.0f);

    gameText.setPosition({ center.x - 35.f, center.y - 50.f });
    overText.setPosition({ center.x - 35.f, center.y + 50.f });

    window.clear(jo::Color::Black);
    window.draw(gameText);
    window.draw(overText);

    window.setView(oldView);
}

void GameOver::handleInput(const jo::Event& event) {
    if (cooldown > 0.f) {
        return;
    }

    if (event.is<jo::Event::KeyPressed>() ||
        event.is<jo::Event::MouseButtonPressed>()) {
        if (onRestart) {
            Logger::info("GameOver input received, restarting...");
            onRestart();
        }
    }
}

void GameOver::setOnRestart(std::function<void()> callback) {
    onRestart = callback;
}
