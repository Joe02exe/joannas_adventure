#pragma once

#include "audiomanager.h"
#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <list>

class Controller {
  public:
    Controller(WindowManager& windowManager, AudioManager& audioManager);

    bool getInput(
        float dt, sf::RenderWindow& window,
        const std::vector<sf::FloatRect>& collisions,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& sharedDialogueBox,
        TileManager& tileManager, RenderEngine& renderEngine

    );

    bool updateStep(
        float dt, sf::RenderWindow& window,
        std::vector<sf::FloatRect>& collisions,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& sharedDialogueBox,
        TileManager& tileManager, RenderEngine& renderEngine

    );

    sf::View& getPlayerView() const {
        return playerView;
    }

    sf::View& getMiniMapView() const {
        return miniMapView;
    }

    Player& getPlayer() {
        return player;
    }

    bool renderInventory() const {
        return displayInventory;
    }

  private:
    WindowManager& windowManager;
    AudioManager& audioManager;
    Player player;
    sf::View& playerView;
    sf::View& miniMapView;
    bool facingLeft = false;
    bool keyPressed = false;
    bool displayInventory = true;

    int counter = 0;
};