#include "joanna/systems/menu.h"

#include "joanna/core/savegamemanager.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>

Menu::Menu(WindowManager& windowManager, Controller& controller)
    : pos(0), windowManager(&windowManager), controller(&controller) {
    this->font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
    this->font.setSmooth(false);

    pressed = theselect = false;

    set_values();
}

void Menu::set_values() {
    pos_mouse = { 0, 0 };
    mouse_coord = { 0, 0 };

    options = { "Joanna's Farm", "Play", "Save", "Options", "About", "Quit" };
    sizes = { 12, 10, 10, 10, 10, 10 };

    texts.clear();       // just in case
    float spacing = 5.f; // vertical spacing between texts

    // Compute total height
    float totalHeight = -spacing; // remove extra after last
    for (std::size_t i = 0; i < options.size(); ++i) {
        totalHeight += static_cast<float>(sizes[i]) + spacing;
    }

    // Starting Y to vertically center
    float startY =
        windowManager->getMainView().getCenter().y - (totalHeight / 2.f);

    for (std::size_t i = 0; i < options.size(); ++i) {
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(sizes[i]);
        text.setLetterSpacing(2.f);
        text.setOutlineColor(sf::Color(51, 202, 127));

        // Horizontal centering
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition({ windowManager->getMainView().getCenter().x,
                           std::floor(startY) });
        Logger::info(
            "Menu option '{}' positioned at ({}, {})", options[i],
            text.getPosition().x, text.getPosition().y
        );

        startY += static_cast<float>(sizes[i]) + spacing;

        // Move into vector
        texts.push_back(std::move(text));
    }

    texts[0].setOutlineThickness(1);
    pos = 1;
    texts[pos].setOutlineThickness(1);

    pressed = theselect = false;
}

void Menu::loop_events() {
    auto& window = windowManager->getWindow();

    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        // Handle navigation keys (W/S)
        if (!pressed) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && pos < 5) {
                updateSelection(pos + 1);
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
                       pos > 1) {
                updateSelection(pos - 1);
            }
        }

        // Reset pressed flag if no navigation keys are pressed
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            pressed = false;
        }

        // Handle action keys (Space/Enter)
        if (!theselect) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                handleSelection();
                break; // exit the loop after a selection
            }
        }
    }
}

// Helper to update the selection visually
void Menu::updateSelection(const std::size_t newPos) {
    texts[pos].setOutlineThickness(0); // deselect previous
    pos = static_cast<int>(newPos);
    texts[pos].setOutlineThickness(1); // select new
    theselect = false;
    pressed = true;
}

// Helper to handle menu selection actions
void Menu::handleSelection() {
    theselect = true;

    switch (pos) {
        case 1:
            Logger::info("Play selected");
            break;
        case 2: {
            const SaveGameManager saveManager;
            auto playerPos = controller->getPlayer().getPosition();
            GameState state;
            state.player.x = playerPos.x;
            state.player.y = playerPos.y;
            saveManager.saveGame(state);
            Logger::info("Save selected");
            theselect = false;
            pressed = false;
            break;
        }
        case 3:
            Logger::info("Options selected");
            theselect = false;
            pressed = false;
            break;
        case 4:
            Logger::info("About selected");
            theselect = false;
            pressed = false;
            break;
        case 5:
            windowManager->getWindow().close();
            break;
        default:
            break;
    }
}

void Menu::draw_all() {
    windowManager->setView(windowManager->getMainView());
    windowManager->getWindow().clear(sf::Color::Black);

    for (const auto& t : texts) {
        windowManager->getWindow().draw(t);
    }
    windowManager->pollEvents();
    windowManager->getWindow().display();
}

void Menu::show() {
    auto tmp = windowManager->getWindow().getView();
    while (windowManager->getWindow().isOpen() && !theselect) {
        loop_events();
        draw_all();
    }
    windowManager->getWindow().setView(tmp);
}