#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

class WindowManager {
public:
    WindowManager(unsigned width = 900, unsigned height = 900, const std::string& title = "Game", sf::Vector2f initialPos = { 150.f, 165.f});
    void handleResize(unsigned newWidth, unsigned newHeight);
    void setView(const sf::View& v);
    void clear();
    void display();
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

    sf::FloatRect computeMainViewPort(sf::Vector2u newSize) const;
};