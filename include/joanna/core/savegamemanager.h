#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

struct PlayerState {
    int health = 100;
    float x = 0.0f;
    float y = 0.0f;
};

struct GameState {
    PlayerState player;
    int score = 0;
};

class SaveGameManager {
  public:
    SaveGameManager(const std::string gameName);

    void saveGame(const GameState& state);
    [[nodiscard]] GameState loadGame() const;
    [[nodiscard]] bool saveExists() const;

  private:
    [[nodiscard]] std::filesystem::path getSaveDirectory() const;
    [[nodiscard]] std::filesystem::path getSaveFilePath() const;

    std::string m_gameName;
};
