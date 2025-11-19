#include "joanna/core/savegamemanager.h"
#include <cstdlib>
#include <fstream>

using json = nlohmann::json;

SaveGameManager::SaveGameManager(const std::string gameName)
    : m_gameName(std::move(gameName)) {
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

std::filesystem::path SaveGameManager::getSaveFilePath() const {
    return getSaveDirectory() / "savegame.json";
}

bool SaveGameManager::saveExists() const {
    return std::filesystem::exists(getSaveFilePath());
}

void SaveGameManager::saveGame(const GameState& state) {
    json j;
    j["player"]["health"] = state.player.health;
    j["player"]["x"] = state.player.x;
    j["player"]["y"] = state.player.y;
    j["score"] = state.score;

    std::ofstream file(getSaveFilePath());
    if (file) {
        file << j.dump(4); // pretty print with indent 4
    }
}

GameState SaveGameManager::loadGame() const {
    GameState state;
    std::ifstream file(getSaveFilePath());
    if (file) {
        json j;
        file >> j;

        if (j.contains("player")) {
            state.player.health = j["player"].value("health", 100);
            state.player.x = j["player"].value("x", 0.0f);
            state.player.y = j["player"].value("y", 0.0f);
        }
        state.score = j.value("score", 0);
    }
    return state;
}
