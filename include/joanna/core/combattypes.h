#pragma once

#include "joanna/entities/entityutils.h"
#include <string>

enum class CombatState { PlayerTurn, EnemyTurn, Victory, Defeat };

struct Attack {
    std::string name;
    int damage;
    State animationState;

    // Timing
    float impactTime = 0.5f;
    float endTime = 1.0f;

    // Movement (optional)
    float moveSpeed = 0.f;
    float targetOffset = 0.f;
    float moveThreshold = 5.f;
};
