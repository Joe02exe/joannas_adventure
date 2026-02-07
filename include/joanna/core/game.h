#include "joanna/core/renderengine.h"
#include "joanna/core/windowmanager.h"
#include "joanna/systems/controller.h"
#include "joanna/systems/font_renderer.h"
#include "joanna/systems/menu.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/world/tilemanager.h"
#include "joanna/game/combat/combat_system.h"
#include "joanna/core/postprocessing.h"
#include "joanna/systems/audiomanager.h"
#include "joanna/entities/enemy.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <list>

class Game {
  public:
    Game();
    void run();

  private:
    void initialize();
    void handleInput();
    void update(float dt);
    void render(float dt);

    void updateOverworld(float dt);
    void updateCombat(float dt);
    void renderOverworld(float dt);
    void renderCombat();

    static void resize(
        sf::Vector2u size, float targetAspectRatio, sf::View& camera,
        sf::RenderWindow& window, PostProcessing& postProc
    );

    // Systems
    WindowManager windowManager;
    AudioManager audioManager;
    TileManager tileManager;
    RenderEngine renderEngine;
    CombatSystem combatSystem;
    PostProcessing postProc;
    FontRenderer fontRenderer;

    // Game State
    std::unique_ptr<Controller> controller; // our controller needs windowManager, so ptr for flexible init
    std::unique_ptr<Menu> menu; // our menu needs references, so ptr
    
    MusicId currentMusicId = MusicId::Overworld;
    GameStatus gameStatus = GameStatus::Overworld;
    sf::Clock clock;

    std::list<std::unique_ptr<Entity>> entities;
    std::shared_ptr<DialogueBox> sharedDialogueBox;
    
    // pointers to specific enemies for logic tracking
    Enemy* enemyPtr = nullptr;
    Enemy* skeletonPtr = nullptr;
    Enemy* randomSkeletonPtr = nullptr;
    
    // Config
    const float FIXED_TIMESTEP = 0.016f;
};