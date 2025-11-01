#include "menu.h"
#include "logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

Menu::Menu(sf::RenderWindow& window) : window(window) {
    this->font.openFromFile("assets/font/Pixellari.ttf");

    pos = 0;
    pressed = theselect = false;

    set_values();
}

void Menu::set_values() {
    pos_mouse = { 0, 0 };
    mouse_coord = { 0, 0 };

    options = { "Joe's Farm", "Resume", "Save", "Options", "About", "Quit" };
    sizes = { 24, 20, 20, 20, 20, 20 };

    texts.clear();        // just in case
    float spacing = 10.f; // vertical spacing between texts

    // Compute total height
    float totalHeight = -spacing; // remove extra after last
    for (std::size_t i = 0; i < options.size(); ++i) {
        totalHeight += sizes[i] + spacing;
    }

    // Starting Y to vertically center
    float startY = window.getView().getCenter().y - totalHeight / 2.f;

    for (std::size_t i = 0; i < options.size(); ++i) {
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(sizes[i]);
        text.setOutlineColor(sf::Color::Black);

        // Horizontal centering
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition({ window.getView().getCenter().x, startY });
        Logger::info(
            "Menu option '{}' positioned at ({}, {})", options[i],
            text.getPosition().x, text.getPosition().y
        );

        startY += sizes[i] + spacing;

        // Move into vector
        texts.push_back(std::move(text));
    }

    // Highlight first selectable option (skip "War Game" title)
    pos = 1;
    texts[pos].setOutlineThickness(4);

    pressed = theselect = false;
}

void Menu::loop_events() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        pos_mouse = sf::Mouse::getPosition(window);
        mouse_coord = window.mapPixelToCoords(pos_mouse);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && !pressed) {
            if (pos < 5) {
                ++pos;
                texts[pos].setOutlineThickness(4);
                texts[pos - 1].setOutlineThickness(0);
                theselect = false;
                pressed = true;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && !pressed) {
            if (pos > 1) {
                --pos;
                texts[pos].setOutlineThickness(4);
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
                window.close();
            }
        }
    }
}
 
void Menu::draw_all() {
    window.clear(sf::Color(216,189,138));

    for (const auto& t : texts) {
        window.draw(t);
    }
    window.display();
}

void Menu::show() {
    while (window.isOpen() && !theselect) {
        loop_events();
        draw_all();
    }
}