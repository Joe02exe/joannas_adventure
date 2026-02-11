#include "joanna/utils/debug.h"
#include "joanna/entities/player.h"
#include "joanna/systems/controller.h"
#include <fmt/format.h>

#include <imgui-SFML.h>
#include <imgui.h>

void DebugUI::init(sf::RenderWindow& window) {
    ImGui::CreateContext();
    bool res = ImGui::SFML::Init(window);
    if (!res) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
    ImGui::GetIO().FontGlobalScale = 2.5f;
}

void DebugUI::processEvent(const sf::Window& window, const sf::Event& event)
    const {
    if constexpr (IMGUI_ENABLED) {
        ImGui::SFML::ProcessEvent(window, event);
    }
}

void DebugUI::update(
    const float dt, sf::RenderWindow& window, Player& player,
    GameStatus& gameStatus, CombatSystem& combatSystem, Enemy& testEnemy,
    Controller& controller
) const {
    if constexpr (!IMGUI_ENABLED) {
        return;
    }
    ImGui::Begin("Debug Window");
    ImGui::PushItemWidth(200.0f);
    std::string text = fmt::format(
        "Player pos: {:.2f}, {:.2f}", player.getPosition().x,
        player.getPosition().y
    );

    ImGui::TextUnformatted(text.c_str());

    static float input_x = 0.f;
    static float input_y = 0.f;
    static int item_id = 0;

    // Only sync values if the user isn't currently typing in an input box
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

    ImGui::InputInt("Item id", &item_id);

    if (ImGui::Button("Add item to inventory")) {
        auto map = player.getInventory().mapGidToName();
        player.getInventory().addItem(
            Item(std::to_string(item_id), map[item_id])
        );
    }
    if (ImGui::Button("Add carrot to inventory")) {
        player.getInventory().addItem(Item("691", "carrot"));
    }
    if (ImGui::Button("Add sword to inventory")) {
        player.getInventory().addItem(Item("3050", "sword"));
    }
    if (ImGui::Button("Add pickaxe to inventory")) {
        player.getInventory().addItem(Item("3113", "pickaxe"));
    }

    if (ImGui::Button("Deal 10 Damage to Player")) {
        player.takeDamage(10);
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
            player.setPosition({ 150.f, 400.f });
            player.setScale({ 1.f, 1.f });
            controller.getPlayerView().setCenter(player.getPosition());
        }
    }
    ImGui::End();
}

void DebugUI::render(sf::RenderWindow& window) const {
    if constexpr (IMGUI_ENABLED) {
        window.setView(window.getDefaultView());
    }
    ImGui::SFML::Render(window);
}

void DebugUI::shutdown() {
    ImGui::SFML::Shutdown();
}
