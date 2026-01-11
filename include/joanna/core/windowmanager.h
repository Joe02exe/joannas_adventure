#pragma once

#include "joanna/utils/debug.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

class WindowManager {
  public:
    explicit WindowManager(
        unsigned width = 900, unsigned height = 900,
        const std::string& title = "Game",
        sf::Vector2f initialPos = { 150.f, 165.f }
    );
    void setView(const sf::View& v);
    void clear();
    void display();
    void setCenter(const sf::Vector2f& center);

    void handleResizeEvent(sf::Vector2u newSize);

    sf::RenderWindow& getWindow() {
        return window;
    }

    sf::View& getMainView() {
        return mainView;
    }

    sf::View& getMiniMapView() {
        return miniMapView;
    }

    sf::View& getUiView() {
        return uiView;
    }

    DebugUI getDebugUI() {
        return debug_ui;
    }

    const sf::View& getDefaultView() {
        return window.getDefaultView();
    }

    void pollEvents();
    void render();

  private:
    sf::RenderWindow window;
    sf::View mainView;
    sf::View miniMapView;
    sf::View uiView;
    float targetAspectRatio;
    static constexpr float MINI_MAP_SIZE = 0.25f;
    DebugUI debug_ui;

    sf::FloatRect computeMainViewPort(sf::Vector2u newSize) const;
};