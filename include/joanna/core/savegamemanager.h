#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

struct PlayerState {
    int health = 100;
    float x = 0.0f;
    float y = 0.0f;
    int attack = 0;
    int defense = 0;
    int level = 1;
    int currentExp = 0;
    int expToNextLevel = 0;
    std::unordered_set<std::string> visitedInteractions;
};

struct ItemState {
    std::string id;
    std::uint32_t quantity;
};

struct InventoryState {
    std::vector<ItemState> items;
};

struct ObjectState {
    std::uint32_t id;
    std::uint32_t gid;
    int x;
    int y;
};

struct MapState {
    std::vector<ObjectState> items;
};

struct GameState {
    PlayerState player;
    InventoryState inventory;
    MapState map;
};

class SaveGameManager {
  public:
    SaveGameManager();

    void saveGame(const GameState& state, const std::string& index) const;
    [[nodiscard]] GameState loadGame(const std::string& index) const;
    [[nodiscard]] bool saveExists(const std::string& index) const;

  private:
    [[nodiscard]] std::filesystem::path getSaveDirectory() const;
    [[nodiscard]] std::filesystem::path getSaveFilePath(const std::string& index
    ) const;

    std::string m_gameName;
};
