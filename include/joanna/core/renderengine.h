#pragma once

#include "joanna/core/graphics.h"
#include "joanna/entities/enemy.h"
#include "joanna/entities/interactable.h"
#include "joanna/entities/player.h"
#include "joanna/utils/dialogue_box.h"
#include "joanna/world/tilemanager.h"
#include <list>
#include <optional>

class RenderEngine {
  public:
    RenderEngine();

    void render(
        jo::RenderTarget& target, Player& player, TileManager& tileManager,
        std::list<std::unique_ptr<Entity>>& entities,
        const std::shared_ptr<DialogueBox>& dialogueBox, float dt
    );

  private:
    float offset = 0.f;
    float dir = 1.f;
    float frameTimer = 0.f;

    // Cached indicator sprite (tile ID 2919) — initialized on first render call
    bool m_indicatorCached = false;
    std::optional<jo::Sprite> m_indicatorSprite;
};
