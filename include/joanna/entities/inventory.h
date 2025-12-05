#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>

class TileManager;

struct Item {
    std::string id;   // unique identifier - tile id
    std::string name; // display name
    bool stackable = true;

    Item() = default;
    Item(std::string id, std::string name, bool stackable = true);
};

struct StoredItem {
    Item item;
    std::uint32_t quantity = 0;

    StoredItem() = default;
    StoredItem(Item item, std::uint32_t q);
};

class Inventory {
  public:
    explicit Inventory(std::size_t capacity = 100);

    std::uint32_t addItem(const Item& item, std::uint32_t quantity = 1);
    std::uint32_t removeItem(const std::string& id, std::uint32_t quantity = 1);
    bool hasItem(const std::string& id) const;
    std::uint32_t getQuantity(const std::string& id) const;
    std::size_t slotsUsed() const;
    std::vector<StoredItem> listItems() const;
    void clear();

    void setCapacity(std::size_t cap);
    void draw(sf::RenderTarget& target) const;
    void displayInventory(sf::RenderTarget& target, TileManager& tileManager);

    std::size_t capacity() const;

  private:
    // Internal helpers assume mutex held
    std::size_t slotsUsedUnlocked() const;
    sf::Font font;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, StoredItem> items_;
    std::size_t capacity_;
};
