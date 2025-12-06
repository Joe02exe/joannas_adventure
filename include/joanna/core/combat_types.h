#pragma once

#include <string>

enum class CombatState { PlayerTurn, EnemyTurn, Victory, Defeat };

struct Attack {
    std::string name;
    int damage;
    // visual properties
};
