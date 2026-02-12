#include "joanna/utils/dialogue_box.h"
#include "joanna/utils/logger.h"

DialogueBox::DialogueBox(FontRenderer& fontRenderer)
    : fontRenderer(fontRenderer) {

    bubbleBackground.setFillColor(sf::Color(255, 255, 255, 240));
    bubbleBackground.setOutlineColor(sf::Color(50, 50, 50));
    bubbleBackground.setOutlineThickness(2.0f);

}

static const float TEXT_MAX_WIDTH = 200.0f;

void DialogueBox::setDialogue(
    const std::vector<std::string>& messages, const void* owner
) {
    clear();
    this->owner = owner;
    for (const auto& msg : messages) {
        dialogueQueue.push(msg);
    }
}

void DialogueBox::addDialogueLine(const std::string& message) {
    dialogueQueue.push(message);
}

void DialogueBox::show() {
    if (!dialogueQueue.empty() && !active) {
        active = true;

        currentDialogue.clear();
        visibleCharCount = 0;

        nextLine();
    }
}

void DialogueBox::hide() {
    active = false;
}

void DialogueBox::clear() {
    while (!dialogueQueue.empty()) {
        dialogueQueue.pop();
    }
    currentDialogue.clear();
    visibleText.clear();
    visibleCharCount = 0;
    displayTime = 0.0f;
    owner = nullptr;
}

void DialogueBox::nextLine() {
    if (active && !currentDialogue.empty() &&
        visibleCharCount < currentDialogue.length()) {
        skipTypewriter();
    } else if (!dialogueQueue.empty()) {
        std::string rawMessage = dialogueQueue.front();
        dialogueQueue.pop();

        currentDialogue = wrapText(rawMessage, TEXT_MAX_WIDTH);

        sf::Text measuringText(fontRenderer.getFont(), currentDialogue);
        measuringText.setCharacterSize(16);
        
        currentTextHeight = measuringText.getLocalBounds().size.y;

        visibleCharCount = 0;
        displayTime = 0.0f;
    } else {
        hide();
    }
}

void DialogueBox::skipTypewriter() {
    visibleCharCount = currentDialogue.length();
    displayTime = currentDialogue.length() * TYPE_SPEED;
}

std::string DialogueBox::wrapText(const std::string& text, float maxWidth) {
    std::string wrappedText;
    std::string currentLine;
    std::string word;

    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];

        if (c == ' ' || c == '\n' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ' && c != '\n') {
                word += c;
            }

            std::string testLine =
                currentLine.empty() ? word : currentLine + " " + word;

            sf::Text tempText(fontRenderer.getFont(), testLine);
            tempText.setCharacterSize(16);
            float lineWidth = tempText.getLocalBounds().size.x;

            if (lineWidth > maxWidth && !currentLine.empty()) {
                wrappedText += currentLine + "\n";
                currentLine = word;
            } else {
                currentLine = testLine;
            }

            if (c == '\n') {
                wrappedText += currentLine + "\n";
                currentLine.clear();
            }

            word.clear();
        } else {
            word += c;
        }
    }

    if (!currentLine.empty()) {
        wrappedText += currentLine;
    }

    return wrappedText;
}

void DialogueBox::updateTypewriter(float dt) {
    if (!active)
        return;

    displayTime += dt;

    size_t targetCharCount = static_cast<size_t>(displayTime / TYPE_SPEED);

    if (targetCharCount > currentDialogue.length()) {
        visibleCharCount = currentDialogue.length();
    } else {
        visibleCharCount = targetCharCount;
    }

    visibleText = currentDialogue.substr(0, visibleCharCount);
}

void DialogueBox::update(float dt, const sf::Vector2f& targetPosition) {
    if (!active)
        return;

    updateTypewriter(dt);
}

void DialogueBox::render(sf::RenderTarget& target) {
    if (!active || !fontRenderer.isLoaded())
        return;

    sf::View oldView = target.getView();

    target.setView(target.getDefaultView());
    sf::Vector2f viewSize = target.getDefaultView().getSize();

    float bubbleHeight = currentTextHeight + 30.0f;
    bubbleBackground.setSize(sf::Vector2f(BUBBLE_WIDTH, bubbleHeight * 2.0f));

    bubbleBackground.setPosition({ (viewSize.x - BUBBLE_WIDTH) / 2.0f,
                                   (viewSize.y / 1.4f) -
                                       bubbleBackground.getSize().y - 20.0f });

    textPosition = bubbleBackground.getPosition() + sf::Vector2f(15.0f, 25.0f);

    target.draw(bubbleBackground);

    fontRenderer.drawText(
        target, visibleText, textPosition, 24, sf::Color::Black, 0
    );

    if (!isTyping()) {
        std::string indicator = "< Space >";
        sf::Vector2f indicatorPos = bubbleBackground.getPosition() +
                                    bubbleBackground.getSize() -
                                    sf::Vector2f(100.0f, 25.0f);

        fontRenderer.drawText(
            target, indicator, indicatorPos, 16, sf::Color(100, 100, 100), 0
        );
    }

    target.setView(oldView);
}