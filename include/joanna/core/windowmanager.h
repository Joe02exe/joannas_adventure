#pragma once

#include "joanna/utils/debug.h"

#include "joanna/core/graphics.h"

class WindowManager {
  public:
    explicit WindowManager(
        unsigned width = 900, unsigned height = 900,
        const std::string& title = "Game",
        jo::Vector2f initialPos = { 150.f, 165.f }
    );
    void setView(const jo::View& v);
    void clear();
    void display();
    void setCenter(const jo::Vector2f& center);

    void handleResizeEvent(jo::Vector2u newSize);

    jo::RenderWindow& getWindow() {
        return window;
    }

    jo::View& getMainView() {
        return mainView;
    }

    jo::View& getMiniMapView() {
        return miniMapView;
    }

    jo::View& getUiView() {
        return uiView;
    }

    jo::View& getMapOverviewView() {
        return mapOverviewView;
    }

    DebugUI getDebugUI() {
        return debug_ui;
    }

    const jo::View& getDefaultView() {
        return window.getDefaultView();
    }

    void pollEvents();
    void render();

  private:
    jo::RenderWindow window;
    jo::View mainView;
    jo::View miniMapView;
    jo::View mapOverviewView;
    jo::View uiView;
    float targetAspectRatio;
    static constexpr float MINI_MAP_SIZE = 0.25f;
    DebugUI debug_ui;

    jo::FloatRect computeMainViewPort(jo::Vector2u newSize) const;
};