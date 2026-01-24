#include <gtest/gtest.h>
#include "joanna/entities/inventory.h"

class InventoryTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

    Item createItem(const std::string& id, bool stackable = true) {
        return Item(id, "Test Item " + id, stackable);
    }
};

TEST_F(InventoryTest, InitialStateIsEmpty) {
    Inventory inv(10);

    EXPECT_EQ(inv.slotsUsed(), 0);
    EXPECT_EQ(inv.capacity(), 10);
    EXPECT_TRUE(inv.listItems().empty());
}

TEST_F(InventoryTest, AddStackableItem) {
    Inventory inv(5);
    Item apple = createItem("101", true);

    uint32_t added = inv.addItem(apple, 5);

    EXPECT_EQ(added, 5);
    EXPECT_EQ(inv.slotsUsed(), 1);
    EXPECT_EQ(inv.getQuantity("101"), 5);

    added = inv.addItem(apple, 3);

    EXPECT_EQ(added, 3);
    EXPECT_EQ(inv.slotsUsed(), 1);
    EXPECT_EQ(inv.getQuantity("101"), 8);
}

TEST_F(InventoryTest, AddUnstackableItem) {
    Inventory inv(5);
    Item sword = createItem("3050", false);

    inv.addItem(sword, 1);
    EXPECT_EQ(inv.slotsUsed(), 1);

    inv.addItem(sword, 1);
    EXPECT_EQ(inv.slotsUsed(), 2);

    EXPECT_EQ(inv.listItems().size(), 2);
}

TEST_F(InventoryTest, CapacityLimitReached) {
    Inventory inv(2);
    Item itemA = createItem("1", false);
    Item itemB = createItem("2", false);
    Item itemC = createItem("3", false);

    EXPECT_EQ(inv.addItem(itemA, 1), 1);
    EXPECT_EQ(inv.addItem(itemB, 1), 1);

    EXPECT_EQ(inv.slotsUsed(), 2);

    EXPECT_EQ(inv.addItem(itemC, 1), 0); // Should return 0 added
    EXPECT_EQ(inv.slotsUsed(), 2);
}

TEST_F(InventoryTest, RemoveItem) {
    Inventory inv(10);
    Item potion = createItem("1", true);

    inv.addItem(potion, 10);

    uint32_t removed = inv.removeItem("1", 4);
    EXPECT_EQ(removed, 4);
    EXPECT_EQ(inv.getQuantity("1"), 6);

    removed = inv.removeItem("1", 10);
    EXPECT_EQ(removed, 6);
    EXPECT_EQ(inv.getQuantity("1"), 0);
    EXPECT_EQ(inv.slotsUsed(), 0);
}

TEST_F(InventoryTest, SelectionNavigation) {
    Inventory inv(10);
    inv.addItem(createItem("1"), 1);
    inv.addItem(createItem("2"), 1);
    inv.addItem(createItem("3"), 1);

    EXPECT_EQ(inv.getSelectedSlotIndex(), 0);
    EXPECT_EQ(inv.getSelectedItemId(), "1");

    inv.selectNext();
    EXPECT_EQ(inv.getSelectedSlotIndex(), 1);
    EXPECT_EQ(inv.getSelectedItemId(), "2");

    inv.selectNext();
    EXPECT_EQ(inv.getSelectedSlotIndex(), 2);
    EXPECT_EQ(inv.getSelectedItemId(), "3");

    // Wrap Around
    inv.selectNext();
    EXPECT_EQ(inv.getSelectedSlotIndex(), 0);
    EXPECT_EQ(inv.getSelectedItemId(), "1");

    inv.selectPrevious();
    EXPECT_EQ(inv.getSelectedSlotIndex(), 2);
    EXPECT_EQ(inv.getSelectedItemId(), "3");
}

TEST_F(InventoryTest, SelectionClampingOnRemoval) {
    Inventory inv(10);
    inv.addItem(createItem("1"), 1);
    inv.addItem(createItem("2"), 1);

    inv.selectSlot(1);
    EXPECT_EQ(inv.getSelectedItemId(), "2");

    inv.removeItem("2", 1);

    EXPECT_EQ(inv.getSelectedSlotIndex(), 0);
    EXPECT_EQ(inv.getSelectedItemId(), "1");

    inv.removeItem("1", 1);

    EXPECT_EQ(inv.slotsUsed(), 0);
    EXPECT_EQ(inv.getSelectedItemId(), "");
}

TEST_F(InventoryTest, SelectionSlotValid) {
    Inventory inv(10);
    inv.addItem(createItem("1"), 1);
    inv.addItem(createItem("2"), 1);

    inv.selectSlot(1);
    EXPECT_EQ(inv.getSelectedItemId(), "2");
    EXPECT_EQ(inv.getSelectedSlotIndex(), 1);
}

TEST_F(InventoryTest, SelectionSlotInvalid) {
    Inventory inv(10);
    inv.addItem(createItem("1"), 1);
    inv.addItem(createItem("2"), 1);

    inv.selectSlot(4);
    EXPECT_EQ(inv.getSelectedItemId(), "1");
    EXPECT_EQ(inv.getSelectedSlotIndex(), 0);
}

TEST_F(InventoryTest, AddInvisibleItem) {
    Inventory inv(10);
    inv.addItem(createItem("1"), 1);
    inv.addItem(createItem("2"), 1);

    EXPECT_EQ(inv.slotsUsed(), 2);

    const auto item = Item("1056", "invisible", true);
    inv.addItem(item, 1);

    EXPECT_EQ(inv.slotsUsed(), 3);
    EXPECT_EQ(inv.listItems().at(2).item.name, "invisible");
}

TEST_F(InventoryTest, CheckOrderWithInvisibleItem) {
    Inventory inv(10);

    const auto item = Item("1056", "invisible", true);
    inv.addItem(item, 1);

    EXPECT_EQ(inv.slotsUsed(), 1);
    EXPECT_EQ(inv.listItems().at(0).item.name, "invisible");

    inv.addItem(createItem("1"), 1);
    EXPECT_EQ(inv.slotsUsed(), 2);
    EXPECT_EQ(inv.listItems().at(0).item.id, "1");
    EXPECT_EQ(inv.listItems().at(1).item.name, "invisible");

    inv.addItem(createItem("2"), 1);
    EXPECT_EQ(inv.listItems().at(0).item.id, "1");
    EXPECT_EQ(inv.listItems().at(1).item.id, "2");
    EXPECT_EQ(inv.listItems().at(2).item.name, "invisible");
}