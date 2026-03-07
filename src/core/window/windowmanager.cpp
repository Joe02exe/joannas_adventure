#include "joanna/core/windowmanager.h"

#include "joanna/utils/debug.h"

#include "joanna/core/graphics.h"
#if IMGUI_ENABLED
#include <imgui-SFML.h>
#endif

WindowManager::WindowManager(
    unsigned width, unsigned height, const std::string& title,
    jo::Vector2f initialPos
)
    : window(jo::VideoMode({ width, height }), title),
      targetAspectRatio(
          static_cast<float>(width) / static_cast<float>(height)
      ) {

    mainView.setSize({ static_cast<float>(width), static_cast<float>(height) });
    mainView.zoom(1.f / 6.f); // 6x scale for perfectly crisp pixel art
    mainView.setCenter({ 0.f, 0.f });

    miniMapView.setSize({ 250.f, 250.f });
    miniMapView.setCenter({ 0.f, 0.f });
    miniMapView.setViewport(
        jo::FloatRect({ 0.75f, 0.f }, { MINI_MAP_SIZE, MINI_MAP_SIZE })
    );

    uiView.setCenter({ 0, 0 });
    uiView.setSize({ 900.f, 900.f });

    mapOverviewView.setSize({ static_cast<float>(width),
                              static_cast<float>(height) });
    mapOverviewView.zoom(.25f);
    mapOverviewView.setCenter({ 0.f, 0.f });

    window.setMouseCursorVisible(false);

    // ImGUI
    if constexpr (IMGUI_ENABLED) {
        DebugUI::init(window);
    }
    window.setFramerateLimit(60);
}

void WindowManager::setCenter(const jo::Vector2f& center) {
    mainView.setCenter(center);
    miniMapView.setCenter(center);
}

void WindowManager::handleResizeEvent(jo::Vector2u newSize) {
    jo::FloatRect mainViewport = computeMainViewPort(newSize);

    jo::FloatRect miniViewport;
    miniViewport.position = { mainViewport.position.x + mainViewport.size.x -
                                  (MINI_MAP_SIZE * mainViewport.size.x),
                              mainViewport.position.y };
    miniViewport.size = { MINI_MAP_SIZE * mainViewport.size.x,
                          MINI_MAP_SIZE * mainViewport.size.y };

    mainView.setViewport(mainViewport);
    miniMapView.setViewport(miniViewport);
    uiView.setViewport(mainViewport);
    mapOverviewView.setViewport(mainViewport);
}

void WindowManager::pollEvents() {

    while (const std::optional<jo::Event> event = window.pollEvent()) {
        if constexpr (IMGUI_ENABLED) {
            debug_ui.processEvent(window, *event);
        }
        if (event->is<jo::Event::Closed>()) {
            window.close();
        }
        if (const auto* resized = event->getIf<jo::Event::Resized>()) {
            jo::Vector2u newSize(resized->size.x, resized->size.y);

            handleResizeEvent(newSize);
        }
    }
}

jo::FloatRect WindowManager::computeMainViewPort(jo::Vector2u newSize) const {
    float newAspectRatio =
        static_cast<float>(newSize.x) / static_cast<float>(newSize.y);

    jo::FloatRect mainViewport;
    if (newAspectRatio > targetAspectRatio) {
        float width = targetAspectRatio / newAspectRatio;
        mainViewport = { { (1.f - width) / 2.f, 0.f }, { width, 1.f } };
    } else {
        float height = newAspectRatio / targetAspectRatio;
        mainViewport = { { 0.f, (1.f - height) / 2.f }, { 1.f, height } };
    }
    return mainViewport;
}

void WindowManager::setView(const jo::View& view) {
    window.setView(view);
}

void WindowManager::clear() {
    window.clear(jo::Color::Black);
}

void WindowManager::display() {
    window.display();
}

void WindowManager::render() {
    debug_ui.render(window);
}
