#pragma once

#include "./player.h"
#include "joanna/entities/interactable.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>

class NPC: public Interactable {
  public:
    NPC(const sf::Vector2f& startPos, const std::string& name,
        const std::string& buttonTexturePath);

    void interact() override;

    void update(float dt, Player::State state, bool facingLeft);

    void applyFrame();

  private:
    Player::State currentState = Player::State::Idle;
    Player::Direction facing = Player::Direction::Right;
    float frameTimer = 0.f;
    int currentFrame = 0;
    std::unordered_map<Player::State, Animation> animations;
};
