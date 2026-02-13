#include "joanna/systems/menu.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/savegamemanager.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include <cmath>
#include <sys/stat.h>
#include <utility>

#include "joanna/core/game.h"
#include "joanna/entities/interactables/chest.h"
#include "joanna/entities/interactables/stone.h"
#include "joanna/entities/npc.h"

namespace {
const sf::Color COLOR_TEXT_NORMAL = sf::Color::Black;
const sf::Color COLOR_TEXT_SELECTED =
    sf::Color(50, 200, 50); // Greenish highlight
const sf::Color COLOR_BG_NORMAL = sf::Color(50, 50, 50, 200);
const float MENU_SPACING = 20.f;
const unsigned int FONT_SIZE_TITLE = 60;
const unsigned int FONT_SIZE_ITEM = 40;
} // namespace

Menu::Menu(
    WindowManager& windowManager, Controller& controller,
    TileManager& tileManager, AudioManager& audioManager,
    std::list<std::unique_ptr<Entity>>& entities, Game& game
)
    : windowManager(&windowManager), controller(&controller),
      tileManager(&tileManager), audioManager(&audioManager),
      entities(&entities), game(&game),
      mouseSprite(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/cursor.png"
      )) {
    mouseSprite.setOrigin({ 0.f, 0.f });
    mouseSprite.setScale({ 3.f, 3.f });

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
    std::vector<std::string> opts;
    opts.emplace_back("Joanna's Adventure");
    if (canResume) {
        opts.emplace_back("Resume");
    }
    opts.emplace_back("New game");
    opts.emplace_back("Load game");
    opts.emplace_back("Save");
    opts.emplace_back("About");
    opts.emplace_back("Quit");
    setOptions(opts);
}

void Menu::rebuildUI() {
    menuTexts.clear();
    menuBackgrounds.clear();

    if (options.empty()) {
        return;
    }

    // 1. Calculate Total Height to center vertically
    float totalHeight = 0.f;
    for (size_t i = 0; i < options.size(); ++i) {
        float size = (i == 0) ? static_cast<float>(FONT_SIZE_TITLE)
                              : static_cast<float>(FONT_SIZE_ITEM);
        totalHeight += size + MENU_SPACING;
    }
    totalHeight -= MENU_SPACING; // Remove trailing spacing

    const sf::View& view = windowManager->getUiView();
    float startY = view.getCenter().y - (totalHeight / 2.f);
    float currentY = startY;

    // 2. Build Objects
    for (size_t i = 0; i < options.size(); ++i) {
        unsigned int charSize = (i == 0) ? FONT_SIZE_TITLE : FONT_SIZE_ITEM;

        // Text Setup
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(charSize);
        text.setFillColor(COLOR_TEXT_NORMAL);
        text.setStyle(sf::Text::Regular);
        text.setOutlineThickness(2.0f);
        text.setOutlineColor(sf::Color::White);
        text.setLetterSpacing(2.f);

        // Center Horizontally
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.size.x / 2.f, 0.f });
        text.setPosition({ view.getCenter().x, std::floor(currentY) });
        menuTexts.push_back(std::move(text));
        currentY += static_cast<float>(charSize) + MENU_SPACING;
    }
}

sf::Vector2f Menu::getMouseWorldPos() const {
    auto& window = windowManager->getWindow();
    const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getUiView());
}

void Menu::handleHover(const sf::Vector2f& mousePos) {
    if (showAbout) {
        return;
    }

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

        if (!isMenuOpen) {
            break;
        }

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
    if (newIndex < 1) {
        newIndex = 1;
    }
    if (newIndex >= static_cast<int>(menuTexts.size())) {
        newIndex = static_cast<int>(menuTexts.size()) - 1;
    }

    selectedIndex = newIndex;
}

void Menu::executeSelection() {
    // Safeguard
    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(options.size())) {
        return;
    }

    std::string choice = options[selectedIndex];
    Logger::info("Menu selected: " + choice);

    // Map string options to actions.
    // Note: A switch/case on string is not valid in C++, so we use if/else or
    // map logic. Alternatively, rely on the index (1=Play, 2=Save), but that
    // breaks if menu order changes. For now, sticking to the index logic from
    // your original code but checking safety.

    if (choice == "Resume") {
        isMenuOpen = false;
    } else if (choice == "Save") {
        loadingInteraction = false;
        auto& player = controller->getPlayer();

        GameState state;
        state.player.x = player.getPosition().x;
        state.player.y = player.getPosition().y;
        state.player.health = player.getHealth();
        state.player.visitedInteractions = player.getVisitedInteractions();
        state.player.attack = player.getStats().attack;
        state.player.defense = player.getStats().defense;
        state.player.level = player.getLevel();
        state.player.currentExp = player.getCurrentExp();
        state.player.expToNextLevel = player.getExpToNextLevel();

        // Save Inventory
        for (const auto& item : player.getInventory().listItems()) {
            state.inventory.items.push_back({ item.item.id, item.quantity });
        }

        for (const auto& object : tileManager->getRenderObjects()) {
            state.map.items.push_back({ object.id, object.gid,
                                        object.position.x, object.position.y });
        }
        stateToSave = state;

        SaveGameManager manager;
        std::vector<std::string> slotOptions;
        slotOptions.push_back(options[0]); // Keep the title

        // Generate strings for Slot 1, 2, 3
        for (int i = 1; i <= 3; ++i) {
            std::string idx = std::to_string(i);
            std::string info = manager.getSaveInfo(idx);
            // Result: "Slot 1 - Empty" or "Slot 1 - 2023-10-25 14:30"
            std::string optionStr = "Slot ";
            optionStr += idx;
            optionStr += " - ";
            optionStr += info;

            slotOptions.push_back(optionStr);
        }
        slotOptions.emplace_back("Back");

        setOptions(slotOptions);
    } else if (choice == "New game") {
        isMenuOpen = false; // Start new game
        tileManager->reloadObjectsFromTileson();
        controller->getPlayer().getInventory().clear();
        controller->getPlayer().setHealth(200);
        controller->getPlayer().setPosition({ 150.f, 400.f });
        controller->getPlayer().resetInteractions();
        controller->getPlayer().resetStats();
        windowManager->setCenter({ 150.f, 400.f });
        game->resetEntities();
    } else if (choice == "Load game") {
        loadingInteraction = true;
        SaveGameManager manager;
        std::vector<std::string> slotOptions;
        slotOptions.emplace_back(options[0]);

        for (int i = 1; i <= 3; ++i) {
            std::string idx = std::to_string(i);
            std::string info = manager.getSaveInfo(idx);
            std::string optionStr = "Slot ";
            optionStr += idx;
            optionStr += " - ";
            optionStr += info;

            slotOptions.push_back(optionStr);
        }
        slotOptions.emplace_back("Back");
        setOptions(slotOptions);
    } else if (choice == "Options") {
        // Placeholder
    } else if (choice == "About") {
        aboutTextContent = "Joanna's Adventure\n\n"
                           "A small RPG game.\n\n"
                           "Controls:\n"
                           "WASD : navigate\n"
                           "Shift : sprint\n"
                           "T : Talk to opponent\n"
                           "E : Apply items in inventory\n"
                           "1-9 : Select inventory slot\n"
                           "Space : pickup item / dialogue\n"
                           "P / Escape : Menu\n\n"
                           "Press Escape or Click to close.";
        showAbout = true;
    } else if (choice == "Quit") {
        windowManager->getWindow().close();
        isMenuOpen = false;
    } else if (choice == "Back") {
        resetToDefaultMenu();
    } else if (choice.find("Slot") != std::string::npos) {
        std::stringstream ss(choice);
        std::string temp, slotNumStr;
        ss >> temp;       // reads "Slot"
        ss >> slotNumStr; // reads "1" (stops at space before "-")

        Logger::info("Selected Save Slot ID: " + slotNumStr);
        const SaveGameManager saveManager;
        if (loadingInteraction) {
            SaveGameManager manager;
            game->resetEntities();
            if (choice.find("Empty") != std::string::npos) {
                Logger::info("Cannot load empty slot.");
                return; // Do nothing if they click an empty slot while loading
            }

            GameState state = manager.loadGame(slotNumStr);

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
            controller->getPlayer().setHealth(state.player.health);

            controller->getPlayer().getStats().attack = state.player.attack;
            controller->getPlayer().getStats().defense = state.player.defense;
            controller->getPlayer().setLevel(state.player.level);
            controller->getPlayer().setCurrentExp(state.player.currentExp);
            controller->getPlayer().setExpToNextLevel(
                state.player.expToNextLevel
            );

            controller->getPlayer().setInteractions(
                state.player.visitedInteractions
            );
            tileManager->loadObjectsFromSaveGame(state.map.items);
            isMenuOpen = false;

            // move entities back to default positions or states if needed
            bool guard1Reset = state.player.visitedInteractions.count(
                                   "assets/player/npc/guard1.png_Bring key"
                               ) != 0u;
            bool guard2Reset = state.player.visitedInteractions.count(
                                   "assets/player/npc/guard2.png_Bring key"
                               ) != 0u;
            bool stoneLeftReset =
                state.player.visitedInteractions.count("left") != 0u;
            bool stoneRightReset =
                state.player.visitedInteractions.count("right") != 0u;
            bool chestOpened =
                state.player.visitedInteractions.count("chestOpened") != 0u;
            bool goblinDead =
                state.player.visitedInteractions.count("goblinDead") != 0u;

            // Use an iterator instead of a range-based for loop
            for (auto it = entities->begin(); it != entities->end();
                 /* increment handled inside */) {
                auto& ptr = *it; // Access the unique_ptr

                if (!ptr) {
                    ++it;
                    continue;
                }

                auto p = ptr.get();
                bool itemRemoved =
                    false; // Flag to track if we deleted something

                // --- NPC Logic ---
                if (auto* npc = dynamic_cast<NPC*>(p)) {
                    if (npc->getUniqueSpriteId() ==
                            "assets/player/npc/guard1.png" &&
                        guard1Reset) {
                        npc->setPosition(
                            npc->getPosition() - sf::Vector2f(50.f, 50.f)
                        );
                    }
                    if (npc->getUniqueSpriteId() ==
                            "assets/player/npc/guard2.png" &&
                        guard2Reset) {
                        npc->setPosition(
                            npc->getPosition() - sf::Vector2f(50.f, 50.f)
                        );
                    }
                }

                // --- Chest Logic ---
                if (auto* chest = dynamic_cast<Chest*>(p)) {
                    if (chest->getChestId() == "chest" && chestOpened) {
                        chest->setChestOpen(true);
                    }
                }

                // --- Stone Logic ---
                if (auto* stone = dynamic_cast<Stone*>(p)) {
                    if ((stone->getStoneId() == "left" && stoneLeftReset) ||
                        (stone->getStoneId() == "right" && stoneRightReset)) {
                        it = entities->erase(it);
                        itemRemoved = true;
                    }
                }

                if (itemRemoved) {
                    continue;
                }
                // --- Goblin Logic ---
                if (auto* enemy = dynamic_cast<Enemy*>(p)) {
                    if (enemy->getType() == Enemy::EnemyType::Goblin &&
                        goblinDead) {
                        // Remove the goblin safely
                        it = entities->erase(it);
                        game->resetEnemyPointer();
                        itemRemoved = true;
                    }
                }

                // Only increment the iterator if we didn't remove anything
                // (erase() automatically advances 'it' to the next item)
                if (!itemRemoved) {
                    ++it;
                }
            }
        } else {
            saveManager.saveGame(stateToSave, slotNumStr);
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

    // draw black screen with alpha channel
    sf::RectangleShape blackScreen(sf::Vector2f(window.getSize()));
    blackScreen.setFillColor(sf::Color(0, 0, 0, 153));
    window.draw(blackScreen);

    // 2. Draw Menu UI
    windowManager->setView(windowManager->getUiView());
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
    const sf::View& view = windowManager->getUiView();
    const sf::Vector2f center = view.getCenter();

    sf::Text textObj(font);
    textObj.setString(aboutTextContent);
    textObj.setCharacterSize(30);
    textObj.setFillColor(sf::Color::White);
    textObj.setLetterSpacing(1.f);
    textObj.setLineSpacing(1.2f);

    sf::FloatRect textBounds = textObj.getLocalBounds();
    constexpr float padding = 20.f;
    sf::Vector2f boxSize(
        (textBounds.size.x + padding) * 2.f, (textBounds.size.y + padding) * 2.f
    );

    float startY = view.getCenter().y - (boxSize.x / 2.f) + 100.f;
    textObj.setOrigin({ textBounds.size.x / 2.f, 0.f });
    textObj.setPosition({ view.getCenter().x, startY });

    sf::RectangleShape box;
    box.setSize(boxSize);
    box.setOrigin(boxSize / 2.f);
    box.setPosition(center);
    box.setFillColor(sf::Color(20, 20, 30, 220));
    box.setOutlineColor(sf::Color(200, 200, 200, 180));
    box.setOutlineThickness(2.f);

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

    // Wait for keys to be released to prevent immediate re-triggering
    while (windowManager->getWindow().isOpen() &&
           (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))) {
        windowManager->pollEvents();
    }

    windowManager->getWindow().setView(originalView);
}