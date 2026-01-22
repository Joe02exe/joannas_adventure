#pragma once

#include "joanna/core/combattypes.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include <SFML/Graphics.hpp>

struct EntityState {
    sf::Vector2f position;
    sf::Vector2f scale;
    Direction facing;
};

class CombatSystem {
  public:
    CombatSystem();

    void startCombat(Player& player, Enemy& enemy);
    void endCombat();
    void update(float dt);
    void render(sf::RenderTarget& target, class TileManager& tileManager);
    void handleInput(sf::Event& event);

    CombatState getState() const {
        return currentState;
    }

    bool battleFinished() const;

  private:
    Player* player = nullptr;
    Enemy* enemy = nullptr;

    enum class TurnPhase { Input, Approaching, Attacking, Returning, EndTurn, Countering };

    EntityState playerState;
    EntityState enemyState;

    const sf::Texture& caveBackground;
    const sf::Texture& beachBackground;
    const sf::Texture* currentBackground = nullptr;

    const sf::Texture attackButtonTexture;
    const sf::Texture attackButtonRollTexture;

    CombatState currentState = CombatState::PlayerTurn;
    TurnPhase phase = TurnPhase::Input;

    float turnTimer = 0.0f;
    float victoryTimer = 0.0f;

    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    Attack currentAttack;

    State pState = State::Idle;
    State eState = State::Idle;

    void updatePlayerTurn(float dt, State& pState, State& eState);
    void updateEnemyTurn(float dt, State& pState, State& eState);
    void processApproach(float dt, Entity* actor, sf::Vector2f target, float speed, float threshold, State& actorState);
    void processReturn(float dt, Entity* actor, sf::Vector2f startPos, float speed, float threshold, State& actorState, Direction moveFacing, Direction endFacing);
    void updateAttackMovement(float dt, Entity* attacker, const sf::Vector2f& targetPos, const Attack& attack);

    template <typename Defender>
    void processAttack(float dt, Entity* attacker, Defender* defender, State& attackerState, State& defenderState, const Attack& attack);

    template <typename Defender>
    void updateAttackTimeline(float dt, Defender* defender, State& defenderState, const Attack& attack);
    
    void e_chooseAttack();

    sf::Texture counterButtonTexture;
    sf::Texture counterButtonGoodTexture;
    sf::Texture counterButtonBadTexture;
    bool canCounter = false;
    bool counterSuccess = false;
    bool damageDealt = false;
    void processCounter(float dt);
};
