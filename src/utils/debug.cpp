#include "joanna/utils/debug.h"

#include "joanna/entities/player.h"

#include <imgui-SFML.h>
#include <imgui.h>

void DebugUI::init(sf::RenderWindow& window) {
    bool res = ImGui::SFML::Init(window);
    if (!res) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
}

void DebugUI::processEvent(
    const sf::Window& window, const sf::Event& event
) const {
    if (enabled) {
        ImGui::SFML::ProcessEvent(window, event);
    }
}

void DebugUI::update(
    const float dt, sf::RenderWindow& window, Player& player
) const {
    if (!enabled) {
        return;
    }
    ImGui::SFML::Update(window, sf::seconds(dt));

    ImGui::Begin("Debug Window");
    ImGui::Text(
        "Player pos: %.2f, %.2f", player.getPosition().x, player.getPosition().y
    );
    ImGui::End();
}

void DebugUI::render(sf::RenderWindow& window) const {
    if (!enabled) {
        return;
    }

    window.setView(window.getDefaultView()); // IMPORTANT
    ImGui::SFML::Render(window);
}

void DebugUI::shutdown() {
    ImGui::SFML::Shutdown();
}
