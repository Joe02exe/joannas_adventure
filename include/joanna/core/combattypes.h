#pragma once

#include "joanna/entities/entityutils.h"
#include <string>

enum class CombatState { PlayerTurn, EnemyTurn, Victory, Defeat };

struct Attack {
    std::string name;
    int damage;
    State animationState;
};
