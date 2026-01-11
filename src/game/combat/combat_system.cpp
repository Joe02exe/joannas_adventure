#include "joanna/systems/combat_system.h"
#include "joanna/utils/resourcemanager.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

CombatSystem::CombatSystem()
    : backgroundTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/images/combat_background_cave.png"
      )),
      attackButtonTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/attack.png"
      )) {}

void CombatSystem::startCombat(Player& p, Enemy& e) {
    player = &p;
    enemy = &e;
    std::cout << "Combat Started!\n";

    // Save state
    playerState.position = player->getPosition();
    playerState.scale = player->getScale();
    playerState.facing = player->getFacing();

    enemyState.position = enemy->getPosition();
    enemyState.scale = enemy->getScale();
    enemyState.facing = enemy->getFacing();

    // Assuming screen size 900x900 and player on left, enemy on right
    sf::Vector2f pPos(-100.f, 300);
    sf::Vector2f ePos(430.f, 300);

    player->setPosition(pPos);
    enemy->setPosition(ePos);
    player->setFacing(Direction::Right);
    enemy->setFacing(Direction::Left);
    player->setScale({ 7.f, 7.f });
    enemy->setScale({ 7.f, 7.f });

    phase = TurnPhase::Input;
    currentState = CombatState::PlayerTurn;
    eState = State::Idle;
    pState = State::Idle;
}

void CombatSystem::endCombat() {
    player->setPosition(playerState.position);
    player->setScale(playerState.scale);
    player->setFacing(playerState.facing);

    enemy->setPosition(enemyState.position);
    enemy->setScale(enemyState.scale);
    enemy->setFacing(enemyState.facing);
    enemy->takeDamage(-100);
}

void CombatSystem::update(float dt) {

    dt -= 0.007f; // just for visual purpose
    static AudioManager dummyAudio;

    if (currentState == CombatState::PlayerTurn) {
        updatePlayerTurn(dt, pState, eState);
    } else if (currentState == CombatState::EnemyTurn) {
        updateEnemyTurn(dt, pState, eState);
    }

    player->update(dt, pState, player->getFacing() == Direction::Left, dummyAudio);
    enemy->update(dt, eState);
}

void CombatSystem::processApproach(float dt, Entity* actor, sf::Vector2f target, float speed, float threshold, State& actorState) {
    actorState = State::Running;
    sf::Vector2f dir = target - actor->getPosition();
    
    // Move if distance is greater than threshold (considering direction)
    bool shouldMove = (speed > 0 && dir.x > threshold) || (speed < 0 && dir.x < -threshold);
    
    if (shouldMove) {
        actor->setPosition(actor->getPosition() + sf::Vector2f(speed * dt, 0.f));
    } else {
        phase = TurnPhase::Attacking;
        turnTimer = 0.0f;
    }
}

void CombatSystem::processReturn(float dt, Entity* actor, sf::Vector2f startPos, float speed, float threshold, State& actorState, Direction moveFacing, Direction endFacing) {
    actor->setFacing(moveFacing);
    actorState = State::Running;
    sf::Vector2f dir = startPos - actor->getPosition();
    
    bool shouldMove = (speed > 0 && dir.x > threshold) || (speed < 0 && dir.x < -threshold);
    
    if (shouldMove) {
        actor->setPosition(actor->getPosition() + sf::Vector2f(speed * dt, 0.f));
    } else {
        actor->setPosition(startPos);
        actor->setFacing(endFacing);
        phase = TurnPhase::EndTurn;
        actorState = State::Idle;
    }
}

template <typename Defender>
void CombatSystem::processAttack(float dt, Entity* attacker, Defender* defender, State& attackerState, State& defenderState, const Attack& attack, float& timer, TurnPhase& nextPhase, float impactTime, float endTime, float moveSpeed, float targetOffset, float moveThreshold) {
    attackerState = attack.animationState;

    if (moveSpeed != 0.f) {
        sf::Vector2f target = defender->getPosition();
        target.x += targetOffset;
        sf::Vector2f dir = target - attacker->getPosition();
        
        bool shouldMove = (moveSpeed > 0 && dir.x > moveThreshold) || (moveSpeed < 0 && dir.x < -moveThreshold);
        
        if (shouldMove) {
            attacker->setPosition(attacker->getPosition() + sf::Vector2f(moveSpeed * dt, 0.f));
        }
    }

    if (timer < impactTime) {
        defenderState = State::Idle;
    } else if (timer < endTime) {
        defenderState = State::Hurt;
    } else {
        defenderState = State::Idle;
        nextPhase = TurnPhase::Returning;
        timer = 0.0f;
        defender->takeDamage(attack.damage);
    }
    timer += dt;
}

// Explicit instantiations
template void CombatSystem::processAttack<Player>(float, Entity*, Player*, State&, State&, const Attack&, float&, TurnPhase&, float, float, float, float, float);
template void CombatSystem::processAttack<Enemy>(float, Entity*, Enemy*, State&, State&, const Attack&, float&, TurnPhase&, float, float, float, float, float);

void CombatSystem::updatePlayerTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Approaching) {
        processApproach(dt, player, targetPos, 500.f, 10.f, pState);
    } else if (phase == TurnPhase::Attacking) {
        if (currentAttack.name == "Attack") {
             processAttack(dt, player, enemy, pState, eState, currentAttack, turnTimer, phase, 0.3f, 0.8f);
        } else if (currentAttack.name == "Roll") {
             processAttack(dt, player, enemy, pState, eState, currentAttack, turnTimer, phase, 0.2f, 0.85f, 800.f, -90.f, 5.f);
        }
    } else if (phase == TurnPhase::Returning) {
        processReturn(dt, player, startPos, -500.f, -10.f, pState, Direction::Left, Direction::Right);
    } else if (phase == TurnPhase::EndTurn) {
        //TODO checkEndTurn
        if (enemy->getHealth() <= 0) {
            eState = State::Dead;
            currentState = CombatState::Victory;
            std::cout << "Victory!\n";
        } else {
            currentState = CombatState::EnemyTurn;
            phase = TurnPhase::Input;
            turnTimer = 0.0f;
        }
    }
}

void CombatSystem::e_chooseAttack(){
    startPos = enemy->getPosition();
        targetPos = player->getPosition();

        if (rand() % 2 == 0) {
            currentAttack = { "Mining", 5, State::Mining };
            targetPos.x += 130.f;
        } else {
            currentAttack = { "Roll", 8, State::Roll };
            targetPos.x += 280.f;
        }
        phase = TurnPhase::Approaching;
}


void CombatSystem::updateEnemyTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Input) {
        e_chooseAttack();
    } else if (phase == TurnPhase::Approaching) {
        processApproach(dt, enemy, targetPos, -500.f, -10.f, eState);
    } else if (phase == TurnPhase::Attacking) {
        if (currentAttack.name == "Mining") {
             processAttack(dt, enemy, player, eState, pState, currentAttack, turnTimer, phase, 0.4f, 0.9f);
        } else if (currentAttack.name == "Roll") {
             processAttack(dt, enemy, player, eState, pState, currentAttack, turnTimer, phase, 0.2f, 0.8f, -800.f, 85.f, -5.f);
        }
    } else if (phase == TurnPhase::Returning) {
        processReturn(dt, enemy, startPos, 500.f, 10.f, eState, Direction::Right, Direction::Left);
    } else if (phase == TurnPhase::EndTurn) {
        //TODO this can be generalized into one function
        if (player->getHealth() <= 0) {
            pState = State::Dead;
            currentState = CombatState::Defeat;
            std::cout << "Defeat!\n";
        } else {
            currentState = CombatState::PlayerTurn;
            phase = TurnPhase::Input;
        }
    }
}

void CombatSystem::render(sf::RenderTarget& target) {

    // currently set statically... because viewport is set to 900x900
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale({ 900.f / 1400.f, 900.f / 1400.f });
    backgroundSprite.setPosition({ 0.f, 0.f });
    target.draw(backgroundSprite);

    // either render player or enemy based on current combat state
    if (currentState == CombatState::EnemyTurn) {
        player->draw(target);
        enemy->draw(target);
    } else {
        enemy->draw(target);
        player->draw(target);
    }

    if (currentState == CombatState::PlayerTurn && phase == TurnPhase::Input) {
        sf::Sprite attackButtonSprite(attackButtonTexture);
        attackButtonSprite.setScale({ 3, 3 });
        attackButtonSprite.setPosition({ 95.f, 300.f });
        target.draw(attackButtonSprite);
    }
}

void CombatSystem::handleInput(sf::Event& event) {
    if (currentState == CombatState::PlayerTurn && phase == TurnPhase::Input) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            startPos = player->getPosition();
            targetPos = enemy->getPosition();
            if (keyEvent->code == sf::Keyboard::Key::A) {
                currentAttack = { "Attack", 10, State::Attack };
                targetPos.x -= 120.f; // close range attack, but still a bit
                                      // away from the enemy
                phase = TurnPhase::Approaching;
            } else if (keyEvent->code == sf::Keyboard::Key::D) {
                currentAttack = { "Roll", 15, State::Roll };
                targetPos.x -=
                    340.f; // Start roll from further away, therefore our
                           // targetPos is a bit away from the enemy
                phase = TurnPhase::Approaching;
            }
        }
    }
}