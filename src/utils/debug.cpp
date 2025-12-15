#include "joanna/utils/debug.h"
#include "joanna/entities/player.h"
#include <fmt/format.h>

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
    GameStatus& gameStatus, CombatSystem& combatSystem, Enemy& testEnemy
) const {
    if (!enabled) {
        return;
    }
    ImGui::SFML::Update(window, sf::seconds(dt));

    ImGui::Begin("Debug Window");
    ImGui::PushItemWidth(200.0f);
    std::string text = fmt::format(
        "Player pos: {:.2f}, {:.2f}", player.getPosition().x,
        player.getPosition().y
    );

    ImGui::TextUnformatted(text.c_str());

    // persistent UI state so user edits aren't lost each frame
    static float input_x = 0.f;
    static float input_y = 0.f;
    static int item_id = 0;
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

    if (ImGui::InputInt("Item id", &item_id)) {
    }
    if (ImGui::Button("Add item to inventory")) {
        player.getInventory().addItem(Item(std::to_string(item_id), "item"));
    }
    if (ImGui::Button("Add carrot to inventory")) {
        player.getInventory().addItem(Item("691", "carrot"));
    }

    if (ImGui::Button("Add sword to inventory")) {
        player.getInventory().addItem(Item("3050", "sword"));
    }

    if (gameStatus == GameStatus::Overworld) {
        if (ImGui::Button("Start Combat")) {
            gameStatus = GameStatus::Combat;
            combatSystem.startCombat(player, testEnemy);
        }
    } else {
        if (ImGui::Button("End Combat")) {
            gameStatus = GameStatus::Overworld;
            combatSystem.endCombat();
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
