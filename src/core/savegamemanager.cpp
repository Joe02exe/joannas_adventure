#include "joanna/core/savegamemanager.h"

#include "nlohmann/json.hpp"
#include <cstdlib>
#include <fstream>

using json = nlohmann::json;

SaveGameManager::SaveGameManager() : m_gameName("Joanna") {
    std::filesystem::create_directories(getSaveDirectory());
}

std::filesystem::path SaveGameManager::getSaveDirectory() const {
#ifdef _WIN32
    char* appdata = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata) {
        std::filesystem::path path(appdata);
        free(appdata);
        return path / m_gameName;
    }
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

std::filesystem::path SaveGameManager::getSaveFilePath(const std::string& index
) const {
    return getSaveDirectory() / ("savegame_" + index + ".json");
}

bool SaveGameManager::saveExists(const std::string& index) const {
    return std::filesystem::exists(getSaveFilePath(index));
}

void SaveGameManager::saveGame(const GameState& state, const std::string& index)
    const {
    json j;

    j["player"]["x"] = state.player.x;
    j["player"]["y"] = state.player.y;
    j["player"]["health"] = state.player.health;
    j["player"]["attack"] = state.player.attack;
    j["player"]["defense"] = state.player.defense;
    j["player"]["level"] = state.player.level;
    j["player"]["currentExp"] = state.player.currentExp;
    j["player"]["expToNextLevel"] = state.player.expToNextLevel;

    j["player"]["visitedInteractions"] = json::array();
    for (const auto& interaction : state.player.visitedInteractions) {
        j["player"]["visitedInteractions"].push_back(interaction);
    }

    j["player"]["inventory"] = json::array(); // Initialize as array
    for (const auto& item : state.inventory.items) {
        j["player"]["inventory"].push_back({ { "id", item.id },
                                             { "quantity", item.quantity } });
    }

    for (const auto& item : state.map.items) {
        j["map"]["objects"].push_back({ { "id", item.id },
                                        { "gid", item.gid },
                                        { "x", item.x },
                                        { "y", item.y } });
    }

    std::ofstream file(getSaveFilePath(index));
    if (file) {
        file << j.dump(4); // pretty print with indent 4
    }
}

GameState SaveGameManager::loadGame(const std::string& index) const {

    std::ifstream file(getSaveFilePath(index));
    // Handle the case where the file doesn't exist yet
    if (!file.is_open()) {
        // Return a default/empty state or throw an exception
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error&) {
        return {};
    }

    GameState state;

    state.player.x = j["player"].value("x", 0.0f);
    state.player.y = j["player"].value("y", 0.0f);
    state.player.health = j["player"].value("health", 100);
    state.player.attack = j["player"].value("attack", 0);
    state.player.defense = j["player"].value("defense", 0);
    state.player.level = j["player"].value("level", 1);
    state.player.currentExp = j["player"].value("currentExp", 0);
    state.player.expToNextLevel = j["player"].value("expToNextLevel", 0);

    if (j["player"].contains("visitedInteractions")) {
        for (const auto& iteraction : j["player"]["visitedInteractions"]) {
            state.player.visitedInteractions.emplace(iteraction);
        }
    }

    if (j["player"].contains("inventory")) {
        for (const auto& itemJson : j["player"]["inventory"]) {
            ItemState newItem;
            newItem.id = itemJson["id"];
            newItem.quantity = itemJson["quantity"];
            state.inventory.items.push_back(newItem);
        }
    }
    if (j["map"].contains("objects")) {
        for (const auto& objJson : j["map"]["objects"]) {
            ObjectState newObj{};
            newObj.id = objJson["id"];
            newObj.gid = objJson["gid"];
            newObj.x = objJson["x"];
            newObj.y = objJson["y"];
            state.map.items.push_back(newObj);
        }
    }
    return state;
}
