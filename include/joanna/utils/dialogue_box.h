#pragma once
#include "joanna/systems/font_renderer.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <queue>

class DialogueBox {
public:

    DialogueBox(FontRenderer& fontRenderer);

    void setDialogue(const std::vector<std::string>& messages, const void* owner = nullptr);
    void addDialogueLine(const std::string& message);
    void show();
    void hide();
    void clear();
    void nextLine();
    void skipTypewriter();
    
    bool isActive() const { return active; }
    bool isTyping() const { return visibleCharCount < currentDialogue.length(); }
    bool hasMoreLines() const { return !dialogueQueue.empty(); }
    const void* getOwner() const { return owner; }
    
    void update(float dt, const sf::Vector2f& targetPosition);
    void render(sf::RenderTarget& target);

private:
    void updateTypewriter(float dt);
    std::string wrapText(const std::string& text, float maxWidth);

    FontRenderer fontRenderer;

    bool active = false;
    std::queue<std::string> dialogueQueue;
    std::string currentDialogue;
    std::string visibleText;
    size_t visibleCharCount = 0;
    float displayTime = 0.0f;
    
    sf::RectangleShape bubbleBackground;
    sf::ConvexShape bubblePointer;
    sf::Vector2f textPosition;
    const void* owner = nullptr;
    static constexpr float BUBBLE_WIDTH = 370.0f;
    static constexpr float TYPE_SPEED = 0.05f;
    float currentTextHeight = 0.0f;
};