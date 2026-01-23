#include <gtest/gtest.h>
#include "joanna/entities/player.h"

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

};

TEST_F(PlayerTest, InitialState) {
    Player player(
        "assets/player/main/idle.png",
        "assets/player/main/walk.png",
        "assets/player/main/run.png",
        {0.f, 0.f}
    );

    EXPECT_EQ(player.getHealth(), 20);
    EXPECT_EQ(player.getInventory().capacity(), 20);
    EXPECT_EQ(player.getPosition(), sf::Vector2f(0.f, 0.f));
}

TEST_F(PlayerTest, TakeDamage) {
    Player player(
        "assets/player/main/idle.png",
        "assets/player/main/walk.png",
        "assets/player/main/run.png",
        {0.f, 0.f}
    );

    EXPECT_EQ(player.getHealth(), 20);

    player.takeDamage(5);

    EXPECT_EQ(player.getHealth(), 15);

    player.takeDamage(20);

    EXPECT_EQ(player.getHealth(), 0);
}

TEST_F(PlayerTest, AddItemToInventory) {
    Player player(
        "assets/player/main/idle.png",
        "assets/player/main/walk.png",
        "assets/player/main/run.png",
        {0.f, 0.f}
    );

    Item potion("1330", "Health Potion", true);

    player.addItemToInventory(potion, 3);

    EXPECT_EQ(player.getInventory().getQuantity("1330"), 3);
}

TEST_F(PlayerTest, SetHealth) {
    Player player(
        "assets/player/main/idle.png",
        "assets/player/main/walk.png",
        "assets/player/main/run.png",
        {0.f, 0.f}
    );

    EXPECT_EQ(player.getHealth(), 20);

    player.setHealth(15);

    EXPECT_EQ(player.getHealth(), 15);

    player.setHealth(25);

    EXPECT_EQ(player.getHealth(), 20);
}

