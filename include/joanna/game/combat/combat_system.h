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
    void render(sf::RenderTarget& target, TileManager& tileManager, const sf::Font& font);
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

    sf::Texture caveBackground;
    sf::Texture beachBackground;
    sf::Texture* currentBackground = nullptr;

    sf::Texture attackButtonTexture;
    sf::Texture attackButtonRollTexture;

    sf::Texture counterButtonTexture;
    sf::Texture counterButtonGoodTexture;
    sf::Texture counterButtonBadTexture;

    CombatState currentState = CombatState::PlayerTurn;
    TurnPhase phase = TurnPhase::Input;

    float turnTimer = 0.0f;
    float victoryTimer = 0.0f;

    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    Attack currentAttack;

    State pState = State::Idle;
    State eState = State::Idle;

    bool canCounter = false;
    bool counterSuccess = false;
    bool damageDealt = false;

    void updatePlayerTurn(float dt, State& pState, State& eState);
    void updateEnemyTurn(float dt, State& pState, State& eState);
    void processApproach(float dt, Entity* actor, sf::Vector2f target, float speed, float threshold, State& actorState);
    void processReturn(float dt, Entity* actor, sf::Vector2f startPos, float speed, float threshold, State& actorState, Direction moveFacing, Direction endFacing);
    void updateAttackMovement(float dt, Entity* attacker, const sf::Vector2f& targetPos, const Attack& attack);
    void e_chooseAttack();
    void processCounter(float dt);

    template <typename Defender, typename Attacker>
    void processAttack(float dt, Attacker* attacker, Defender* defender, State& attackerState, State& defenderState, const Attack& attack);

    template <typename Defender, typename Attacker>
    void updateAttackTimeline(float dt, Defender* defender, State& defenderState, const Attack& attack, Attacker* attacker);
    struct DamageIndicator {
        sf::Vector2f position;
        std::string text;
        sf::Color color;
        float lifeTime;
        float maxLifeTime;
        float speed;
    };

    std::vector<DamageIndicator> damageTexts;
    sf::Font damageFont;

    void spawnDamageText(Entity* target, int amount);
};
