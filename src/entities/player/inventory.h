#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <mutex>


// Basic item description. Id should be unique per item type.
struct Item {
    std::string id;        // unique identifier, e.g. "potion_health"
    std::string name;      // display name
    bool stackable = true; // if true, multiple units share one inventory slot

    Item() = default;
    Item(std::string id_, std::string name_, bool stackable_ = true);
};

struct StoredItem {
    Item item;
    std::uint32_t quantity = 0;

    StoredItem() = default;
    StoredItem(Item it, std::uint32_t q);
};

class Inventory {
public:
    Inventory(std::size_t capacity = 100);

    std::uint32_t addItem(const Item& item, std::uint32_t quantity = 1);
    std::uint32_t removeItem(const std::string& id, std::uint32_t quantity = 1);
    bool hasItem(const std::string& id) const;
    std::uint32_t getQuantity(const std::string& id) const;
    std::size_t slotsUsed() const;
    std::vector<StoredItem> listItems() const;
    void clear();

    void setCapacity(std::size_t cap);

    std::size_t capacity() const;

private:
    // Internal helpers assume mutex held
    std::size_t slotsUsedUnlocked() const;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, StoredItem> items_;
    std::size_t capacity_;
};
