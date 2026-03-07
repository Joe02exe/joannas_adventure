#include "joanna/systems/menu.h"

#include "joanna/core/renderengine.h"
#include "joanna/core/savegamemanager.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

#include "joanna/core/graphics.h"
#include <cmath>
#include <spdlog/spdlog.h>
#include <utility>

#include "joanna/core/game.h"
#include "joanna/entities/interactables/chest.h"
#include "joanna/entities/interactables/stone.h"
#include "joanna/entities/npc.h"

namespace {
const jo::Color COLOR_TEXT_NORMAL = jo::Color::White;
const jo::Color COLOR_TEXT_SELECTED =
    jo::Color(50, 200, 50); // Greenish highlight
const jo::Color COLOR_BG_NORMAL = jo::Color(50, 50, 50, 200);
constexpr float MENU_SPACING = 20.f;
constexpr unsigned int FONT_SIZE_TITLE = 60;
constexpr unsigned int FONT_SIZE_ITEM = 40;
} // namespace

Menu::Menu(
    WindowManager& windowManager, Controller& controller,
    TileManager& tileManager, AudioManager& audioManager,
    std::list<std::unique_ptr<Entity>>& entities, Game& game
)
    : windowManager(&windowManager), controller(&controller),
      tileManager(&tileManager), audioManager(&audioManager),
      entities(&entities), game(&game),
      mouseSprite(ResourceManager<jo::Texture>::getInstance()->get(
          "assets/buttons/cursor.png"
      )) {
    mouseSprite.setOrigin({ 0.f, 0.f });
    mouseSprite.setScale({ 3.f, 3.f });

    font = &ResourceManager<jo::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
#ifdef MIYOO_BUILD
    font->setSmooth(false);
#endif
    // font->setSmooth(false); // Removed in SFML 3

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
        const float size = (i == 0) ? static_cast<float>(FONT_SIZE_TITLE)
                                    : static_cast<float>(FONT_SIZE_ITEM);
        totalHeight += size + MENU_SPACING;
    }
    totalHeight -= MENU_SPACING; // Remove trailing spacing

    const jo::View& view = windowManager->getUiView();
    const float startY = view.getCenter().y - (totalHeight / 2.f);
    float currentY = startY;

    // 2. Build Objects
    for (size_t i = 0; i < options.size(); ++i) {
        const unsigned int charSize =
            (i == 0) ? FONT_SIZE_TITLE : FONT_SIZE_ITEM;

        // Text Setup
        jo::Text text(*font);
        text.setString(options.at(i));
        text.setCharacterSize(charSize);
        text.setFillColor(COLOR_TEXT_NORMAL);
        text.setStyle(jo::Text::Regular);
        text.setOutlineThickness(2.0f);
        text.setOutlineColor(jo::Color::White);
        text.setLetterSpacing(2.f);

        // Center Horizontally (account for SFML's internal left-padding in
        // bounds.position.x)
        const jo::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                         bounds.position.y });
        text.setPosition({ view.getCenter().x, std::floor(currentY) });
        menuTexts.push_back(std::move(text));
        currentY += static_cast<float>(charSize) + MENU_SPACING;
    }
}

jo::Vector2f Menu::getMouseWorldPos() const {
    auto& window = windowManager->getWindow();
    const jo::Vector2i pixelPos = jo::Mouse::getPosition(window);
    return window.mapPixelToCoords(pixelPos, windowManager->getUiView());
}

void Menu::handleHover(const jo::Vector2f& mousePos) {
    if (showAbout) {
        return;
    }

    // Start at 1 to skip Title
    for (size_t i = 1; i < menuTexts.size(); ++i) {
        if (menuTexts.at(i).getGlobalBounds().contains(mousePos)) {
            if (static_cast<int>(i) != selectedIndex) {
                selectedIndex = static_cast<int>(i);
            }
            break;
        }
    }
}

void Menu::handleInput(jo::Window& window) {
    while (const std::optional<jo::Event> event = window.pollEvent()) {

        if (!isMenuOpen) {
            break;
        }

        if (event->is<jo::Event::Closed>()) {
            window.close();
            isMenuOpen = false;
        }

        if (const auto* resized = event->getIf<jo::Event::Resized>()) {
            windowManager->handleResizeEvent(
                jo::Vector2u(resized->size.x, resized->size.y)
            );
            rebuildUI();
        }

        // --- About Overlay Inputs ---
        if (showAbout) {
            if (event->is<jo::Event::KeyPressed>() &&
                    (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Escape) ||
                     jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Enter)) ||
                (event->is<jo::Event::MouseButtonPressed>())) {
                showAbout = false;
            }
            continue; // Skip standard menu inputs
        }

        // --- Standard Menu Inputs ---
        if (event->is<jo::Event::KeyPressed>()) {
            if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::W) ||
                jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Up)) {
                updateSelection(-1);
            } else if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::S) ||
                       jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Down)) {
                updateSelection(1);
            } else if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Space) ||
                       jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Enter)) {
                executeSelection();
            } else if (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Escape)) {
                isMenuOpen = false;
            }
        } else if (event->is<jo::Event::MouseButtonPressed>()) {
            if (jo::Mouse::isButtonPressed(jo::Mouse::Button::Left)) {
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

GameState Menu::createGameState(Player& player) {
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
        state.map.items.push_back({ object.id, object.gid, object.position.x,
                                    object.position.y });
    }
    return state;
}

void Menu::resetNewGame() const {
    spdlog::info("Menu::resetNewGame - Starting reset logic...");

    spdlog::info("Reloading objects from Tileson...");
    tileManager->reloadObjectsFromTileson();

    spdlog::info("Clearing inventory...");
    controller->getPlayer().getInventory().clear();

    spdlog::info("Resetting player entity...");
    controller->getPlayer().setHealth(200);
    controller->getPlayer().setPosition({ 150.f, 400.f });
    controller->getPlayer().resetInteractions();
    controller->getPlayer().resetStats();

    spdlog::info("Resetting viewport center...");
    windowManager->setCenter({ 150.f, 400.f });

    spdlog::info("Resetting game entities...");
    game->resetEntities();

    spdlog::info("Menu::resetNewGame - Complete!");
}

void Menu::loadEntityStates(const GameState& state) {
    // move entities back to default positions or states if needed
    const bool guard1Reset = state.player.visitedInteractions.count(
                                 "assets/player/npc/guard1.png_Bring key"
                             ) != 0u;
    const bool guard2Reset = state.player.visitedInteractions.count(
                                 "assets/player/npc/guard2.png_Bring key"
                             ) != 0u;
    const bool stoneLeftReset =
        state.player.visitedInteractions.count("left") != 0u;
    const bool stoneRightReset =
        state.player.visitedInteractions.count("right") != 0u;
    const bool chestOpened =
        state.player.visitedInteractions.count("chestOpened") != 0u;
    const bool goblinDead =
        state.player.visitedInteractions.count("goblinDead") != 0u;

    for (auto it = entities->begin(); it != entities->end();) {
        auto& ptr = *it;

        if (!ptr) {
            ++it;
            continue;
        }

        auto* p = ptr.get();
        bool itemRemoved = false;

        // --- NPC Logic ---
        if (auto* npc = dynamic_cast<NPC*>(p)) {
            if (npc->getUniqueSpriteId() == "assets/player/npc/guard1.png" &&
                guard1Reset) {
                npc->setPosition(npc->getPosition() - jo::Vector2f(50.f, 50.f));
            }
            if (npc->getUniqueSpriteId() == "assets/player/npc/guard2.png" &&
                guard2Reset) {
                npc->setPosition(npc->getPosition() - jo::Vector2f(50.f, 50.f));
            }
        }

        // --- Chest Logic ---
        if (auto* chest = dynamic_cast<Chest*>(p)) {
            if (chest->getChestId() == "chest" && chestOpened) {
                chest->setChestOpen(true);
            }
        }

        // --- Stone Logic ---
        if (const auto* stone = dynamic_cast<Stone*>(p)) {
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
        if (const auto* enemy = dynamic_cast<Enemy*>(p)) {
            if (enemy->getType() == Enemy::EnemyType::Goblin && goblinDead) {
                // Remove the goblin safely
                it = entities->erase(it);
                game->resetEnemyPointer();
                itemRemoved = true;
            }
        }

        if (!itemRemoved) {
            ++it;
        }
    }
}

bool Menu::loadGameState(
    const std::string& choice, const std::string& slotNumStr
) {
    const SaveGameManager manager;
    game->resetEntities();
    if (choice.find("Empty") != std::string::npos) {
        Logger::info("Cannot load empty slot.");
        return true;
    }

    GameState state = manager.loadGame(slotNumStr);

    controller->getPlayer().setPosition(
        jo::Vector2f(state.player.x, state.player.y)
    );
    controller->getPlayerView().setCenter(
        jo::Vector2f(state.player.x, state.player.y)
    );
    controller->getMiniMapView().setCenter(
        jo::Vector2f(state.player.x, state.player.y)
    );
    controller->getPlayer().getInventory().loadState(state.inventory);
    controller->getPlayer().setHealth(state.player.health);

    controller->getPlayer().getStats().attack = state.player.attack;
    controller->getPlayer().getStats().defense = state.player.defense;
    controller->getPlayer().setLevel(state.player.level);
    controller->getPlayer().setCurrentExp(state.player.currentExp);
    controller->getPlayer().setExpToNextLevel(state.player.expToNextLevel);

    controller->getPlayer().setInteractions(state.player.visitedInteractions);
    tileManager->loadObjectsFromSaveGame(state.map.items);
    isMenuOpen = false;

    loadEntityStates(state);
    return false;
}

void Menu::executeSelection() {
    // Safeguard
    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(options.size())) {
        return;
    }

    std::string choice = options.at(selectedIndex);
    Logger::info("Menu selected: " + choice);

    if (choice == "Resume") {
        isMenuOpen = false;
    } else if (choice == "Save") {
        loadingInteraction = false;
        auto& player = controller->getPlayer();

        stateToSave = createGameState(player);

        std::vector<std::string> slotOptions;
        slotOptions.push_back(options.at(0));

        for (int i = 1; i <= 3; ++i) {
            SaveGameManager manager;
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
    } else if (choice == "New game") {
        resetNewGame();
        isMenuOpen = false;
    } else if (choice == "Load game") {
        loadingInteraction = true;
        std::vector<std::string> slotOptions;
        slotOptions.emplace_back(options.at(0));

        for (int i = 1; i <= 3; ++i) {
            SaveGameManager manager;
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
    } else if (choice == "About") {
        aboutTextContent = "Joanna's Adventure\n\n"
                           "A small RPG game.\n\n"
                           "Controls:\n"
                           "up/down/left/right : navigate\n"
                           "Y : sprint\n"
                           "A/B : Talk / pickup item\n"
                           "X : Apply items in inventory\n"
                           "L/R : Switch Item in inventory\n"
                           "Start : Menu\n"
                           "Select : Map\n\n"
                           "Press Escape or Click to close.";
        showAbout = true;
    } else if (choice == "Quit") {
        windowManager->getWindow().close();
        isMenuOpen = false;
    } else if (choice == "Back") {
        resetToDefaultMenu();
    } else if (choice.find("Slot") != std::string::npos) {
        std::stringstream ss(choice);
        std::string temp;
        std::string slotNumStr;
        ss >> temp;
        ss >> slotNumStr;

        Logger::info("Selected Save Slot ID: " + slotNumStr);
        if (loadingInteraction) {
            if (loadGameState(choice, slotNumStr)) {
                return;
            }
        } else {
            const SaveGameManager saveManager;
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

    // background
    windowManager->setView(windowManager->getMainView());
    window.clear();

    render_engine.render(
        window, controller->getPlayer(), tileManager, entities, dialogueBox, 0.f
    );

    // draw background
    auto view = window.getView();

    jo::RectangleShape blackScreen(view.getSize());
    blackScreen.setPosition(view.getCenter() - view.getSize() / 2.f);
    blackScreen.setFillColor(jo::Color(0, 0, 0, 153));
    window.draw(blackScreen);

    // draw menu options
    windowManager->setView(windowManager->getUiView());

    if (!showAbout) {
        renderMenuOptions(window);
    } else {
        renderAboutOverlay(window);
    }

    window.draw(mouseSprite);

    window.display();
}

void Menu::renderMenuOptions(jo::RenderTarget& target) {
    for (size_t i = 0; i < menuTexts.size(); ++i) {
        // Visual Logic: Highlight selected item
        if (static_cast<int>(i) == selectedIndex && !showAbout) {
            menuTexts.at(i).setFillColor(COLOR_TEXT_SELECTED);
            menuTexts.at(i).setString(
                "> " + options.at(i) + " <"
            ); // Optional stylistic choice
            // Re-center after string change (account for SFML bounds.position.x
            // internal padding)
            const jo::FloatRect bounds = menuTexts.at(i).getLocalBounds();
            menuTexts.at(i).setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                                        bounds.position.y });
        } else {
            menuTexts.at(i).setFillColor(COLOR_TEXT_NORMAL);
            menuTexts.at(i).setString(options.at(i));
            // Re-center
            const jo::FloatRect bounds = menuTexts.at(i).getLocalBounds();
            menuTexts.at(i).setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                                        bounds.position.y });
        }

        target.draw(menuTexts.at(i));
    }
}

void Menu::renderAboutOverlay(jo::RenderTarget& target) const {
    const jo::View& view = windowManager->getUiView();
    const jo::Vector2f centre = view.getCenter() - jo::Vector2f(100.f, 0.f);

    constexpr unsigned int titleSz = FONT_SIZE_TITLE;
    constexpr unsigned int charSz = 30;
    const float titleSpacing = 40.f;
    const float lineSpacing = 42.f; // manual line spacing
    const float promptSpacing = 50.f;

    // ---------------- TITLE ----------------
    jo::Text titleObj(*font);
    titleObj.setString("ABOUT");
    titleObj.setCharacterSize(titleSz);
    titleObj.setFillColor(COLOR_TEXT_NORMAL);
    titleObj.setOutlineThickness(2.0f);
    titleObj.setOutlineColor(jo::Color::White);
    titleObj.setLetterSpacing(2.f);

    jo::FloatRect tBounds = titleObj.getLocalBounds();
    titleObj.setOrigin({ tBounds.position.x + tBounds.size.x / 2.f, 0.f });

    std::vector<std::string> lines;
    std::stringstream ss(aboutTextContent);
    std::string line;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    jo::Text promptObj(*font);
    promptObj.setString("[Press 'Esc' or 'A' to return]");
    promptObj.setCharacterSize(charSz);
    promptObj.setFillColor(COLOR_TEXT_NORMAL);
    promptObj.setOutlineThickness(2.0f);
    promptObj.setOutlineColor(jo::Color::White);
    promptObj.setLetterSpacing(2.f);

    jo::FloatRect pBounds = promptObj.getLocalBounds();
    promptObj.setOrigin({ pBounds.position.x + pBounds.size.x / 2.f, 0.f });

    // ---------------- CALCULATE TOTAL HEIGHT ----------------
    float textBlockHeight = lines.size() * lineSpacing;

    float totalHeight = tBounds.size.y + titleSpacing + textBlockHeight +
                        promptSpacing + pBounds.size.y;

    float currentY = centre.y - totalHeight / 2.f;

    titleObj.setPosition({ centre.x, currentY });
    target.draw(titleObj);

    currentY += tBounds.size.y + titleSpacing;

    for (const auto& l : lines) {
        jo::Text lineObj(*font);
        lineObj.setString(l);
        lineObj.setCharacterSize(charSz);
        lineObj.setFillColor(COLOR_TEXT_NORMAL);
        lineObj.setOutlineThickness(2.0f);
        lineObj.setOutlineColor(jo::Color::White);
        lineObj.setLetterSpacing(2.f);

        jo::FloatRect bounds = lineObj.getLocalBounds();
        lineObj.setOrigin({ bounds.position.x + bounds.size.x / 2.f, 0.f });
        lineObj.setPosition({ centre.x, currentY });

        target.draw(lineObj);
        currentY += lineSpacing;
    }

    currentY += promptSpacing - lineSpacing; // adjust spacing
    promptObj.setPosition({ centre.x, currentY });
    target.draw(promptObj);
}

/**
 * Show Menu and wait for input of user
 */
void Menu::show(
    RenderEngine& render_engine, TileManager& tileManager,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox, AudioManager& audioManager
) {

    isMenuOpen = true;
    const auto originalView = windowManager->getWindow().getView();

    while (windowManager->getWindow().isOpen() && isMenuOpen) {

        // 1. Update Input
        jo::Vector2f mousePos = getMouseWorldPos();
        mouseSprite.setPosition(mousePos);
        handleHover(mousePos);
        handleInput(windowManager->getWindow());

        // 2. Render
        render(render_engine, tileManager, entities, dialogueBox);
    }

    // Wait for keys to be released to prevent immediate re-triggering
    while (windowManager->getWindow().isOpen() &&
           (jo::Keyboard::isKeyPressed(jo::Keyboard::Key::Escape) ||
            jo::Keyboard::isKeyPressed(jo::Keyboard::Key::P))) {
        windowManager->pollEvents();
    }

    windowManager->getWindow().setView(originalView);
}