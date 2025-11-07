#include "windowmanager.h"

#include "logger.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

WindowManager::WindowManager(
    unsigned width, unsigned height, const std::string& title
)
    : window(sf::VideoMode({ width, height }), title),
      targetAspectRatio(
          static_cast<float>(height) / static_cast<float>(width)
      ) {

    mainView.setSize({ static_cast<float>(width), static_cast<float>(height) });
    mainView.zoom(0.15f);
    mainView.setCenter({ 0.f, 0.f });

    miniMapView.setSize({ 250.f, 250.f });
    miniMapView.setCenter({ 0.f, 0.f });
    miniMapView.setViewport(
        sf::FloatRect({ 0.75f, 0.f }, { MINI_MAP_SIZE, MINI_MAP_SIZE })
    );
}

void WindowManager::setCenter(const sf::Vector2f& center) {
    mainView.setCenter(center);
    miniMapView.setCenter(center);
}

void WindowManager::pollEvents() {

    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::Vector2u newSize(resized->size.x, resized->size.y);

            sf::FloatRect mainViewport = computeMainViewPort(newSize);

            sf::FloatRect miniViewport;
            miniViewport.position = { mainViewport.position.x +
                                          mainViewport.size.x -
                                          MINI_MAP_SIZE * mainViewport.size.x,
                                      mainViewport.position.y };
            miniViewport.size = { MINI_MAP_SIZE * mainViewport.size.x,
                                  MINI_MAP_SIZE * mainViewport.size.y };

            mainView.setViewport(mainViewport);
            miniMapView.setViewport(miniViewport);

        }
    }
}

sf::FloatRect WindowManager::computeMainViewPort(sf::Vector2u newSize) {
    float newAspectRatio =
        static_cast<float>(newSize.x) / static_cast<float>(newSize.y);

    sf::FloatRect mainViewport;
    if (newAspectRatio > targetAspectRatio) {
        float width = targetAspectRatio / newAspectRatio;
        mainViewport = { { (1.f - width) / 2.f, 0.f }, { width, 1.f } };
    } else {
        float height = newAspectRatio / targetAspectRatio;
        mainViewport = { { 0.f, (1.f - height) / 2.f }, { 1.f, height } };
    }
    return mainViewport;
}

void WindowManager::render() {}