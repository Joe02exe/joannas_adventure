#pragma once

#include "joanna/entities/entityutils.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>

class NPC: public Interactable {
  public:
    NPC(const sf::Vector2f& startPos, const std::string& npcTexturePath,
        const std::string& buttonTexturePath,
        std::shared_ptr<DialogueBox> dialogueBox);

    void interact() override;

    void update(
        float dt, State state, bool facingLeft, const sf::Vector2f& playerPos
    );

    void setDialogue(const std::vector<std::string>& messages);

    std::shared_ptr<DialogueBox> getDialogueBox() {
        return dialogueBox;
    }

    void applyFrame();

  private:
    State currentState = State::Idle;
    float frameTimer = 0.f;
    int currentFrame = 0;
    std::unordered_map<State, Animation> animations;
    std::shared_ptr<DialogueBox> dialogueBox;
};
