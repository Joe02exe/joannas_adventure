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
}

void CombatSystem::endCombat() {
    if (player != nullptr) {
        player->setPosition(playerState.position);
        player->setScale(playerState.scale);
        player->setFacing(playerState.facing);
    }
    if (enemy != nullptr) {
        enemy->setPosition(enemyState.position);
        enemy->setScale(enemyState.scale);
        enemy->setFacing(enemyState.facing);
        enemy->takeDamage(-100);
    }
}

void CombatSystem::update(float dt) {

    dt -= 0.007f; // just for visual purpose
    static AudioManager dummyAudio;

    if (currentState == CombatState::PlayerTurn) {
        updatePlayerTurn(dt, pState, eState);
    } else if (currentState == CombatState::EnemyTurn) {
        updateEnemyTurn(dt, pState, eState);
    }

    // Determine facing for player
    // Player faces Right (towards enemy) unless returning
    bool facingLeft = false;
    if (currentState == CombatState::PlayerTurn &&
        phase == TurnPhase::Returning) {
        facingLeft = true;
    }

    player->update(dt, pState, facingLeft, dummyAudio);
    enemy->update(dt, eState);
}

void CombatSystem::updatePlayerTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Approaching) {
        pState = State::Running;
        sf::Vector2f dir = targetPos - player->getPosition();
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 10.f) {
            dir /= dist;
            player->setPosition(player->getPosition() + dir * 500.f * dt);
        } else {
            phase = TurnPhase::Attacking;
            turnTimer = 0.0f;
        }
    } else if (phase == TurnPhase::Attacking) {
        pState = currentAttack.animationState;

        // Hurt animation logic: delay 0.2s, play once (approx 0.64s)
        if (currentAttack.name == "Attack") {
            if (turnTimer < 0.3f) {
                eState = State::Idle;
            } else if (turnTimer < 0.8f) {
                eState = State::Hurt;
            } else {
                eState = State::Idle;
                phase = TurnPhase::Returning;
                turnTimer = 0.0f;
                enemy->takeDamage(currentAttack.damage);
            }
        } else {
        }

        // Move player during Roll
        if (currentAttack.name == "Roll") {
            sf::Vector2f rollTarget = enemy->getPosition();
            rollTarget.x -= 90.f; // Stop right in front
            sf::Vector2f dir = rollTarget - player->getPosition();
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 5.f) {
                dir /= dist;
                player->setPosition(player->getPosition() + dir * 800.f * dt);
            }
            if (turnTimer < 0.2f) {
                eState = State::Idle;
            } else if (turnTimer < 0.85f) {
                eState = State::Hurt;
            } else {
                eState = State::Idle;
                phase = TurnPhase::Returning;
                turnTimer = 0.0f;
                enemy->takeDamage(currentAttack.damage);
            }
        }

        turnTimer += dt;

    } else if (phase == TurnPhase::Returning) {
        pState = State::Running;
        sf::Vector2f dir = startPos - player->getPosition();
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 10.f) {
            dir /= dist;
            player->setPosition(player->getPosition() + dir * 500.f * dt);
        } else {
            player->setPosition(startPos);
            phase = TurnPhase::EndTurn;
            pState = State::Idle;
        }
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

void CombatSystem::updateEnemyTurn(float dt, State& pState, State& eState) {
    if (phase == TurnPhase::Input) {
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
    } else if (phase == TurnPhase::Approaching) {
        eState = State::Running;
        enemy->setFacing(Direction::Left);
        sf::Vector2f dir = targetPos - enemy->getPosition();
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 10.f) {
            dir /= dist;
            enemy->setPosition(enemy->getPosition() + dir * 500.f * dt);
        } else {
            phase = TurnPhase::Attacking;
            turnTimer = 0.0f;
        }
    } else if (phase == TurnPhase::Attacking) {
        eState = currentAttack.animationState;

        if (currentAttack.name == "Mining") {
            if (turnTimer < 0.4f) {
                pState = State::Idle;
            } else if (turnTimer < 0.9f) {
                pState = State::Hurt;
            } else {
                pState = State::Idle;
                phase = TurnPhase::Returning;
                turnTimer = 0.0f;
                player->takeDamage(currentAttack.damage);
            }
        }

        // Move enemy during Roll
        if (currentAttack.name == "Roll") {
            sf::Vector2f rollTarget = player->getPosition();
            rollTarget.x += 85.f; // Stop right in front
            sf::Vector2f dir = rollTarget - enemy->getPosition();
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 5.f) {
                dir /= dist;
                enemy->setPosition(enemy->getPosition() + dir * 800.f * dt);
            }
            if (turnTimer < 0.2f) {
                pState = State::Idle;
            } else if (turnTimer < 0.8f) {
                pState = State::Hurt;
            } else {
                pState = State::Idle;
                phase = TurnPhase::Returning;
                turnTimer = 0.0f;
                enemy->takeDamage(currentAttack.damage);
            }
        }

        turnTimer += dt;
    } else if (phase == TurnPhase::Returning) {
        enemy->setFacing(Direction::Right);
        eState = State::Running;
        sf::Vector2f dir = startPos - enemy->getPosition();
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 10.f) {
            dir /= dist;
            enemy->setPosition(enemy->getPosition() + dir * 500.f * dt);
        } else {
            enemy->setPosition(startPos);
            enemy->setFacing(Direction::Left);
            phase = TurnPhase::EndTurn;
            eState = State::Idle;
        }
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

void CombatSystem::render(sf::RenderTarget& target) {

    // currently set statically... because  viewport is set to 900x900
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
                targetPos.x -= 120.f; // Close range
                phase = TurnPhase::Approaching;
            } else if (keyEvent->code == sf::Keyboard::Key::D) {
                currentAttack = { "Roll", 15, State::Roll };
                targetPos.x -= 340.f; // Start roll from further away
                phase = TurnPhase::Approaching;
            }
        }
    }
}