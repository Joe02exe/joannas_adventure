#include "joanna/systems/gameover.h"
#include "joanna/utils/resourcemanager.h"
#include <iostream>

GameOver::GameOver(WindowManager& windowManager)
    : windowManager(windowManager),
      font(ResourceManager<sf::Font>::getInstance()->get(
          "assets/font/minecraft.ttf"
      )),
      gameText(font), overText(font) {

    gameText.setString("Game");
    gameText.setCharacterSize(120);
    gameText.setFillColor(sf::Color::Red);

    overText.setString("Over");
    overText.setCharacterSize(120);
    overText.setFillColor(sf::Color::Red);

    alignText();
}

void GameOver::update(float dt) {
    if (cooldown > 0.f) {
        cooldown -= dt;
    }
}

void GameOver::alignText() {
    sf::FloatRect gameBounds = gameText.getLocalBounds();
    sf::FloatRect overBounds = overText.getLocalBounds();

    gameText.setOrigin({ gameBounds.position.x + gameBounds.size.x / 2.0f,
                         gameBounds.position.y + gameBounds.size.y / 2.0f });
    overText.setOrigin({ overBounds.position.x + overBounds.size.x / 2.0f,
                         overBounds.position.y + overBounds.size.y / 2.0f });

    sf::Vector2f center = windowManager.getMainView().getCenter();
    gameText.setPosition({ center.x, center.y - 50.f });
    overText.setPosition({ center.x, center.y + 50.f });
}

void GameOver::render() {
    sf::RenderWindow& window = windowManager.getWindow();

    // Ensure view is set correctly for UI
    sf::View oldView = window.getView();
    sf::Vector2u size = window.getSize();
    sf::View view(sf::FloatRect(
        { 0.f, 0.f }, { static_cast<float>(size.x), static_cast<float>(size.y) }
    ));
    window.setView(view);

    sf::Vector2f center(size.x / 2.0f, size.y / 2.0f);

    gameText.setPosition({ center.x, center.y - 50.f });
    overText.setPosition({ center.x, center.y + 50.f });

    window.clear(sf::Color::Black);
    window.draw(gameText);
    window.draw(overText);

    window.setView(oldView);
}

void GameOver::handleInput(const sf::Event& event) {
    if (cooldown > 0.f) {
        return;
    }

    if (event.is<sf::Event::KeyPressed>() ||
        event.is<sf::Event::MouseButtonPressed>()) {
        if (onRestart) {
            Logger::info("GameOver input received, restarting...");
            onRestart();
        }
    }
}

void GameOver::setOnRestart(std::function<void()> callback) {
    onRestart = callback;
}
