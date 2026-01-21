#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include "joanna/utils/resourcemanager.h"

#include <algorithm>
#include <cstdlib>  // for rand
#include <iostream> // for std::cout

Enemy::Enemy(const sf::Vector2f& startPos, EnemyType type)
    : Entity(
          sf::FloatRect(
              { startPos.x - 48.f, startPos.y - 32.f }, { 96.f, 64.f }
          ),
          ResourceManager<sf::Texture>::getInstance()->get(
              type == EnemyType::Goblin
                  ? "assets/player/enemies/goblin/idle.png"
                  : "assets/player/enemies/skeleton/idle.png"
          ),
          // no hitbox for now
          std::nullopt,
          Direction::Right
      ),
      homePoint(startPos), patrolTarget(startPos), type(type) {

    std::string basePath = type == EnemyType::Goblin
                               ? "assets/player/enemies/goblin/"
                               : "assets/player/enemies/skeleton/";

    if (type == EnemyType::Goblin) {
        animations[State::Idle] = Animation(basePath + "idle.png", { 96, 64 }, 8);
        animations[State::Walking] = Animation(basePath + "run.png", { 96, 64 }, 8);
        animations[State::Running] = Animation(basePath + "run.png", { 96, 64 }, 8);
        animations[State::Attack] = Animation(basePath + "attack.png", { 96, 64 }, 9);
        animations[State::Hurt] = Animation(basePath + "hurt.png", { 96, 64 }, 8);
        animations[State::Dead] = Animation(basePath + "dead.png", { 96, 64 }, 9);
        
        animations[State::Roll] = Animation(basePath + "roll.png", { 96, 64 }, 10);
        animations[State::Mining] = Animation(basePath + "mining.png", { 96, 64 }, 10);
    } else {
        // Skeleton
        animations[State::Idle] = Animation(basePath + "idle.png", { 96, 64 }, 6);
        animations[State::Walking] = Animation(basePath + "walk.png", { 96, 64 }, 8);
        animations[State::Running] = Animation(basePath + "walk.png", { 96, 64 }, 8);
        animations[State::Attack] = Animation(basePath + "attack.png", { 96, 64 }, 7);
        animations[State::Hurt] = Animation(basePath + "hurt.png", { 96, 64 }, 8);
        animations[State::Dead] = Animation(basePath + "dead.png", { 96, 64 }, 10);
    }

    // Set initial texture
    applyFrame();

    // Initialize attacks
    if (type == EnemyType::Goblin) {
        attacks.push_back({ "Mining", 2, State::Mining, 0.4f, 0.9f, 0.f, 130.f, 5.f, true, 0.1f, 0.43f, 130.f });
        attacks.push_back({ "Roll", 1, State::Roll, 0.2f, 0.8f, -800.f, 85.f, -5.f, true, 0.16f, 0.23f, 280.f });
    } else {
        // Skeleton attacks
        attacks.push_back({ "Attack", 0, State::Attack, 0.32f, 0.7f, 0.f, 100.f, 5.f, true, 0.1f, 0.5f, 100.f });
    }
}

void Enemy::update(float dt, State state) {
    if (currentState != state) {
        switchState(state);
    }

    frameTimer += dt;
    const auto& anim = animations[currentState];
    // Use frameCount from animation if we had it exposed, or just size of
    // frames
    float duration = static_cast<float>(anim.frames.size()) * Animation::frameTime;

    if (frameTimer >= Animation::frameTime) {
        frameTimer -= Animation::frameTime;

        if (currentState == State::Dead &&
            currentFrame == anim.frames.size() - 1) {
            // Do not loop dead animation
        } else {
            currentFrame++;
            if (currentFrame >= anim.frames.size()) {
                currentFrame = 0;
            }
        }
        applyFrame();
    }
}

void Enemy::draw(sf::RenderTarget& target) const {
    Entity::render(target);
}

void Enemy::switchState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        frameTimer = 0.f;
        applyFrame();
    }
}

void Enemy::applyFrame() {
    const auto& anim = animations[currentState];
    setTexture(anim.texture);
    setFrame(anim.frames[currentFrame]);
}

void Enemy::takeDamage(int amount) {
    health = std::max(health - amount, 0);
    std::cout << "Enemy took " << amount << " damage. Health: " << health
              << "\n";
}

bool Enemy::updateOverworld(float dt, Player& player, TileManager& tileManager) {
    const sf::Vector2f playerPos = player.getPosition();
    const sf::Vector2f myPos = getPosition();
    const auto distToPlayer = static_cast<float>(std::sqrt(std::pow(playerPos.x - myPos.x, 2) + std::pow(playerPos.y - myPos.y, 2)));
    const auto distToHome = static_cast<float>(std::sqrt(std::pow(myPos.x - homePoint.x, 2) + std::pow(myPos.y - homePoint.y, 2)));

    // trigger combat if very close to player
    if (distToPlayer < 10.f) {
        return true;
    }

    // torch radius (player "brightness")
    const float torchRadius = 20.f; 
    const bool hasLOS = tileManager.checkLineOfSight(myPos, playerPos);

    // State Transitions
    if (aiState == OverworldState::Idle) {
        if (hasLOS || distToPlayer < torchRadius) {
            if (reactionTimer == 0.f) {
                reactionTimer = 0.2f; // Start reaction delay
            } else {
                reactionTimer -= dt;
                if (reactionTimer <= 0.f) {
                    aiState = OverworldState::Pursuing;
                    reactionTimer = 0.f;
                }
            }
        } else {
            reactionTimer = 0.f; // Reset if player lost before reaction complete
        }
    } else if (aiState == OverworldState::Pursuing) {
        // stop pursuing if lost player (max radius check removed to prevent oscillation)
        if (!hasLOS && distToPlayer > torchRadius) {
            aiState = OverworldState::Idle;
            patrolTarget = homePoint;
        }
    }

    State nextAnimState = State::Idle;

    if (aiState == OverworldState::Idle) {
        patrolTimer -= dt;
        if (patrolTimer <= 0.f) {
            const float radius = 30.f;
            const float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.f;
            const float dist = static_cast<float>(rand() % 100) / 100.f * radius;
            sf::Vector2f potentialTarget = homePoint + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
            patrolTarget = potentialTarget;
            patrolTimer = 10.f;
        }
        sf::Vector2f dir = patrolTarget - myPos;
        const float distToTarget = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
        
        if (distToTarget > 5.f) {
            dir /= distToTarget;
            const sf::Vector2f move = dir * speed * dt * 0.5f;

            setPosition(myPos + move);
            nextAnimState = State::Walking;
            setFacing(dir.x > 0 ? Direction::Right : Direction::Left);
        } else {
            nextAnimState = State::Idle;
        }

    } else if (aiState == OverworldState::Pursuing) {
        // chase player
        sf::Vector2f dir = playerPos - myPos;
        if (distToPlayer > 10.f) {
            dir /= distToPlayer;
            const sf::Vector2f move = dir * speed * dt;
            const sf::Vector2f nextPos = myPos + move;
            const auto distNextToHome = static_cast<float>(std::sqrt(std::pow(nextPos.x - homePoint.x, 2) + std::pow(nextPos.y - homePoint.y, 2)));

            // move only if the NEW position is within the max pursuit radius -> this allows the enemy to move back towards home if it's currently stuck at the boundary
            if (distNextToHome <= 60.f) {
                setPosition(nextPos);
                nextAnimState = State::Running;
            } else {
                nextAnimState = State::Idle;
            }
            
            // always face the player when pursuing
            setFacing(dir.x > 0 ? Direction::Right : Direction::Left);
        } else {
             nextAnimState = State::Idle;
             setFacing(dir.x > 0 ? Direction::Right : Direction::Left);
        }
    }
    update(dt, nextAnimState);
    return false;
}
