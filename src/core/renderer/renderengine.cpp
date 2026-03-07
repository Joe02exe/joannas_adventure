#include "joanna/core/renderengine.h"
#include "joanna/entities/interactables/chest.h"
#include "joanna/entities/interactables/stone.h"

RenderEngine::RenderEngine() = default;

void RenderEngine::render(
    jo::RenderTarget& target, Player& player, TileManager& tileManager,
    std::list<std::unique_ptr<Entity>>& entities,
    const std::shared_ptr<DialogueBox>& dialogueBox, float dt
) {
    auto view = target.getView();
    auto center = view.getCenter();
    auto size = view.getSize();
    // Expand the view rect slightly to prevent pop-in (tightened to 16px to
    // save rendering overhead)
    jo::FloatRect viewRect(
        { center.x - size.x / 2.f - 16.f, center.y - size.y / 2.f - 16.f },
        { size.x + 32.f, size.y + 32.f }
    );

    auto drawTile = [&](const TileRenderInfo& tile) {
        if (tile.texture &&
#ifdef MIYOO_BUILD
            viewRect.intersects(jo::FloatRect(tile.position, { 16.f, 16.f }))) {
#else
            viewRect
                .findIntersection(jo::FloatRect(tile.position, { 16.f, 16.f }))
                .has_value()) {
#endif
            jo::Sprite sprite(*tile.texture);
            sprite.setTextureRect(tile.textureRect);
            sprite.setPosition(tile.position);
            target.draw(sprite);
        }
    };

    // draw background and ground tiles
    for (const auto& tile : tileManager.getTiles()) {
        drawTile(tile);
    }

    // Pre-compute player bottom for depth sorting
    float playerBottom = 0.f;
    if (player.getCollisionBox().has_value()) {
        playerBottom = player.getCollisionBox().value().position.y +
                       player.getCollisionBox().value().size.y + 2.f;
    }
    bool playerDrawn = false;

    // --- Single-pass entity classification ---
    // Classify each visible entity once to avoid repeated dynamic_cast calls.
    struct EntityInfo {
        Entity* ptr;
        bool isStone;
        Interactable* interactable; // nullptr if not interactable
        float bottom;               // bottom of collision box, or 0
    };

    std::vector<EntityInfo> visibleEntities;
    visibleEntities.reserve(entities.size());

    for (auto& entity : entities) {
        if (!viewRect.contains(entity->getPosition()))
            continue;
        EntityInfo info;
        info.ptr = entity.get();
        info.isStone = (dynamic_cast<Stone*>(entity.get()) != nullptr);
        info.interactable = dynamic_cast<Interactable*>(entity.get());
        if (entity->getCollisionBox().has_value()) {
            // Use collision box bottom for depth sorting
            info.bottom = entity->getCollisionBox().value().position.y +
                          entity->getCollisionBox().value().size.y;
        } else {
            // No collision box: use sprite position Y as depth reference.
            // We add a fixed offset (16px) to approximate foot-level.
            info.bottom = entity->getPosition().y + 16.f;
        }
        visibleEntities.push_back(info);
    }

    // Draw stones first (they are always behind everything)
    for (auto& info : visibleEntities) {
        if (info.isStone) {
            info.ptr->render(target);
        }
    }

    // Draw entities below the player (foot-position y-sort)
    for (auto& info : visibleEntities) {
        if (info.isStone)
            continue;
        if (info.bottom <= playerBottom) {
            info.ptr->render(target);
        }
    }

    // Draw collidable/decorative tiles with player sorting
    for (const auto& tile : tileManager.getCollidableTiles()) {
        float middleTile = 0.f;
        if (tile.collisionBox.has_value()) {
            middleTile = tile.collisionBox.value().position.y +
                         tile.collisionBox.value().size.y - 2.f;
        }
        if (!playerDrawn && middleTile >= playerBottom) {
            player.draw(target);
            playerDrawn = true;
        }
        drawTile(tile);
    }

    // Draw entities above the player (larger bottom y = further down = in front
    // of player)
    for (auto& info : visibleEntities) {
        if (info.isStone)
            continue;
        if (info.bottom > playerBottom) {
            info.ptr->render(target);
        }
    }

    // If the player is still not drawn (player above all tiles)
    if (!playerDrawn) {
        player.draw(target);
    }

    // Draw interact buttons for nearby interactables
    for (auto& info : visibleEntities) {
        if (info.interactable == nullptr)
            continue;
        if (info.isStone && !player.getInventory().hasItemByName("pickaxe")) {
            continue;
        }
        if (info.interactable->canPlayerInteract(player.getPosition())) {
            if (auto* chest = dynamic_cast<Chest*>(info.ptr)) {
                if (chest->isChestOpen())
                    continue;
            }
            info.interactable->renderButton(target);
        }
    }

    // draw overlay tiles
    for (const auto& tile : tileManager.getOverlayTiles()) {
        drawTile(tile);
    }

    // Cache the pick-up indicator sprite once
    if (!m_indicatorCached) {
        m_indicatorSprite = tileManager.getTextureById(2919);
        m_indicatorCached = true;
    }

    // draw objects with floating effect
    frameTimer += dt;
    auto playerPos = player.getPosition();
    for (const auto& item : tileManager.getRenderObjects()) {
        // Simple culling for objects too
        if (!viewRect.contains(jo::Vector2f(item.position))) {
            continue;
        }

        jo::Sprite sprite = item.sprite;
        sprite.setPosition(jo::Vector2f({ static_cast<float>(item.position.x),
                                          static_cast<float>(item.position.y) +
                                              offset }));
        if (offset > 5.f) {
            dir = -1.f;
        }
        if (offset < 0.f) {
            dir = 1.f;
        }
        if (frameTimer > 0.08f) {
            frameTimer = 0.f;
            offset += dir * 0.5f;
        }
        target.draw(sprite);

        float dx = playerPos.x - static_cast<float>(item.position.x) - 8.f;
        float dy = playerPos.y - static_cast<float>(item.position.y) - 8.f;
        if (dx * dx + dy * dy <= 16.f * 16.f && m_indicatorSprite.has_value()) {
            m_indicatorSprite->setPosition(
                jo::Vector2f({ static_cast<float>(item.position.x),
                               static_cast<float>(item.position.y) - 10.f })
            );
            target.draw(*m_indicatorSprite);
        }
    }
}