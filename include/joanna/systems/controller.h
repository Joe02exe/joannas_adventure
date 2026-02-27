#pragma once

#include "audiomanager.h"
#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"

#include "joanna/core/graphics.h"
#include <list>

class Game;

class Controller {
  public:
    Controller(
        WindowManager& windowManager, AudioManager& audioManager, Game& game
    );

    bool getInput(
        float dt, jo::RenderWindow& window,
        const std::vector<jo::FloatRect>& collisions,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& sharedDialogueBox,
        TileManager& tileManager, RenderEngine& renderEngine
    );

    bool updateStep(
        float dt, jo::RenderWindow& window,
        std::vector<jo::FloatRect>& collisions,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& sharedDialogueBox,
        TileManager& tileManager, RenderEngine& renderEngine

    );

    bool isMapOverviewActive() const {
        return showMapOverview;
    }

    jo::View& getPlayerView() const {
        return playerView;
    }

    jo::View& getMiniMapView() const {
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
    Game& game;
    Player player;
    jo::View& playerView;
    jo::View& miniMapView;
    bool facingLeft = false;
    bool keyPressed = false;
    bool displayInventory = true;

    bool showMapOverview = false;
    bool mPressed = false;

    int counter = 0;
};
