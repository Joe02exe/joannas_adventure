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

Menu::Menu(
    WindowManager& windowManager, Controller& controller,
    TileManager& tileManager, AudioManager& audioManager
)
    : windowManager(&windowManager), controller(&controller),
      audioManager(&audioManager), tileManager(&tileManager),
      mouseSprite(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/cursor.png"
      )) {
    mouseSprite.setOrigin({ 0.f, 0.f });

    font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
    font.setSmooth(false);

    // Initialize Default Menu
    resetToDefaultMenu();
}

void Menu::setOptions(const std::vector<std::string>& newOptions) {
    options = newOptions;
    // Reset selection to first clickable item (index 1 usually, as 0 is title)
    selectedIndex = (options.size() > 1) ? 1 : 0;
    rebuildUI();
}

void Menu::resetToDefaultMenu() {
    setOptions({ "Joanna's Farm", "New game", "Load game", "Save", "Options",
                 "About", "Quit" });
}

void Menu::rebuildUI() {
    menuTexts.clear();
    menuBackgrounds.clear();

    if (options.empty())
        return;

    // 1. Calculate Total Height to center vertically
    float totalHeight = 0.f;
    for (size_t i = 0; i < options.size(); ++i) {
        float size = (i == 0) ? static_cast<float>(FONT_SIZE_TITLE)
                              : static_cast<float>(FONT_SIZE_ITEM);
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
        text.setFillColor(COLOR_TEXT_NORMAL
        ); // We handle highlight in render/update now
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
        background.setPosition({ view.getCenter().x - 5.f,
                                 std::floor(currentY) });

        menuTexts.push_back(std::move(text));
        menuBackgrounds.push_back(std::move(background));

        currentY += static_cast<float>(charSize) + MENU_SPACING;
    }
}

sf::Vector2f Menu::getMouseWorldPos() const {
    auto& window = windowManager->getWindow();
    const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getMainView());
}

void Menu::handleHover(const sf::Vector2f& mousePos) {
    if (showAbout)
        return;

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
                    (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape) ||
                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) ||
                (event->is<sf::Event::MouseButtonPressed>())) {
                showAbout = false;
            }
            continue; // Skip standard menu inputs
        }

        // --- Standard Menu Inputs ---
        if (event->is<sf::Event::KeyPressed>()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                updateSelection(-1);
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                updateSelection(1);
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                executeSelection();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                isMenuOpen = false;
            }
        } else if (event->is<sf::Event::MouseButtonPressed>()) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                // Check if we are clicking the currently hovered item
                if (selectedIndex >= 1 &&
                    selectedIndex < static_cast<int>(menuTexts.size())) {
                    if (menuTexts[selectedIndex].getGlobalBounds().contains(
                            getMouseWorldPos()
                        )) {
                        audioManager->play_sfx(SfxId::Click);
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
    if (newIndex < 1)
        newIndex = 1;
    if (newIndex >= static_cast<int>(menuTexts.size()))
        newIndex = static_cast<int>(menuTexts.size()) - 1;

    selectedIndex = newIndex;
}

void Menu::executeSelection() {
    // Safeguard
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(options.size()))
        return;

    std::string choice = options[selectedIndex];
    Logger::info("Menu selected: " + choice);

    // Map string options to actions.
    // Note: A switch/case on string is not valid in C++, so we use if/else or
    // map logic. Alternatively, rely on the index (1=Play, 2=Save), but that
    // breaks if menu order changes. For now, sticking to the index logic from
    // your original code but checking safety.

    if (choice == "Save") {
        loadingInteraction = false;
        auto& player = controller->getPlayer();

        GameState state;
        state.player.x = player.getPosition().x;
        state.player.y = player.getPosition().y;
        state.player.health = player.getHealth();

        // Save Inventory
        for (const auto& item : player.getInventory().listItems()) {
            state.inventory.items.push_back({ item.item.id, item.quantity });
        }

        for (const auto& object : tileManager->getRenderObjects()) {
            state.map.items.push_back({ object.id, object.gid,
                                        object.position.x, object.position.y });
        }
        stateToSave = state;

        // Update menu to show slots (Example of sub-menu logic)
        setOptions({ options[0], "Slot 1", "Slot 2", "Slot 3", "Back" });
    } else if (choice == "New game") {
        isMenuOpen = false; // Start new game
        tileManager->reloadObjectsFromTileson();
        controller->getPlayer().getInventory().clear();
    } else if (choice == "Load game") {
        loadingInteraction = true;
        setOptions({ options[0], "Slot 1", "Slot 2", "Slot 3", "Back" });
    } else if (choice == "Options") {
        // Placeholder
    } else if (choice == "About") {
        aboutTextContent = "Joanna's Farm\n\n"
                           "A small RPG game.\n\n"
                           "Controls:\n"
                           "WASD : navigate\n"
                           "Shift : sprint\n"
                           "T : Talk to opponent\n"
                           "E : Display inventory\n"
                           "Space : pickup item\n\n"
                           "Press Escape or Click to close.";
        showAbout = true;
    } else if (choice == "Quit") {
        windowManager->getWindow().close();
        isMenuOpen = false;
    } else if (choice == "Back") {
        resetToDefaultMenu();
    } else if (choice.find("Slot") != std::string::npos) {
        Logger::info("Selected Save Slot: " + choice);
        const SaveGameManager saveManager;
        std::string slotNumberStr = choice.substr(choice.find(' ') + 1);
        if (loadingInteraction) {
            SaveGameManager manager;
            GameState state = manager.loadGame(slotNumberStr);

            controller->getPlayer().setPosition(
                sf::Vector2f(state.player.x, state.player.y)
            );
            controller->getPlayerView().setCenter(
                sf::Vector2f(state.player.x, state.player.y)
            );
            controller->getMiniMapView().setCenter(
                sf::Vector2f(state.player.x, state.player.y)
            );
            controller->getPlayer().getInventory().loadState(state.inventory);
            tileManager->loadObjectsFromSaveGame(state.map.items);
            isMenuOpen = false;
        } else {
            saveManager.saveGame(stateToSave, slotNumberStr);
            Logger::info("Saved game to slot " + choice);
        }
        // Maybe go back to main menu after selecting slot?
        resetToDefaultMenu();
    }
}

void Menu::render(
    RenderEngine& render_engine, TileManager& tileManager,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox
) {

    auto& window = windowManager->getWindow();

    // 1. Draw Game World (as background)
    windowManager->setView(windowManager->getMainView());
    window.clear();

    render_engine.render(
        window, controller->getPlayer(), tileManager, entities, dialogueBox, 0.f
    );

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
            menuTexts[i].setString(
                "> " + options[i] + " <"
            ); // Optional stylistic choice
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
    constexpr float padding = 6.f;
    sf::Vector2f boxSize(
        (textBounds.size.x + padding) * 2.f, (textBounds.size.y + padding) * 2.f
    );

    // Center the text logic
    // Note: The origin math in your original code was a bit specific to the
    // font/bounds. Standard centering:
    // textObj.setOrigin({ textBounds.size.y / 2.f, textBounds.size.x / 2.f });
    // textObj.setOrigin({ 0,0 });
    // textObj.setPosition(center);

    const sf::View& view = windowManager->getMainView();
    float startY = view.getCenter().y - (boxSize.x / 2.f) + 75.f;
    textObj.setOrigin({ textBounds.size.x / 2.f, 0.f });
    textObj.setPosition({ view.getCenter().x, startY });

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

void Menu::show(
    RenderEngine& render_engine, TileManager& tileManager,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox, AudioManager& audioManager
) {

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
