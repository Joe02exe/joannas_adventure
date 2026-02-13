#include "joanna/core/savegamemanager.h"

#include "joanna/utils/logger.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

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
    try {
        const json j = state;

        if (std::ofstream file(getSaveFilePath(index)); file.is_open()) {
            file << j.dump(4);
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to save game to slot {}: {}", index, e.what());
    }
}

GameState SaveGameManager::loadGame(const std::string& index) const {
    const std::filesystem::path path = getSaveFilePath(index);

    if (!std::filesystem::exists(path)) {
        return {};
    }

    try {
        std::ifstream file(path);
        json j;
        file >> j;

        return j.get<GameState>();

    } catch (const std::exception& e) {
        return {};
    }
}

std::string SaveGameManager::getSaveInfo(const std::string& index) const {
    const auto path = getSaveFilePath(index);

    if (!std::filesystem::exists(path)) {
        return "Empty";
    }

    try {
        // last update time of the file
        const auto ftime = std::filesystem::last_write_time(path);

        const auto systemTime =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() +
                std::chrono::system_clock::now()
            );
        const std::time_t cftime = std::chrono::system_clock::to_time_t(systemTime);

        // Format the time (e.g., "YYYY-MM-DD HH:MM")
        std::stringstream ss;
        ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M");
        return ss.str();
    } catch (...) {
        return "Unknown"; // Fallback if permission/conversion fails
    }
}