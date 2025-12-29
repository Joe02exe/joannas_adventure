#include "joanna/systems/menu.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/savegamemanager.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include <utility>

namespace {
const sf::Color COLOR_TEXT_NORMAL = sf::Color::Black;
const sf::Color COLOR_TEXT_SELECTED =
    sf::Color(50, 200, 50); // Greenish highlight
const sf::Color COLOR_BG_NORMAL = sf::Color(50, 50, 50, 200);
const float MENU_SPACING = 7.5f;
const unsigned int FONT_SIZE_TITLE = 18;
const unsigned int FONT_SIZE_ITEM = 14;
} // namespace

Menu::Menu(WindowManager& windowManager, Controller& controller)
    : windowManager(&windowManager), controller(&controller),
      mouseSprite(
          ResourceManager<sf::Texture>::getInstance()->get(
              "assets/buttons/cursor.png"
          )
      ) {
    mouseSprite.setOrigin({ 0.f, 0.f });

    font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
    font.setSmooth(false);

    // Initialize Default Menu
    // TODO: Ideally read this from a config file
    setOptions({ "Joanna's Farm", "Play", "Save", "Options", "About", "Quit" });
}

void Menu::setOptions(const std::vector<std::string>& newOptions) {
    options = newOptions;
    // Reset selection to first clickable item (index 1 usually, as 0 is title)
    selectedIndex = (options.size() > 1) ? 1 : 0;
    rebuildUI();
}

void Menu::rebuildUI() {
    menuTexts.clear();
    menuBackgrounds.clear();

    if (options.empty()) return;

    // 1. Calculate Total Height to center vertically
    float totalHeight = 0.f;
    for (size_t i = 0; i < options.size(); ++i) {
        float size = (i == 0) ? FONT_SIZE_TITLE : FONT_SIZE_ITEM;
        totalHeight += size + MENU_SPACING;
    }
    totalHeight -= MENU_SPACING; // Remove trailing spacing

    const sf::View& view = windowManager->getMainView();
    float startY = view.getCenter().y - (totalHeight / 2.f);
    float currentY = startY;

    // 2. Build Objects
    for (size_t i = 0; i < options.size(); ++i) {
        unsigned int charSize = (i == 0) ? FONT_SIZE_TITLE : FONT_SIZE_ITEM;

        // Text Setup
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(charSize);
        text.setFillColor(COLOR_TEXT_NORMAL); // We handle highlight in render/update now
        text.setStyle(sf::Text::Regular);
        text.setOutlineThickness(1.5f);
        text.setOutlineColor(sf::Color::White);
        text.setLetterSpacing(2.f);

        // Center Horizontally
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition({ view.getCenter().x, std::floor(currentY) });

        // Background Setup
        sf::RectangleShape background;
        background.setSize({ bounds.size.x + 10.f, bounds.size.y + 6.f });
        background.setFillColor(COLOR_BG_NORMAL);
        background.setOrigin({ bounds.size.x / 2.f, 0.f });
        background.setPosition({ view.getCenter().x - 5.f, std::floor(currentY) });

        menuTexts.push_back(std::move(text));
        menuBackgrounds.push_back(std::move(background));

        currentY += charSize + MENU_SPACING;
    }
}

sf::Vector2f Menu::getMouseWorldPos() const {
    auto& window = windowManager->getWindow();
    const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getMainView());
}

void Menu::handleHover(const sf::Vector2f& mousePos) {
    if (showAbout) return;

    // Start at 1 to skip Title
    for (size_t i = 1; i < menuTexts.size(); ++i) {
        if (menuTexts[i].getGlobalBounds().contains(mousePos)) {
            if (static_cast<int>(i) != selectedIndex) {
                selectedIndex = static_cast<int>(i);
            }
            break;
        }
    }
}

void Menu::handleInput(sf::Window& window) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            isMenuOpen = false;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            windowManager->handleResizeEvent(resized->size);
            rebuildUI(); // Re-center items after resize
        }

        // --- About Overlay Inputs ---
        if (showAbout) {
            if (event->is<sf::Event::KeyPressed>() &&
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) ||
                (event->is<sf::Event::MouseButtonPressed>())) {
                showAbout = false;
            }
            continue; // Skip standard menu inputs
        }

        // --- Standard Menu Inputs ---
        if (event->is<sf::Event::KeyPressed>()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                updateSelection(-1);
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                updateSelection(1);
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                executeSelection();
            }
        } else if (event->is<sf::Event::MouseButtonPressed>()) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                // Check if we are clicking the currently hovered item
                if (selectedIndex >= 1 && selectedIndex < static_cast<int>(menuTexts.size())) {
                    if (menuTexts[selectedIndex].getGlobalBounds().contains(getMouseWorldPos())) {
                        executeSelection();
                    }
                }
            }
        }
    }
}

void Menu::updateSelection(int direction) {
    int newIndex = selectedIndex + direction;
    // Constrain between 1 (Skip title) and size-1
    if (newIndex < 1) newIndex = 1;
    if (newIndex >= static_cast<int>(menuTexts.size())) newIndex = static_cast<int>(menuTexts.size()) - 1;

    selectedIndex = newIndex;
}

void Menu::executeSelection() {
    // Safeguard
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(options.size())) return;

    std::string choice = options[selectedIndex];
    Logger::info("Menu selected: " + choice);

    // Map string options to actions.
    // Note: A switch/case on string is not valid in C++, so we use if/else or map logic.
    // Alternatively, rely on the index (1=Play, 2=Save), but that breaks if menu order changes.
    // For now, sticking to the index logic from your original code but checking safety.

    if (choice == "Play") {
        isMenuOpen = false; // Breaks the loop in show()
    }
    else if (choice == "Save") {
        auto& player = controller->getPlayer();

        GameState state;
        state.player.x = player.getPosition().x;
        state.player.y = player.getPosition().y;
        state.player.health = player.getHealth();

        // Save Inventory
        for (const auto& item : player.getInventory().listItems()) {
            state.inventory.items.push_back({ item.item.id, item.quantity });
        }
        stateToSave = state;

        // Update menu to show slots (Example of sub-menu logic)
        setOptions({ options[0], "Slot 1", "Slot 2", "Slot 3", "Back" });
    }
    else if (choice == "Options") {
        // Placeholder
    }
    else if (choice == "About") {
        aboutTextContent = "Joanna's Farm\n\n"
                           "A small farming game prototype.\n\n"
                           "Controls:\n"
                           "- W/S: navigate menu\n"
                           "- Space/Enter: select\n\n"
                           "Press Escape or Click to close.";
        showAbout = true;
    }
    else if (choice == "Quit") {
        windowManager->getWindow().close();
        isMenuOpen = false;
    }
    else if (choice == "Back") {
        // Restore main menu
        setOptions({ "Joanna's Farm", "Play", "Save", "Options", "About", "Quit" });
    }
    else if (choice.find("Slot") != std::string::npos) {
        Logger::info("Selected Save Slot: " + choice);
        const SaveGameManager saveManager;
        std::string slotNumberStr = choice.substr(choice.find(" ") + 1);
        saveManager.saveGame(stateToSave, slotNumberStr);
        Logger::info("Saved game to slot " + choice);
        // Maybe go back to main menu after selecting slot?
        setOptions({ "Joanna's Farm", "Play", "Save", "Options", "About", "Quit" });
    }
}

void Menu::render(RenderEngine& render_engine, TileManager& tileManager,
                  std::list<std::unique_ptr<Entity>>& entities,
                  const std::shared_ptr<DialogueBox>& dialogueBox) {

    auto& window = windowManager->getWindow();

    // 1. Draw Game World (as background)
    windowManager->setView(windowManager->getMainView());
    window.clear();

    render_engine.render(window, controller->getPlayer(), tileManager, entities, dialogueBox, 0.f);

    // 2. Draw Menu UI
    renderMenuOptions(window);

    // 3. Draw Overlay
    if (showAbout) {
        renderAboutOverlay(window);
    }

    // 4. Draw Cursor
    window.draw(mouseSprite);

    window.display();
}

void Menu::renderMenuOptions(sf::RenderTarget& target) {
    for (size_t i = 0; i < menuTexts.size(); ++i) {
        // Visual Logic: Highlight selected item
        if (static_cast<int>(i) == selectedIndex && !showAbout) {
            menuTexts[i].setFillColor(COLOR_TEXT_SELECTED);
            menuTexts[i].setString("> " + options[i] + " <"); // Optional stylistic choice
            // Re-center if the string length changed
            sf::FloatRect bounds = menuTexts[i].getLocalBounds();
            menuTexts[i].setOrigin({ bounds.size.x / 2.f, 0.f });
        } else {
            menuTexts[i].setFillColor(COLOR_TEXT_NORMAL);
            menuTexts[i].setString(options[i]);
            // Re-center
            sf::FloatRect bounds = menuTexts[i].getLocalBounds();
            menuTexts[i].setOrigin({ bounds.size.x / 2.f, 0.f });
        }

        // Draw background (optional, uncomment if desired)
        // target.draw(menuBackgrounds[i]);
        target.draw(menuTexts[i]);
    }
}

void Menu::renderAboutOverlay(sf::RenderTarget& target) {
    const sf::Vector2f center = windowManager->getMainView().getCenter();

    sf::Text textObj(font);
    textObj.setString(aboutTextContent);
    textObj.setCharacterSize(8);
    textObj.setFillColor(sf::Color::White);
    textObj.setLetterSpacing(1.f);
    textObj.setLineSpacing(1.2f);

    sf::FloatRect textBounds = textObj.getLocalBounds();
    constexpr float padding = 16.f;
    sf::Vector2f boxSize((textBounds.size.x + padding) * 2.f, (textBounds.size.y + padding) * 2.f);

    // Center the text logic
    // Note: The origin math in your original code was a bit specific to the font/bounds.
    // Standard centering:
    textObj.setOrigin({ textBounds.size.y / 2.f, textBounds.size.x / 2.f });
    textObj.setPosition(center);

    sf::RectangleShape box;
    box.setSize(boxSize);
    box.setOrigin(boxSize / 2.f);
    box.setPosition(center);
    box.setFillColor(sf::Color(20, 20, 30, 220));
    box.setOutlineColor(sf::Color(200, 200, 200, 180));
    box.setOutlineThickness(1.f);

    target.draw(box);
    target.draw(textObj);
}

void Menu::show(RenderEngine& render_engine, TileManager& tileManager,
                std::list<std::unique_ptr<Entity>>& entities,
                const std::shared_ptr<DialogueBox>& dialogueBox,
                AudioManager& audioManager) {

    isMenuOpen = true;
    const auto originalView = windowManager->getWindow().getView();

    while (windowManager->getWindow().isOpen() && isMenuOpen) {

        // 1. Update Input
        sf::Vector2f mousePos = getMouseWorldPos();
        mouseSprite.setPosition(mousePos);
        handleHover(mousePos);
        handleInput(windowManager->getWindow());

        // 2. Render
        render(render_engine, tileManager, entities, dialogueBox);
    }

    windowManager->getWindow().setView(originalView);
}

/** Gets the mouse coordinates in world space from the given render window.
 *
 * @param window Render window to get mouse coordinates from.
 * @return Mouse coordinates in world space coordinates.
 */
/*
sf::Vector2f
Menu::getMouseCoordinatesFromWindow(const sf::RenderWindow& window) const {
    const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getMainView());
}

/** Highlights the menu option currently hovered by the mouse.
 * @param mouse_pos Mouse position in world coordinates.
 *//*
void Menu::highlightHoveredOption(const sf::Vector2f mouse_pos) {
    if (!showAbout) {
        for (int i = 1; i < texts.size(); ++i) {
            if (texts[i].getGlobalBounds().contains(mouse_pos)) {
                if (pos != i) {
                    texts[pos].setString(
                        texts[pos].getString().substring(
                            2, texts[pos].getString().getSize() - 2
                        )
                    );
                    pos = i;
                    texts[pos].setString("> " + texts[pos].getString());
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
    texts[pos].setString(
        texts[pos].getString().substring(
            2, texts[pos].getString().getSize() - 2
        )
    );
    pos = static_cast<int>(newPos);
    texts[pos].setString("> " + texts[pos].getString());
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
            state.player.health = controller->getPlayer().getHealth();
            const auto& inventory = controller->getPlayer().getInventory();
            for (const auto& item : inventory.listItems()) {
                ItemState itemState;
                itemState.id = item.item.id;
                itemState.quantity = item.quantity;
                state.inventory.items.push_back(itemState);
            }
            saveManager.saveGame(state);
            Logger::info("Save selected");
            isItemClicked = false;
            pressed = false;
            setOptions({ options[0], "Slot 1", "Slot 2", "Slot 3" });

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
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox
) const {
    windowManager->setView(windowManager->getMainView());

    windowManager->getWindow().clear();

    windowManager->pollEvents();

    render_engine.render(
        windowManager->getWindow(), controller->getPlayer(), tileManager,
        entities, dialogueBox, 0.f
    );

    for (std::size_t i = 0; i < texts.size(); ++i) {
        // windowManager->getWindow().draw(backgrounds[i]);
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

        about.setOrigin(
            { (bounds.position.x + bounds.size.y) / 2.f,
              (bounds.position.x + bounds.size.y) / 2.f }
        );
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
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox, AudioManager& audioManager
) {
    const auto tmp = windowManager->getWindow().getView();
    while (windowManager->getWindow().isOpen() && !isItemClicked) {

        handleMenuEvents(audioManager);
        render(render_engine, tileManager, entities, dialogueBox);
    }
    windowManager->getWindow().setView(tmp);
}

void Menu::renderMouseCursor(sf::RenderTarget& window) const {
    window.draw(mouseSprite);
}

void Menu::setOptions(const std::vector<std::string>& newOptions) {
    options = newOptions;
}*/