#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct PlayerState {
    int health = 100;
    float x = 0.0f;
    float y = 0.0f;
};

struct ItemState {
    std::string id;
    std::uint32_t quantity;
};

struct InventoryState {
    std::vector<ItemState> items;
};

struct GameState {
    PlayerState player;
    InventoryState inventory;
};

class SaveGameManager {
  public:
    SaveGameManager();

    void saveGame(const GameState& state) const;
    [[nodiscard]] GameState loadGame() const;
    [[nodiscard]] bool saveExists() const;

  private:
    [[nodiscard]] std::filesystem::path getSaveDirectory() const;
    [[nodiscard]] std::filesystem::path getSaveFilePath() const;

    std::string m_gameName;
};
