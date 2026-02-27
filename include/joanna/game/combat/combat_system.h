#pragma once

#include "joanna/core/combattypes.h"
#include "joanna/core/graphics.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"

struct EntityState {
    jo::Vector2f position;
    jo::Vector2f scale;
    Direction facing;
};

class CombatSystem {
  public:
    CombatSystem();

    void startCombat(Player& player, Enemy& enemy);
    void endCombat();
    void update(float dt);
    void render(
        jo::RenderTarget& target, class TileManager& tileManager,
        const jo::Font& font
    );
    void handleInput(jo::Event& event);

    CombatState getState() const {
        return currentState;
    }

    bool battleFinished() const;

  private:
    Player* player = nullptr;
    Enemy* enemy = nullptr;

    enum class TurnPhase : std::uint8_t {
        Input,
        Approaching,
        Attacking,
        Returning,
        EndTurn,
        Countering
    };

    AudioManager audioManager;

    EntityState playerState;
    EntityState enemyState;

    const jo::Texture* caveBackground;
    const jo::Texture* beachBackground;
    const jo::Texture* currentBackground = nullptr;

    const jo::Texture* attackButtonTexture;
    const jo::Texture* attackButtonRollTexture;

    const jo::Texture* counterButtonTexture;
    const jo::Texture* counterButtonGoodTexture;
    const jo::Texture* counterButtonBadTexture;

    CombatState currentState = CombatState::PlayerTurn;
    TurnPhase phase = TurnPhase::Input;

    float turnTimer = 0.0f;
    float victoryTimer = 0.0f;

    jo::Vector2f startPos;
    jo::Vector2f targetPos;
    Attack currentAttack;

    State pState = State::Idle;
    State eState = State::Idle;

    bool canCounter = false;
    bool counterSuccess = false;
    bool damageDealt = false;

    void updatePlayerTurn(float dt, State& pState, State& eState);
    void updateEnemyTurn(float dt, State& pState, State& eState);
    void processApproach(
        float dt, Entity* actor, jo::Vector2f target, float speed,
        float threshold, State& actorState
    );
    void processReturn(
        float dt, Entity* actor, jo::Vector2f startPos, float speed,
        float threshold, State& actorState, Direction moveFacing,
        Direction endFacing
    );
    void updateAttackMovement(
        float dt, Entity* attacker, const jo::Vector2f& targetPos,
        const Attack& attack
    );
    void e_chooseAttack();
    void processCounter(float dt);

    template <typename Defender, typename Attacker>
    void processAttack(
        float dt, Attacker* attacker, Defender* defender, State& attackerState,
        State& defenderState, const Attack& attack
    );

    template <typename Defender, typename Attacker>
    void updateAttackTimeline(
        float dt, Defender* defender, State& defenderState,
        const Attack& attack, Attacker* attacker
    );

    struct DamageIndicator {
        jo::Vector2f position;
        std::string text;
        jo::Color color;
        float lifeTime;
        float maxLifeTime;
        float speed;
    };

    std::vector<DamageIndicator> damageTexts;
    jo::Font damageFont;

    void spawnDamageText(Entity* target, int amount);
};
