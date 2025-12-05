#include "joanna/systems/menu.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/savegamemanager.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include <utility>

Menu::Menu(WindowManager& windowManager, Controller& controller)
    : pos(0), windowManager(&windowManager), controller(&controller),
      mouseSprite(sf::Sprite(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/cursor.png"
      ))) {
    this->font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
    this->font.setSmooth(false);

    pressed = isItemClicked = false;
    mouseSprite.setOrigin({ 0.f, 0.f });

    set_values();
}

void Menu::set_values() {

    // TODO read from config file
    options = { "Joanna's Farm", "Play", "Save", "Options", "About", "Quit" };
    sizes = { 12, 10, 10, 10, 10, 10 };

    texts.clear();
    backgrounds.clear();
    float spacing = 7.5f; // vertical spacing between texts

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

        const sf::FloatRect bounds = text.getLocalBounds();

        // Horizontal centering
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition({ windowManager->getMainView().getCenter().x,
                           std::floor(startY) });

        sf::RectangleShape background;
        background.setSize({ bounds.size.x + 10.f, bounds.size.y + 6.f }
        ); // padding
        background.setFillColor(sf::Color(50, 50, 50, 200));

        // Position the rectangle so it matches the text origin
        background.setOrigin({ bounds.size.x / 2.f, 0.f }); // match text origin
        background.setPosition({ windowManager->getMainView().getCenter().x -
                                     5.f,
                                 std::floor(startY) }); // adjust for padding

        startY += static_cast<float>(sizes[i]) + spacing;

        // Move into vector
        texts.push_back(std::move(text));
        backgrounds.push_back(std::move(background));
    }

    texts[0].setOutlineThickness(1);
    pos = 1;
    texts[pos].setOutlineThickness(1);

    pressed = isItemClicked = false;
}

/** Gets the mouse coordinates in world space from the given render window.
 *
 * @param window Render window to get mouse coordinates from.
 * @return Mouse coordinates in world space coordinates.
 */
sf::Vector2f Menu::getMouseCoordinatesFromWindow(const sf::RenderWindow& window
) const {
    const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getMainView());
}

/** Highlights the menu option currently hovered by the mouse.
 * @param mouse_pos Mouse position in world coordinates.
 */
void Menu::highlightHoveredOption(const sf::Vector2f mouse_pos) {
    if (!showAbout) {
        for (int i = 1; i < texts.size(); ++i) {
            if (texts[i].getGlobalBounds().contains(mouse_pos)) {
                if (pos != i) {
                    texts[pos].setOutlineThickness(0);
                    pos = i;
                    texts[pos].setOutlineThickness(1);
                }
                break;
            }
        }
    }
}

bool Menu::handleAboutEvents(const std::optional<sf::Event>& event) {
    if (!event.has_value()) {
        return false;
    }
    if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            showAbout = false;
            isItemClicked = false;
            pressed = false;
            return true;
        }
    }
    if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape ||
            key->code == sf::Keyboard::Key::Enter ||
            key->code == sf::Keyboard::Key::Space) {
            showAbout = false;
            isItemClicked = false;
            pressed = false;
            return true;
        }
    }
    // while about is shown, ignore other menu inputs
    return false;
}

void Menu::mouseButtonClicked(
    const sf::Vector2f mouse_pos, const std::optional<sf::Event>& event
) {
    if (!event.has_value()) {
        return;
    }
    if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
        mouseEvent->button == sf::Mouse::Button::Left) {
        Logger::info(
            "Clicked at world position ({}, {})", mouse_pos.x, mouse_pos.y
        );

        for (int i = 1; i < texts.size(); ++i) {
            if (texts[i].getGlobalBounds().contains(mouse_pos)) {
                pos = i;
                Logger::info("Clicked on option " + texts[i].getString());
                handleSelection();
                break;
            }
        }
    }
}

void Menu::handleMenuEvents(AudioManager& audioManager) {
    auto& window = windowManager->getWindow();

    const auto mouse_pos = getMouseCoordinatesFromWindow(window);
    mouseSprite.setPosition(mouse_pos);
    highlightHoveredOption(mouse_pos);

    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            const sf::Vector2u newSize(resized->size.x, resized->size.y);

            windowManager->handleResizeEvent(newSize);
        }

        // If the About overlay is visible, handle only its close inputs here
        if (showAbout) {
            if (handleAboutEvents(event)) {
                break;
            }
            continue;
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
        if (!isItemClicked) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                handleSelection();
                break;
            }
        }

        if (event->is<sf::Event::MouseButtonPressed>()) {
            mouseButtonClicked(mouse_pos, event);
        }
    }
}

// Helper to update the selection visually
void Menu::updateSelection(const std::size_t newPos) {
    texts[pos].setOutlineThickness(0); // deselect previous
    pos = static_cast<int>(newPos);
    texts[pos].setOutlineThickness(1); // select new
    isItemClicked = false;
    pressed = true;
}

// Helper to handle menu selection actions
void Menu::handleSelection() {
    isItemClicked = true;

    switch (pos) {
        case 1:
            Logger::info("Play selected");
            break;
        case 2: {
            const SaveGameManager saveManager;
            const auto playerPos = controller->getPlayer().getPosition();
            GameState state;
            state.player.x = playerPos.x;
            state.player.y = playerPos.y;
            saveManager.saveGame(state);
            Logger::info("Save selected");
            isItemClicked = false;
            pressed = false;
            break;
        }
        case 3:
            Logger::info("Options selected");
            isItemClicked = false;
            pressed = false;
            break;
        case 4:
            // TODO read from config file
            aboutText = "Joanna's Farm\n\n"
                        "A small farming game prototype.\n\n"
                        "Controls:\n"
                        "- W/S: navigate menu\n"
                        "- Space/Enter: select\n\n"
                        "Press Escape, Enter, Space or click to close.";
            showAbout = true;
            isItemClicked = false;
            pressed = false;
            break;
        case 5:
            windowManager->getWindow().close();
            break;
        default:
            break;
    }
}

void Menu::render(
    RenderEngine& render_engine, TileManager& tileManager,
    std::list<std::unique_ptr<Interactable>>& interactable,
    std::shared_ptr<DialogueBox> dialogueBox
) const {
    windowManager->setView(windowManager->getMainView());

    windowManager->getWindow().clear();

    windowManager->pollEvents();

    render_engine.render(
        windowManager->getWindow(), controller->getPlayer(), tileManager,
        interactable, dialogueBox
    );

    for (std::size_t i = 0; i < texts.size(); ++i) {
        windowManager->getWindow().draw(backgrounds[i]);
        windowManager->getWindow().draw(texts[i]);
    }
    renderMouseCursor(windowManager->getWindow());

    // If about overlay is active, draw centered semi-transparent box + text
    if (showAbout) {
        auto& win = windowManager->getWindow();
        const sf::View mainView = windowManager->getMainView();
        const sf::Vector2f center = mainView.getCenter();

        sf::Text about(font);
        about.setString(aboutText);
        about.setCharacterSize(8);
        about.setFillColor(sf::Color::White);
        about.setLetterSpacing(1.f);
        about.setLineSpacing(1.2f);

        const sf::FloatRect bounds = about.getLocalBounds();
        constexpr float padding = 16.f;
        const sf::Vector2f boxSize(
            (bounds.size.y + padding) * 2.f, (bounds.size.x + padding) * 2.f
        );

        about.setOrigin({ (bounds.position.x + bounds.size.y) / 2.f,
                          (bounds.position.x + bounds.size.y) / 2.f });
        about.setPosition(center);

        sf::RectangleShape box;
        box.setSize(boxSize);
        box.setOrigin({ boxSize.x / 2.f, boxSize.y / 2.f });
        box.setPosition(center);
        box.setFillColor(sf::Color(20, 20, 30, 220));
        box.setOutlineColor(sf::Color(200, 200, 200, 180));
        box.setOutlineThickness(1.f);

        // Draw overlay (box then text)
        win.draw(box);
        win.draw(about);
    }

    windowManager->pollEvents();
    windowManager->getWindow().display();
}

void Menu::show(
    RenderEngine& render_engine, TileManager& tileManager,
    std::list<std::unique_ptr<Interactable>>& interactable,
    const std::shared_ptr<DialogueBox>& dialogueBox, AudioManager& audioManager
) {
    const auto tmp = windowManager->getWindow().getView();
    while (windowManager->getWindow().isOpen() && !isItemClicked) {

        handleMenuEvents(audioManager);
        render(render_engine, tileManager, interactable, dialogueBox);
    }
    windowManager->getWindow().setView(tmp);
}

void Menu::renderMouseCursor(sf::RenderTarget& window) const {
    window.draw(mouseSprite);
}
