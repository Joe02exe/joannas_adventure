#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include "joanna/utils/resourcemanager.h"
#include "joanna/world/tilemanager.h"

#include <algorithm>
#include <cstdlib> // for rand

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
          std::nullopt, Direction::Right
      ),
      homePoint(startPos), patrolTarget(startPos), type(type) {

    std::string basePath = type == EnemyType::Goblin
                               ? "assets/player/enemies/goblin/"
                               : "assets/player/enemies/skeleton/";

    if (type == EnemyType::Goblin) {
        animations[State::Idle] =
            Animation(basePath + "idle.png", { 96, 64 }, 8);
        animations[State::Walking] =
            Animation(basePath + "run.png", { 96, 64 }, 8);
        animations[State::Running] =
            Animation(basePath + "run.png", { 96, 64 }, 8);
        animations[State::Attack] =
            Animation(basePath + "attack.png", { 96, 64 }, 9);
        animations[State::Hurt] =
            Animation(basePath + "hurt.png", { 96, 64 }, 8);
        animations[State::Dead] =
            Animation(basePath + "dead.png", { 96, 64 }, 9);

        animations[State::Roll] =
            Animation(basePath + "roll.png", { 96, 64 }, 10);
        animations[State::Mining] =
            Animation(basePath + "mining.png", { 96, 64 }, 10);
    } else {
        // Skeleton
        animations[State::Idle] =
            Animation(basePath + "idle.png", { 96, 64 }, 6);
        animations[State::Walking] =
            Animation(basePath + "walk.png", { 96, 64 }, 8);
        animations[State::Running] =
            Animation(basePath + "walk.png", { 96, 64 }, 8);
        animations[State::Attack] =
            Animation(basePath + "attack.png", { 96, 64 }, 7);
        animations[State::Hurt] =
            Animation(basePath + "hurt.png", { 96, 64 }, 8);
        animations[State::Dead] =
            Animation(basePath + "dead.png", { 96, 64 }, 10);
    }

    // Set initial texture
    applyFrame();

    // Initialize attacks
    if (type == EnemyType::Goblin) {
        attacks.push_back({ "Mining", 50, State::Mining, 0.4f, 0.9f, 0.f, 130.f,
                            5.f, true, 0.1f, 0.43f, 130.f });
        attacks.push_back({ "Roll", 30, State::Roll, 0.2f, 0.8f, -800.f, 85.f,
                            -5.f, true, 0.16f, 0.23f, 280.f });
        // set healöth higher
        maxHealth = 200;
        health = maxHealth;
    } else {
        // Skeleton attacks
        attacks.push_back({ "Attack", 30, State::Attack, 0.32f, 0.7f, 0.f,
                            100.f, 5.f, true, 0.05f, 0.35f, 100.f });
    }
}

void Enemy::update(float dt, State state) {
    if (currentState != state) {
        switchState(state);
    }

    this->frameTimer += dt;
    const auto& anim = animations[this->currentState];
    // Use frameCount from animation if we had it exposed, or just size of
    // frames
    float duration =
        static_cast<float>(anim.frames.size()) * Animation::frameTime;

    if (this->frameTimer >= Animation::frameTime) {
        this->frameTimer -= Animation::frameTime;

        if (this->currentState == State::Dead &&
            this->currentFrame == anim.frames.size() - 1) {
            // Do not loop dead animation
        } else {
            this->currentFrame++;
            if (this->currentFrame >= anim.frames.size()) {
                this->currentFrame = 0;
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
        this->currentState = newState;
        this->currentFrame = 0;
        this->frameTimer = 0.f;
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
}

int Enemy::updateOverworld(float dt, Player& player, TileManager& tileManager) {
    const sf::Vector2f playerPos = player.getPosition();
    const sf::Vector2f myPos = getPosition();
    const auto distToPlayer = getDistance(playerPos, myPos);

    if (shouldTriggerCombat(distToPlayer)) {
        return COMBAT_TRIGGERED;
    }

    updateAIState(dt, myPos, playerPos, distToPlayer, tileManager);

    State nextAnimState = State::Idle;

    if (aiState == OverworldState::Idle) {
        nextAnimState = handleIdleBehavior(dt, myPos);
    } else if (aiState == OverworldState::Pursuing) {
        nextAnimState =
            handlePursuingBehavior(dt, myPos, playerPos, distToPlayer);
    }
    // graphical update
    update(dt, nextAnimState);
    return COMBAT_IDLE;
}

void Enemy::updateAIState(
    float dt, const sf::Vector2f& myPos, const sf::Vector2f& playerPos,
    float distToPlayer, TileManager& tileManager
) {
    const float torchRadius = 100.f; // player "brightness"
    const bool hasLOS = tileManager.checkLineOfSight(myPos, playerPos);

    if (aiState == OverworldState::Idle) {
        if (hasLOS && distToPlayer < torchRadius) {
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
            reactionTimer = 0.f;
        }
    } else if (aiState == OverworldState::Pursuing) {
        // stop pursuing if lost player
        if (!hasLOS || distToPlayer > torchRadius) {
            aiState = OverworldState::Idle;
            patrolTarget = homePoint;
        }
    }
}

State Enemy::handleIdleBehavior(float dt, const sf::Vector2f& myPos) {
    patrolTimer -= dt;
    if (patrolTimer <= 0.f) {
        const float radius = 30.f;
        const float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.f;
        const float dist = static_cast<float>(rand() % 100) / 100.f * radius;
        sf::Vector2f potentialTarget =
            homePoint +
            sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
        patrolTarget = potentialTarget;
        patrolTimer = 10.f;
    }

    sf::Vector2f dir = patrolTarget - myPos;
    const float distToTarget = getDistance(myPos, patrolTarget);

    if (distToTarget > 5.f) {
        dir /= distToTarget;
        const sf::Vector2f move = dir * speed * dt * 0.5f;

        setPosition(myPos + move);
        setFacing(dir.x > 0 ? Direction::Right : Direction::Left);
        return State::Walking;
    }

    return State::Idle;
}

float Enemy::getDistance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    return static_cast<float>(
        std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2))
    );
}

bool Enemy::shouldTriggerCombat(float distToPlayer) {
    return distToPlayer < 10.f;
}

State Enemy::handlePursuingBehavior(
    float dt, const sf::Vector2f& myPos, const sf::Vector2f& playerPos,
    float distToPlayer
) {
    sf::Vector2f dir = playerPos - myPos;
    setFacing(dir.x > 0 ? Direction::Right : Direction::Left);
    dir /= distToPlayer;
    const sf::Vector2f move = dir * speed * dt;
    const sf::Vector2f nextPos = myPos + move;
    const auto distNextToHome = getDistance(nextPos, homePoint);

    sf::Vector2f actualNextPos = myPos;
    // move only if the new position is within max radius
    if (distNextToHome <= 60.f) {
        actualNextPos = nextPos;
    } else {
        // slide along the border
        sf::Vector2f homeToNext = nextPos - homePoint;
        float dist = std::sqrt(
            (homeToNext.x * homeToNext.x) + (homeToNext.y * homeToNext.y)
        );
        if (dist > 0.001f) {
            actualNextPos = homePoint + (homeToNext / dist) * 60.f;
        }
    }
    // only run if we actually moved significantly compared to the last frame
    if (std::abs(actualNextPos.x - myPos.x) > 0.1f ||
        std::abs(actualNextPos.y - myPos.y) > 0.1f) {
        setPosition(actualNextPos);
        return State::Running;
    }
    return State::Idle;
}