#include "joanna/utils/debug.h"
#include <fmt/format.h>
#include "joanna/entities/player.h"

#include <imgui-SFML.h>
#include <imgui.h>

void DebugUI::init(sf::RenderWindow& window) {
    bool res = ImGui::SFML::Init(window);
    if (!res) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
    ImGui::GetIO().FontGlobalScale = 2.5f;
}

void DebugUI::processEvent(const sf::Window& window, const sf::Event& event)
    const {
    if (enabled) {
        ImGui::SFML::ProcessEvent(window, event);
    }
}

void DebugUI::update(
    const float dt, sf::RenderWindow& window, Player& player,
    GameState& gameState, CombatSystem& combatSystem, Enemy* testEnemy
) const {
    if (!enabled) {
        return;
    }
    ImGui::SFML::Update(window, sf::seconds(dt));

    ImGui::Begin("Debug Window");
    ImGui::PushItemWidth(200.0f);
    ImGui::Text(
        "Player pos: %.2f, %.2f", player.getPosition().x, player.getPosition().y
    );

    // persistent UI state so user edits aren't lost each frame
    static float input_x = 0.f;
    static float input_y = 0.f;
    // sync initial values (only if not interacting)
    if (!ImGui::IsAnyItemActive()) {
        input_x = player.getPosition().x;
        input_y = player.getPosition().y;
    }

    if (ImGui::InputFloat("Set Player Position X", &input_x)) {
        player.setPosition({ input_x, player.getPosition().y });
    }
    if (ImGui::InputFloat("Set Player Position Y", &input_y)) {
        player.setPosition({ player.getPosition().x, input_y });
    }

    if (gameState == GameState::Overworld) {
        if (ImGui::Button("Start Combat")) {
            gameState = GameState::Combat;
            combatSystem.startCombat(player, testEnemy);
        }
    } else {
        if (ImGui::Button("End Combat")) {
            gameState = GameState::Overworld;
        }
    }
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
