#pragma once

#include "controller.h"
#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"

#include <SFML/Graphics.hpp>

class Menu {
    int pos;
    bool pressed, isItemClicked;

    WindowManager* windowManager;
    Controller* controller;
    sf::Font font;
    sf::Texture image;

    sf::Sprite mouseSprite;

    std::vector<std::string> options;
    std::vector<sf::Vector2f> coords;
    std::vector<sf::Text> texts;
    std::vector<sf::RectangleShape> backgrounds;
    std::vector<std::size_t> sizes;

    bool showAbout = false;
    std::string aboutText;

  protected:
    void set_values();

    sf::Vector2f getMouseCoordinatesFromWindow(const sf::RenderWindow& window
    ) const;

    void highlightHoveredOption(sf::Vector2f mouse_pos);

    bool handleAboutEvents(const std::optional<sf::Event>& event);

    void mouseButtonClicked(
        sf::Vector2f mouse_pos, const std::optional<sf::Event>& event
    );

    void handleMenuEvents(AudioManager& audioManager);
    void updateSelection(std::size_t newPos);
    void handleSelection();
    void renderMouseCursor(sf::RenderTarget& window) const;
    void render(
        RenderEngine& render_engine, TileManager& tileManager,
        std::list<std::unique_ptr<Interactable>>& interactable,
        std::shared_ptr<DialogueBox> dialogueBox
    ) const;

  public:
    Menu(WindowManager& windowManager, Controller& controller);
    void show(
        RenderEngine& render_engine, TileManager& tileManager,
        std::list<std::unique_ptr<Interactable>>& interactable,
        const std::shared_ptr<DialogueBox>& dialogueBox,
        AudioManager& audioManager
    );
};