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
      )),
      attackButtonRollTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/attack_roll.png"
      )),
      counterButtonTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/attack_punch.png"
      )),
      counterButtonGoodTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/attack_punch_good.png"
      )),
      counterButtonBadTexture(ResourceManager<sf::Texture>::getInstance()->get(
          "assets/buttons/attack_punch_bad.png"
      )) {}

// Explicit instantiations
template void CombatSystem::processAttack<
    Player>(float, Entity*, Player*, State&, State&, const Attack&);
template void CombatSystem::processAttack<
    Enemy>(float, Entity*, Enemy*, State&, State&, const Attack&);
template void CombatSystem::updateAttackTimeline<
    Player>(float, Player*, State&, const Attack&);
template void
CombatSystem::updateAttackTimeline<Enemy>(float, Enemy*, State&, const Attack&);

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
    enemy->resetHealth();
}

void CombatSystem::update(float dt) {

    dt -= 0.007f; // just for visual purpose
    static AudioManager dummyAudio;

    if (currentState == CombatState::PlayerTurn) {
        updatePlayerTurn(dt, pState, eState);
    } else if (currentState == CombatState::EnemyTurn) {
        updateEnemyTurn(dt, pState, eState);
    }

    player->update(
        dt, pState, player->getFacing() == Direction::Left, dummyAudio
    );
    enemy->update(dt, eState);
}

void CombatSystem::processApproach(
    float dt, Entity* actor, sf::Vector2f target, float speed, float threshold,
    State& actorState
) {
    actorState = State::Running;
    sf::Vector2f dir = target - actor->getPosition();

    // move if distance is greater than threshold (considering direction)
    bool shouldMove =
        (speed > 0 && dir.x > threshold) || (speed < 0 && dir.x < -threshold);

    if (shouldMove) {
        actor->setPosition(
            actor->getPosition() + sf::Vector2f(speed * dt, 0.f)
        );
    } else {
        phase = TurnPhase::Attacking;
        turnTimer = 0.0f;
    }
}

void CombatSystem::processReturn(
    float dt, Entity* actor, sf::Vector2f startPos, float speed,
    float threshold, State& actorState, Direction moveFacing,
    Direction endFacing
) {
    actor->setFacing(moveFacing);
    actorState = State::Running;
    sf::Vector2f dir = startPos - actor->getPosition();

    bool shouldMove =
        (speed > 0 && dir.x > threshold) || (speed < 0 && dir.x < -threshold);

    if (shouldMove) {
        actor->setPosition(
            actor->getPosition() + sf::Vector2f(speed * dt, 0.f)
        );
    } else {
        actor->setPosition(startPos);
        actor->setFacing(endFacing);
        phase = TurnPhase::EndTurn;
        actorState = State::Idle;
    }
}

void CombatSystem::updateAttackMovement(
    float dt, Entity* attacker, const sf::Vector2f& targetPos,
    const Attack& attack
) {
    if (attack.moveSpeed != 0.f) {
        sf::Vector2f target = targetPos;
        target.x += attack.targetOffset;
        sf::Vector2f dir = target - attacker->getPosition();

        bool shouldMove =
            (attack.moveSpeed > 0 && dir.x > attack.moveThreshold) ||
            (attack.moveSpeed < 0 && dir.x < -attack.moveThreshold);

        if (shouldMove) {
            attacker->setPosition(
                attacker->getPosition() +
                sf::Vector2f(attack.moveSpeed * dt, 0.f)
            );
        }
    }
}

template <typename Defender>
void CombatSystem::updateAttackTimeline(
    float dt, Defender* defender, State& defenderState, const Attack& attack
) {
    if (turnTimer < attack.impactTime) {
        if (defenderState != State::Counter) {
            defenderState = State::Idle;
        }
    } else if (turnTimer < attack.endTime) {
        defenderState = State::Hurt;
    } else {
        defenderState = State::Idle;
        phase = TurnPhase::Returning;
        turnTimer = 0.0f;
        defender->takeDamage(attack.damage);
    }
}

template <typename Defender>
void CombatSystem::processAttack(
    float dt, Entity* attacker, Defender* defender, State& attackerState,
    State& defenderState, const Attack& attack
) {
    attackerState = attack.animationState;

    updateAttackMovement(dt, attacker, defender->getPosition(), attack);
    updateAttackTimeline(dt, defender, defenderState, attack);

    turnTimer += dt;
}

void CombatSystem::updatePlayerTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Countering) {
        processCounter(dt);
        return;
    }
    if (phase == TurnPhase::Approaching) {
        processApproach(dt, player, targetPos, 500.f, 10.f, pState);
    } else if (phase == TurnPhase::Attacking) {
        processAttack(dt, player, enemy, pState, eState, currentAttack);
    } else if (phase == TurnPhase::Returning) {
        processReturn(
            dt, player, startPos, -500.f, -10.f, pState, Direction::Left,
            Direction::Right
        );
    } else if (phase == TurnPhase::EndTurn) {
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

void CombatSystem::e_chooseAttack() {
    startPos = enemy->getPosition();
    targetPos = player->getPosition();

    const auto& attacks = enemy->getAttacks();

    int attackIndex = static_cast<int>(rand() % attacks.size());
    currentAttack = attacks[attackIndex];

    // apply offset based on attack configuration
    targetPos.x += currentAttack.approachOffset;

    Logger::info(currentAttack.name + " selected");
    phase = TurnPhase::Approaching;
}

void CombatSystem::updateEnemyTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Input) {
        e_chooseAttack();
        damageDealt = false;
    } else if (phase == TurnPhase::Approaching) {
        processApproach(dt, enemy, targetPos, -500.f, -10.f, eState);
    } else if (phase == TurnPhase::Attacking) {
        processAttack(dt, enemy, player, eState, pState, currentAttack);
    } else if (phase == TurnPhase::Returning) {
        processReturn(
            dt, enemy, startPos, 500.f, 10.f, eState, Direction::Right,
            Direction::Left
        );
    } else if (phase == TurnPhase::EndTurn) {
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

void CombatSystem::processCounter(float dt) {
    eState = State::Hurt;
    turnTimer += dt;

    if (counterSuccess && !damageDealt) {
        enemy->takeDamage(1);
        damageDealt = true;
    }

    if (turnTimer > currentAttack.counterWindowEnd) {
        if (enemy->getHealth() <= 0) {
            eState = State::Dead;
            currentState = CombatState::Victory;
            std::cout << "Victory!\n";
        } else {
            eState = State::Idle;
            phase = TurnPhase::Returning;
            currentState = CombatState::EnemyTurn;
        }
    }
}

void CombatSystem::render(sf::RenderTarget& target, TileManager& tileManager) {

    // currently set statically... because viewport is set to 900x900
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale({ 900.f / 1400.f, 900.f / 1400.f });
    backgroundSprite.setPosition({ 0.f, 0.f });
    target.draw(backgroundSprite);

    // either render player or enemy first based on current combat state
    if (currentState == CombatState::EnemyTurn) {
        player->draw(target);
        enemy->draw(target);
    } else {
        enemy->draw(target);
        player->draw(target);
    }

    player->displayHealthBar(target, tileManager);

    if (currentState == CombatState::PlayerTurn && phase == TurnPhase::Input) {
        if (player->getInventory().hasItem("3050")) {
            sf::Sprite attackButtonSprite(attackButtonTexture);
            attackButtonSprite.setScale({ 3, 3 });
            attackButtonSprite.setPosition({ 95.f, 300.f });
            target.draw(attackButtonSprite);
        } else {
            sf::Sprite attackButtonSprite(attackButtonRollTexture);
            attackButtonSprite.setScale({ 3, 3 });
            attackButtonSprite.setPosition({ 95.f, 300.f });
            target.draw(attackButtonSprite);
        }
    }

    if (currentAttack.counterable && currentState == CombatState::EnemyTurn &&
        (phase == TurnPhase::Attacking || phase == TurnPhase::Approaching)) {
        sf::Sprite counterButtonSprite(counterButtonTexture);

        if (phase == TurnPhase::Attacking &&
            turnTimer >= currentAttack.counterWindowStart &&
            turnTimer <= currentAttack.counterWindowEnd) {
            counterButtonSprite.setTexture(counterButtonGoodTexture);
        } else if (phase == TurnPhase::Attacking &&
                   turnTimer > currentAttack.counterWindowEnd) {
            counterButtonSprite.setTexture(counterButtonBadTexture);
        } else {
            counterButtonSprite.setTexture(counterButtonTexture);
        }

        counterButtonSprite.setScale({ 3, 3 });
        counterButtonSprite.setPosition({ 95.f, 300.f });
        target.draw(counterButtonSprite);
    }
}

void CombatSystem::handleInput(sf::Event& event) {
    if (currentState == CombatState::PlayerTurn && phase == TurnPhase::Input) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            startPos = player->getPosition();
            targetPos = enemy->getPosition();
            if (keyEvent->code == sf::Keyboard::Key::A) {
                if (player->getInventory().hasItem("3050")) {
                    currentAttack = { "Attack", 2, State::Attack, 0.3f, 0.8f };
                    targetPos.x -= 120.f; // close range attack, but still a bit
                                          // away from the enemy
                    phase = TurnPhase::Approaching;
                }
            } else if (keyEvent->code == sf::Keyboard::Key::D) {
                currentAttack = { "Roll", 1,     State::Roll, 0.2f,
                                  0.85f,  800.f, -90.f,       5.f };
                targetPos.x -=
                    340.f; // start roll from further away, therefore our
                           // targetPos is a bit away from the enemy
                phase = TurnPhase::Approaching;
            }
        }
    } else if (currentState == CombatState::EnemyTurn &&
               (phase == TurnPhase::Attacking || phase == TurnPhase::Approaching
               ) &&
               currentAttack.counterable) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::D) {
                if (pState == State::Counter)
                    return; // prevents spamming (spamming is still possible but
                            // a bit restricted still with this)

                if (phase == TurnPhase::Attacking &&
                    turnTimer >= currentAttack.counterWindowStart &&
                    turnTimer <= currentAttack.counterWindowEnd) {
                    Logger::info(
                        "Counter successful for turntimer: " +
                        std::to_string(turnTimer)
                    );
                    counterSuccess = true;
                    damageDealt = false;
                    currentState = CombatState::PlayerTurn;
                    phase = TurnPhase::Countering; // use dedicated phase
                    turnTimer = 0.0f;
                    pState = State::Counter;
                } else {
                    Logger::info(
                        "Counter failed for turntimer: " +
                        std::to_string(turnTimer)
                    );
                    counterSuccess = false;
                    // Just play animation, don't interrupt enemy
                    pState = State::Counter;
                    // Player::update will switch back to Idle automatically
                }
            }
        }
    }
}