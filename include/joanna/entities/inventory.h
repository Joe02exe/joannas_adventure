#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>

#include "joanna/core/savegamemanager.h"

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

    bool hasItemByName(const std::string& name) const;

    std::uint32_t getQuantity(const std::string& id) const;

    std::size_t slotsUsed() const;

    const std::vector<StoredItem>& listItems() const;

    std::string getSelectedItemId() const;

    void loadState(InventoryState& state);

    void clear();

    void setCapacity(std::size_t cap);

    void draw(sf::RenderTarget& target) const;

    void selectNext();

    void selectPrevious();

    void selectSlot(std::size_t index);

    int getSelectedSlotIndex() const;

    void drawSlot(
        sf::RenderTarget& target, float slotSize, float padding,
        sf::Vector2f startPos, std::size_t i, std::size_t col, std::size_t row,
        sf::Vector2f& slotPos, bool isSelected
    ) const;

    void drawItemName(
        sf::RenderTarget& target, float slotSize, sf::Vector2f slotPos,
        const StoredItem& st
    ) const;

    void drawItemQuantity(
        sf::RenderTarget& target, float slotSize, sf::Vector2f slotPos,
        const StoredItem& st
    ) const;

    void drawItemSprite(
        sf::RenderTarget& target, TileManager& tileManager, float slotSize,
        sf::Vector2f slotPos, const StoredItem& st
    ) const;

    void drawItems(
        sf::RenderTarget& target, TileManager& tileManager,
        std::vector<StoredItem> vec, std::size_t columns, float slotSize,
        float padding, std::size_t itemCount, sf::Vector2f startPos
    ) const;

    void
    displayInventory(sf::RenderTarget& target, TileManager& tileManager) const;

    std::size_t capacity() const;

    std::unordered_map<int, std::string> mapGidToName() const {
        return idToString;
    }

    bool isItemInvisible(int) const;

  private:
    void checkInventoryInvisibleBounds();

    std::size_t selectedSlotIndex = 0;

    sf::Font font;
    std::vector<StoredItem> items_;
    std::size_t capacity_;
    std::vector<int> invisibleItemIds = { 3056, 3055 };
    std::unordered_map<int, std::string> idToString = {
        { 691, "carrot" },
        { 3050, "sword" },
        { 3056, "piratToken" },    // invisible
        { 3055, "counterAttack" }, // invisible
        { 627, "key" },
        { 628, "bone" },
        { 629, "shield" },
        { 630, "grade" },
        { 703, "mushroom" },
        { 1330, "heal" },
        { 3113, "pickaxe" },
    };
};
