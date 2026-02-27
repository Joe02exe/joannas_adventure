#pragma once

#include "joanna/core/gamestatus.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include "joanna/game/combat/combat_system.h"
#include "joanna/core/graphics.h"

class Controller;
class DebugUI {
  public:
    static void init(jo::RenderWindow& window);
    void processEvent(const jo::Window& window, const jo::Event& event) const;
    void update(
        float dt, jo::RenderWindow& window, Player& player,
        GameStatus& gameStatus, CombatSystem& combatSystem, Enemy& testEnemy,
        Controller& controller
    ) const;
    void render(jo::RenderWindow& window) const;
    static void shutdown();

    void toggle() { enabled = !enabled; }

  private:
    bool enabled = true;
};
