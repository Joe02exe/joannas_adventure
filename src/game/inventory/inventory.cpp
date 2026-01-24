#include "joanna/entities/inventory.h"

#include "joanna/utils/resourcemanager.h"
#include "joanna/world/tilemanager.h"

#include <algorithm>
#include <utility>

#include "joanna/core/savegamemanager.h"

Item::Item(std::string id, std::string name, const bool stackable)
    : id(std::move(id)), name(std::move(name)), stackable(stackable) {}

StoredItem::StoredItem(Item item, const std::uint32_t q)
    : item(std::move(item)), quantity(q) {}

Inventory::Inventory(const std::size_t capacity) : capacity_(capacity) {
    font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
}

std::uint32_t
Inventory::addItem(const Item& item, const std::uint32_t quantity) {
    if (quantity == 0) {
        return 0;
    }
    const std::size_t usedSlots = items_.size();

    if (item.stackable) {
        auto it = std::find_if(
            items_.begin(), items_.end(),
            [&](const StoredItem& si) { return si.item.id == item.id; }
        );

        if (it != items_.end()) {
            it->quantity += quantity;
            return quantity;
        }
    }

    if (usedSlots + 1 > capacity_) {
        return 0;
    }

    auto it =
        std::find_if(items_.begin(), items_.end(), [](const StoredItem& item) {
            return item.item.name == "invisible";
        });

    if (it != items_.end()) {
        items_.insert(it, StoredItem(item, quantity));
        return quantity;
    }
    items_.emplace_back(item, quantity);
    return quantity;
}

std::uint32_t
Inventory::removeItem(const std::string& id, std::uint32_t quantity) {
    if (quantity == 0) {
        return 0;
    }

    auto it =
        std::find_if(items_.begin(), items_.end(), [&](const StoredItem& si) {
            return si.item.id == id;
        });

    if (it == items_.end()) {
        return 0;
    }

    const std::uint32_t removed = std::min<uint32_t>(it->quantity, quantity);

    it->quantity -= removed;

    if (it->quantity == 0) {
        items_.erase(it);

        // Safety check: if we erased the slot we currently have selected,
        // clamp selection to valid range
        if (selectedSlotIndex >= items_.size() && !items_.empty()) {
            selectedSlotIndex = items_.size() - 1;
        } else if (items_.empty()) {
            selectedSlotIndex = 0;
        }
    }

    return removed;
}

bool Inventory::hasItem(const std::string& id) const {
    auto it =
        std::find_if(items_.begin(), items_.end(), [&](const StoredItem& si) {
            return si.item.id == id;
        });
    return it != items_.end();
}

std::uint32_t Inventory::getQuantity(const std::string& id) const {
    auto it =
        std::find_if(items_.begin(), items_.end(), [&](const StoredItem& si) {
            return si.item.id == id;
        });

    return (it == items_.end()) ? 0u : it->quantity;
}

std::size_t Inventory::slotsUsed() const {
    return items_.size();
}

const std::vector<StoredItem>& Inventory::listItems() const {
    return items_;
}

void Inventory::loadState(InventoryState& state) {
    items_.clear();
    items_.reserve(state.items.size()); // Optimization
    for (auto& itemState : state.items) {
        // Warning: map lookups like idToString[...] can throw if key missing
        // safely handled via .at() or ensuring data integrity
        Item item(itemState.id, idToString.at(std::stoi(itemState.id)));
        items_.emplace_back(item, itemState.quantity);
    }
}

void Inventory::clear() {
    items_.clear();
    selectedSlotIndex = 0;
}

void Inventory::setCapacity(std::size_t cap) {
    capacity_ = cap;
}

std::size_t Inventory::capacity() const {
    return capacity_;
}

void Inventory::draw(sf::RenderTarget& target) const {
    // Simple text-based inventory display for demonstration

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

void Inventory::drawSlot(
    sf::RenderTarget& target, const float slotSize, const float padding,
    const sf::Vector2f startPos, std::size_t i, std::size_t col,
    std::size_t row, sf::Vector2f& slotPos, bool isSelected
) const {
    slotPos = { startPos.x + static_cast<float>(col) * (slotSize + padding),
                startPos.y + static_cast<float>(row) * (slotSize + padding) };

    // Slot rectangle
    sf::RectangleShape slot({ slotSize, slotSize });
    slot.setPosition(slotPos);
    if (isSelected) {
        slot.setFillColor(sf::Color(70, 70, 70, 220));
        slot.setOutlineThickness(2.f);
        slot.setOutlineColor(sf::Color(200, 200, 50, 220));
    } else {
        slot.setFillColor(sf::Color(40, 40, 40, 200));
        slot.setOutlineThickness(1.f);
        slot.setOutlineColor(sf::Color(120, 120, 120, 200));
    }
    target.draw(slot);
}

void Inventory::drawItemName(
    sf::RenderTarget& target, const float slotSize, sf::Vector2f slotPos,
    StoredItem& st
) const {
    sf::Text name(font);
    name.setString(st.item.name);
    name.setCharacterSize(14);
    name.setFillColor(sf::Color::White);

    sf::FloatRect tb = name.getLocalBounds();
    name.setOrigin({ tb.position.x + tb.size.x / 2.f, tb.position.y });
    name.setPosition({ slotPos.x + slotSize / 2.f, slotPos.y + 4.f });

    target.draw(name);
}

void Inventory::drawItemQuantity(
    sf::RenderTarget& target, const float slotSize, sf::Vector2f slotPos,
    StoredItem& st
) const {
    if (st.quantity > 1) {
        sf::Text qText(font);
        qText.setString(std::to_string(st.quantity));
        qText.setCharacterSize(16);
        sf::Text shadow = qText;

        qText.setFillColor(sf::Color::White);
        shadow.setFillColor(sf::Color(0, 0, 0, 200));

        sf::FloatRect qb = qText.getLocalBounds();
        sf::Vector2f pos({ slotPos.x + slotSize - qb.size.y - 6.f -
                               qb.position.y,
                           slotPos.y + slotSize - qb.size.x - 6.f });

        shadow.setPosition(pos + sf::Vector2f(1.f, 1.f));
        qText.setPosition(pos);

        target.draw(shadow);
        target.draw(qText);
    }
}

void Inventory::drawItemSprite(
    sf::RenderTarget& target, TileManager& tileManager, const float slotSize,
    sf::Vector2f slotPos, StoredItem& st
) const {
    sf::Sprite icon = tileManager.getTextureById(std::stoi(st.item.id));

    auto bounds = icon.getLocalBounds();
    float targetSize = slotSize * 0.5f;
    float scale = targetSize / bounds.size.x; // assuming square icon

    icon.setScale({ scale, scale });
    icon.setPosition({ slotPos.x + (slotSize - (scale)) / 2.f,
                       slotPos.y + (slotSize - (scale)) / 2.f + 8.f });
    target.draw(icon);
}

void Inventory::drawItems(
    sf::RenderTarget& target, TileManager& tileManager,
    std::vector<StoredItem> vec, const std::size_t columns,
    const float slotSize, const float padding, std::size_t itemCount,
    const sf::Vector2f startPos
) const {
    for (std::size_t i = 0; i < itemCount; ++i) {
        const std::size_t col = i % columns;
        const std::size_t row = i / columns;

        if (vec.at(i).item.name == "invisible") {
            continue;
        }

        sf::Vector2f slotPos;
        StoredItem& st = vec[i];
        bool isSelected = (i == this->selectedSlotIndex);

        drawSlot(
            target, slotSize, padding, startPos, i, col, row, slotPos,
            isSelected
        );
        drawItemName(target, slotSize, slotPos, st);
        drawItemQuantity(target, slotSize, slotPos, st);
        drawItemSprite(target, tileManager, slotSize, slotPos, st);
    }
}

void Inventory::displayInventory(
    sf::RenderTarget& target, TileManager& tileManager
) const {

    // Convert unordered_map to vector for display order
    const auto& vec = items_;

    const std::size_t columns = 8;
    const float slotSize = 64.f;
    const float padding = 6.f;
    // Outer background
    std::size_t itemCount = vec.size();
    std::size_t rows = (itemCount + columns) / columns;
    float totalWidth =
        columns * slotSize + padding * (columns - 1) + 2 * padding;
    float totalHeight = static_cast<float>(rows) * slotSize +
                        padding * (static_cast<float>(rows) - 1) + 2 * padding;

    const auto center = target.getView().getCenter();
    const sf::Vector2f startPos(center.x - totalWidth / 2.f, 360.f);

    sf::RectangleShape bg({ totalWidth, totalHeight });
    bg.setPosition(startPos - sf::Vector2f(padding, padding));
    bg.setFillColor(sf::Color(25, 25, 25, 200));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(180, 180, 180, 120));
    target.draw(bg);

    drawItems(
        target, tileManager, vec, columns, slotSize, padding, itemCount,
        startPos
    );
}

void Inventory::selectNext() {
    if (items_.empty())
        return;
    selectedSlotIndex++;
    checkInventoryInvisibleBounds();
}

void Inventory::selectSlot(std::size_t index) {
    if (items_.empty())
        return;
    selectedSlotIndex = index;
    checkInventoryInvisibleBounds();
}

void Inventory::selectPrevious() {
    if (items_.empty())
        return;
    if (selectedSlotIndex == 0) {
        const auto it = std::find_if(
            items_.begin(), items_.end(),
            [](const StoredItem& item) { return item.item.name == "invisible"; }
        );
        selectedSlotIndex = std::distance(items_.begin(), it) - 1;
    } else {
        selectedSlotIndex--;
    }
}

int Inventory::getSelectedSlotIndex() const {
    return static_cast<int>(selectedSlotIndex);
}

std::string Inventory::getSelectedItemId() const {
    try {
        auto item = listItems().at(getSelectedSlotIndex());
        return item.item.id;
    } catch (const std::out_of_range& e) {
        return "";
    }
}

void Inventory::checkInventoryInvisibleBounds() {
    const auto it =
        std::find_if(items_.begin(), items_.end(), [](const StoredItem& item) {
            return item.item.name == "invisible";
        });
    if (it != items_.end()) {
        if (const std::size_t index = std::distance(items_.begin(), it);
            selectedSlotIndex >= index) {
            if (selectedSlotIndex >= index) {
                selectedSlotIndex = 0;
            }
        }
    }
}