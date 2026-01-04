#pragma once

#include "controller.h"
#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"

#include <SFML/Graphics.hpp>

class Menu {
    WindowManager* windowManager;
    Controller* controller;
    TileManager* tileManager;

    sf::Font font;
    sf::Sprite mouseSprite;

    std::vector<std::string> options;
    std::vector<sf::Text> menuTexts;
    std::vector<sf::RectangleShape> menuBackgrounds;
    GameState stateToSave;

    int selectedIndex = 1; // Start at 1 to skip title
    bool isMenuOpen = true;
    bool loadingInteraction = true;

    // About Screen State
    bool showAbout = false;
    std::string aboutTextContent;

    void handleInput(sf::Window& window);
    void updateSelection(int direction); // -1 for up, 1 for down
    void executeSelection();
    void render(
        RenderEngine& render_engine, TileManager& tileManager,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& dialogueBox
    );
    void renderMenuOptions(sf::RenderTarget& target);
    void renderAboutOverlay(sf::RenderTarget& target);
    void rebuildUI();
    sf::Vector2f getMouseWorldPos() const;
    void handleHover(const sf::Vector2f& mousePos);
    void resetToDefaultMenu();

  public:
    Menu(
        WindowManager& windowManager, Controller& controller,
        TileManager& tileManager
    );
    void show(
        RenderEngine& render_engine, TileManager& tileManager,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& dialogueBox,
        AudioManager& audioManager
    );
    void setOptions(const std::vector<std::string>& newOptions);
};