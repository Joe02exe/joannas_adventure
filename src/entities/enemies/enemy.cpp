#include "joanna/entities/enemy.h"
#include "joanna/entities/player.h"
#include "joanna/world/tilemanager.h"
#include "joanna/utils/resourcemanager.h"

#include <algorithm>
#include <cstdlib>  // for rand
#include <iostream> // for std::cout

Enemy::Enemy(const sf::Vector2f& startPos, const std::string& idlePath)
    : Entity(
          sf::FloatRect(
              { startPos.x - 48.f, startPos.y - 32.f }, { 96.f, 64.f }
          ),
          ResourceManager<sf::Texture>::getInstance()->get(idlePath),
          // no hitbox for now
          std::nullopt,
          Direction::Right
      ),
      homePoint(startPos), patrolTarget(startPos) {

    animations[State::Idle] = Animation(idlePath, { 96, 64 }, 8);
    animations[State::Walking] =
        Animation("assets/player/enemies/goblin/run.png", { 96, 64 }, 8);
    animations[State::Running] =
        Animation("assets/player/enemies/goblin/run.png", { 96, 64 }, 8);

    animations[State::Attack] =
        Animation("assets/player/enemies/goblin/attack.png", { 96, 64 }, 9);
    animations[State::Roll] =
        Animation("assets/player/enemies/goblin/roll.png", { 96, 64 }, 10);
    animations[State::Hurt] =
        Animation("assets/player/enemies/goblin/hurt.png", { 96, 64 }, 8);
    animations[State::Dead] =
        Animation("assets/player/enemies/goblin/dead.png", { 96, 64 }, 9);
    animations[State::Mining] =
        Animation("assets/player/enemies/goblin/mining.png", { 96, 64 }, 10);

    // Set initial texture
    applyFrame();

    // Initialize some dummy attacks
    attacks.push_back({ "Scratch", 5 });
    attacks.push_back({ "Bite", 10 });
}

void Enemy::update(float dt, State state) {
    if (currentState != state) {
        switchState(state);
    }

    frameTimer += dt;
    const auto& anim = animations[currentState];
    // Use frameCount from animation if we had it exposed, or just size of
    // frames
    float duration = anim.frames.size() * Animation::frameTime;

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
    const float distToPlayer = std::sqrt(std::pow(playerPos.x - myPos.x, 2) + std::pow(playerPos.y - myPos.y, 2));
    const float distToHome = std::sqrt(std::pow(myPos.x - homePoint.x, 2) + std::pow(myPos.y - homePoint.y, 2));

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
            const float angle = (rand() % 360) * 3.14159f / 180.f;
            const float dist = (rand() % 100) / 100.f * radius;
            sf::Vector2f potentialTarget = homePoint + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
            patrolTarget = potentialTarget;
            patrolTimer = 10.f;
        }
        sf::Vector2f dir = patrolTarget - myPos;
        const float distToTarget = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        
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
            const float distNextToHome = std::sqrt(std::pow(nextPos.x - homePoint.x, 2) + std::pow(nextPos.y - homePoint.y, 2));

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
