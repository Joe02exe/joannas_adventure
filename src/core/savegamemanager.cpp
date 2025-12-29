#include "joanna/core/savegamemanager.h"

#include "joanna/utils/logger.h"

#include "nlohmann/json.hpp"
#include <cstdlib>
#include <fstream>

using json = nlohmann::json;

SaveGameManager::SaveGameManager() : m_gameName("Joanna") {
    std::filesystem::create_directories(getSaveDirectory());
}

std::filesystem::path SaveGameManager::getSaveDirectory() const {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata)
        return std::filesystem::path(appdata) / m_gameName;
#endif
#ifdef __APPLE__
    const char* home = std::getenv("HOME");
    if (home)
        return std::filesystem::path(home) / "Library" / "Application Support" /
               m_gameName;
#endif
#ifdef __linux__
    const char* home = std::getenv("HOME");
    if (home)
        return std::filesystem::path(home) / ".local" / "share" / m_gameName;
#endif
    return std::filesystem::current_path() / m_gameName; // fallback
}

std::filesystem::path SaveGameManager::getSaveFilePath(std::string index) const {
    return getSaveDirectory() / ("savegame_" + index + ".json");
}

bool SaveGameManager::saveExists(std::string index) const {
    return std::filesystem::exists(getSaveFilePath(index));
}

void SaveGameManager::saveGame(const GameState& state, std::string index) const {
    json j;

    j["player"]["x"] = state.player.x;
    j["player"]["y"] = state.player.y;
    j["player"]["health"] = state.player.health;

    j["player"]["inventory"] = json::array(); // Initialize as array
    for (const auto& item : state.inventory.items) {
        j["player"]["inventory"].push_back({ { "id", item.id },
                                             { "quantity", item.quantity } });
    }

    std::ofstream file(getSaveFilePath(index));
    if (file) {
        file << j.dump(4); // pretty print with indent 4
    }
}

GameState SaveGameManager::loadGame() const {
    // 1. Open the file
    // TODO
    std::ifstream file(getSaveFilePath("1"));

    // Handle the case where the file doesn't exist yet
    if (!file.is_open()) {
        // Return a default/empty state or throw an exception
        return {};
    }

    // 2. Parse the JSON
    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        // Handle corrupted save file
        return {};
    }

    GameState state;

    // 3. Load Player Data
    // using .value() allows you to provide a default fallback if the key is
    // missing
    state.player.x = j["player"].value("x", 0.0f);
    state.player.y = j["player"].value("y", 0.0f);
    state.player.health = j["player"].value("health", 100);

    if (j["player"].contains("inventory")) {
        for (const auto& itemJson : j["player"]["inventory"]) {
            ItemState newItem;
            newItem.id = itemJson["id"];
            newItem.quantity = itemJson["quantity"];
            state.inventory.items.push_back(newItem);
        }
    }
    return state;
}
