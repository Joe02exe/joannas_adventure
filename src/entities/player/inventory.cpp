#include "joanna/entities/inventory.h"

#include <algorithm>
#include <utility>

Item::Item(std::string id, std::string name, const bool stackable)
    : id(std::move(id)), name(std::move(name)), stackable(stackable) {}

StoredItem::StoredItem(Item item, const std::uint32_t q)
    : item(std::move(item)), quantity(q) {}

Inventory::Inventory(const std::size_t capacity) : capacity_(capacity) {}

std::uint32_t
Inventory::addItem(const Item& item, const std::uint32_t quantity) {
    if (quantity == 0) {
        return 0;
    }
    std::lock_guard<std::mutex> lk(mutex_);

    const std::size_t usedSlots = slotsUsedUnlocked();

    const auto it = items_.find(item.id);
    const bool exists = (it != items_.end());

    if (item.stackable) {
        if (!exists) {
            if (usedSlots + 1 > capacity_) {
                return 0;
            }
            items_.emplace(item.id, StoredItem(item, quantity));
            return quantity;
        } else {
            it->second.quantity += quantity;
            return quantity;
        }
    } else {
        const std::size_t freeSlots =
            (capacity_ > usedSlots) ? (capacity_ - usedSlots) : 0;
        const std::uint32_t canAdd = static_cast<std::uint32_t>(
            std::min<std::size_t>(freeSlots, quantity)
        );
        if (canAdd == 0) {
            return 0;
        }
        if (!exists) {
            items_.emplace(item.id, StoredItem(item, canAdd));
        } else {
            it->second.quantity += canAdd;
        }
        return canAdd;
    }
}

std::uint32_t
Inventory::removeItem(const std::string& id, std::uint32_t quantity) {
    if (quantity == 0) {
        return 0;
    }
    std::lock_guard<std::mutex> lk(mutex_);
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return 0;
    }
    const std::uint32_t removed =
        std::min<uint32_t>(it->second.quantity, quantity);
    it->second.quantity -= removed;
    if (it->second.quantity == 0) {
        items_.erase(it);
    }
    return removed;
}

bool Inventory::hasItem(const std::string& id) const {
    std::lock_guard<std::mutex> lk(mutex_);
    return items_.find(id) != items_.end();
}

std::uint32_t Inventory::getQuantity(const std::string& id) const {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = items_.find(id);
    return (it == items_.end()) ? 0u : it->second.quantity;
}

std::size_t Inventory::slotsUsed() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return slotsUsedUnlocked();
}

std::vector<StoredItem> Inventory::listItems() const {
    std::lock_guard<std::mutex> lk(mutex_);
    std::vector<StoredItem> out;
    out.reserve(items_.size());
    for (const auto& [fst, snd] : items_) {
        out.push_back(snd);
    }
    return out;
}

void Inventory::clear() {
    std::lock_guard<std::mutex> lk(mutex_);
    items_.clear();
}

void Inventory::setCapacity(std::size_t cap) {
    std::lock_guard<std::mutex> lk(mutex_);
    capacity_ = cap;
}

std::size_t Inventory::capacity() const {
    return capacity_;
}

std::size_t Inventory::slotsUsedUnlocked() const {
    std::size_t slots = 0;
    for (const auto& kv : items_) {
        if (kv.second.item.stackable) {
            slots += 1;
        } else {
            slots += kv.second.quantity;
}
    }
    return slots;
}

void Inventory::draw(sf::RenderTarget& target) const {
    // Simple text-based inventory display for demonstration
    sf::Font font;
    if (!font.openFromFile("assets/font/minecraft.ttf")) {
        return; // Font loading failed
    }

    auto items = listItems();

    std::string inventoryText;
    if (items.empty()) {
        inventoryText = "(empty)";
    } else {
        inventoryText += "Inventory (" + std::to_string(slotsUsed()) + "/" +
                         std::to_string(capacity()) + ")\n";
        for (const auto& si : items) {
            inventoryText +=
                si.item.name + " x" + std::to_string(si.quantity) + "\n";
        }
    }

    sf::Text text(font);
    text.setString(inventoryText);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color::White);
    text.setPosition({ 0, 0 });
    auto view = target.getView();
    target.setView(target.getDefaultView());
    target.draw(text);
    target.setView(view);
}