#include <gtest/gtest.h>
#include "joanna/entities/enemy.h"

class EnemyTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(EnemyTest, InitialState) {
    // Assuming assets are available in the build directory
    Enemy goblin({100.f, 100.f}, Enemy::EnemyType::Goblin);

    EXPECT_EQ(goblin.getType(), Enemy::EnemyType::Goblin);
    EXPECT_EQ(goblin.getHealth(), 10); // Default health seems to be 10
    EXPECT_FALSE(goblin.isDead());
    
    Enemy skeleton({200.f, 200.f}, Enemy::EnemyType::Skeleton);
    EXPECT_EQ(skeleton.getType(), Enemy::EnemyType::Skeleton);
}

TEST_F(EnemyTest, TakeDamage) {
    Enemy enemy({0.f, 0.f}, Enemy::EnemyType::Goblin);
    EXPECT_EQ(enemy.getHealth(), 10);

    enemy.takeDamage(3);
    EXPECT_EQ(enemy.getHealth(), 7);

    enemy.takeDamage(10);
    EXPECT_EQ(enemy.getHealth(), 0);
    EXPECT_TRUE(enemy.isDead());
}

TEST_F(EnemyTest, ResetHealth) {
    Enemy enemy({0.f, 0.f}, Enemy::EnemyType::Goblin);
    enemy.takeDamage(5);
    EXPECT_EQ(enemy.getHealth(), 5);

    enemy.resetHealth();
    EXPECT_EQ(enemy.getHealth(), 10);
}

TEST_F(EnemyTest, CombatTriggerLogic) {
    // Test the simple distance logic without needing TileManager or Window
    
    EXPECT_TRUE(Enemy::shouldTriggerCombat(5.0f));
    EXPECT_TRUE(Enemy::shouldTriggerCombat(9.9f));
    
    EXPECT_FALSE(Enemy::shouldTriggerCombat(10.0f));
    EXPECT_FALSE(Enemy::shouldTriggerCombat(20.0f));
    EXPECT_FALSE(Enemy::shouldTriggerCombat(100.0f));
}
