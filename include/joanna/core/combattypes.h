#pragma once

#include "joanna/entities/entityutils.h"
#include <string>

#define COMBAT_TRIGGERED 1
#define COMBAT_IDLE 0

enum class CombatState { PlayerTurn, EnemyTurn, Victory, Defeat };

struct Attack {
    std::string name;       // name of the attack for debugging
    int damage;             // damage dealt to the target
    State animationState;   // animation state to play

    // timing
    float impactTime = 0.5f; // time when the attack hits/deals damage
    float endTime = 1.0f;    // total duration of the attack animation

    // movement (optional)
    float moveSpeed = 0.f;     // speed of movement during attack (pixels/sec)
    float targetOffset = 0.f;  // offset from target position to stop at
    float moveThreshold = 5.f; // minimum distance to trigger movement

    // counter
    bool counterable = false;       // can this attack be countered?
    float counterWindowStart = 0.f; // start time of the counter window
    float counterWindowEnd = 0.f;   // end time of the counter window

    float approachOffset = 0.f; // offset for the initial approach position
};
