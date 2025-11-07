#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class WindowManager {
public:
    WindowManager(unsigned width = 900, unsigned height = 900, const std::string& title = "Game");
    void handleResize(unsigned newWidth, unsigned newHeight);
    void setView(const sf::View& v);
    void setCenter(const sf::Vector2f& center);
    sf::RenderWindow& getWindow() { return window; }
    sf::View& getMainView() { return mainView; }
    sf::View& getMiniMapView() { return miniMapView; }
    const sf::View& getDefaultView() { return window.getDefaultView(); }
    void pollEvents(); 
    void render();
private:
    sf::RenderWindow window;
    sf::View mainView;
    sf::View miniMapView;
    float targetAspectRatio;
    const float MINI_MAP_SIZE = 0.25f;

    sf::FloatRect computeMainViewPort(sf::Vector2u newSize);
};