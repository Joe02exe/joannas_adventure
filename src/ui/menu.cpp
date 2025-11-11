#include "menu.h"
#include "../core/logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

Menu::Menu(WindowManager& windowManager) : windowManager(windowManager) {
    this->font.openFromFile("assets/font/minecraft.ttf");
    this->font.setSmooth(false);
    pos = 0;
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
        totalHeight += sizes[i] + spacing;
    }

    // Starting Y to vertically center
    float startY =
        windowManager.getMainView().getCenter().y - totalHeight / 2.f;

    for (std::size_t i = 0; i < options.size(); ++i) {
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(sizes[i]);
        text.setLetterSpacing(2.f);
        text.setOutlineColor(sf::Color(51, 202, 127));

        // Horizontal centering
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition(
            { windowManager.getMainView().getCenter().x, std::floor(startY) }
        );
        Logger::info(
            "Menu option '{}' positioned at ({}, {})", options[i],
            text.getPosition().x, text.getPosition().y
        );

        startY += sizes[i] + spacing;

        // Move into vector
        texts.push_back(std::move(text));
    }

    texts[0].setOutlineThickness(1);
    pos = 1;
    texts[pos].setOutlineThickness(1);

    pressed = theselect = false;
}

void Menu::loop_events() {
    while (const std::optional<sf::Event> event =
               windowManager.getWindow().pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            windowManager.getWindow().close();
        }

        // windowManager.pollEvents();

        pos_mouse = sf::Mouse::getPosition(windowManager.getWindow());
        mouse_coord = windowManager.getWindow().mapPixelToCoords(pos_mouse);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && !pressed) {
            if (pos < 5) {
                ++pos;
                texts[pos].setOutlineThickness(1);
                texts[pos - 1].setOutlineThickness(0);
                theselect = false;
                pressed = true;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && !pressed) {
            if (pos > 1) {
                --pos;
                texts[pos].setOutlineThickness(1);
                texts[pos + 1].setOutlineThickness(0);
                theselect = false;
                pressed = true;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) &&
            !theselect) {
            theselect = true;
            Logger::info("Exit from menu");
            break;
        }

        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            pressed = false;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) &&
            !theselect) {
            theselect = true;
            if (pos == 1) {
                Logger::info("Play selected");
                break;
            }
            if (pos == 2) {
                Logger::info("Save selected");
                pressed = false;
                theselect = false;
                break;
            }
            if (pos == 3) {
                Logger::info("Options selected");
                pressed = false;
                theselect = false;
                break;
            }
            if (pos == 4) {
                Logger::info("About selected");
                pressed = false;
                theselect = false;
                break;
            }
            if (pos == 5) {
                windowManager.getWindow().close();
            }
        }
    }
}

void Menu::draw_all() {
    windowManager.setView(windowManager.getMainView());
    windowManager.getWindow().clear(sf::Color::Black);

    for (const auto& t : texts) {
        windowManager.getWindow().draw(t);
    }
    windowManager.pollEvents();
    windowManager.getWindow().display();
}

void Menu::show() {
    auto tmp = windowManager.getWindow().getView();
    while (windowManager.getWindow().isOpen() && !theselect) {
        loop_events();
        draw_all();
    }
    windowManager.getWindow().setView(tmp);
}