#include "joanna/core/renderengine.h"
#include "joanna/utils/logger.h"

RenderEngine::RenderEngine() = default;

void RenderEngine::render(
    sf::RenderTarget& target, Player& player, TileManager& tileManager,
    std::list<std::unique_ptr<Interactable>>& interactables,
    std::shared_ptr<DialogueBox>& dialogueBox
) {
    const auto& m_textures = tileManager.getGroundTextures();
    const auto& m_tiles = tileManager.getTiles();
    const auto& m_collidables = tileManager.getCollidableTiles();
    const auto& m_overlayTiles = tileManager.getOverlayTiles();
    const auto& m_objects = tileManager.getRenderObjects();

    auto drawTile = [&](const TileRenderInfo& tile) {
        auto it = m_textures.find(tile.texturePath);
        if (it != m_textures.end()) {
            sf::Sprite sprite(*it->second);
            sprite.setTextureRect(tile.textureRect);
            sprite.setPosition(tile.position);
            target.draw(sprite);
        }
    };

    // draw background and ground tiles
    for (const auto& tile : m_tiles) {
        drawTile(tile);
    }

    // draw collidable/decorative tiles with player sorting
    float playerBottom = player.getCollisionBox().value().position.y;
    bool playerDrawn = false;

    // draw iteractables below player
    for (auto& entity : interactables) {
        if (entity->getCollisionBox().has_value()) {
            float middleEntity = entity->getCollisionBox().value().position.y;
            if (middleEntity < playerBottom) {
                entity->render(target);
            }
        } else {
            entity->render(target);
        }
    }

    // draw collidables
    for (const auto& tile : m_collidables) {
        float middleTile = tile.collisionBox.value().position.y;
        if (!playerDrawn && middleTile >= playerBottom) {
            player.draw(target);
            playerDrawn = true;
        }
        drawTile(tile);
    }

    // draw interactables above player
    for (auto& entity : interactables) {
        if (entity->getCollisionBox().has_value()) {
            float middleEntity = entity->getCollisionBox().value().position.y;
            if (middleEntity >= playerBottom) {
                entity->render(target);
            }
        }
    }

    // If the player is still not drawn (player above all tiles)
    if (!playerDrawn) {
        player.draw(target);
    }

    // render interaction buttons once
    for (auto& entity : interactables) {
        if (entity->canPlayerInteract(player.getPosition())) {
            entity->renderButton(target);
        }
    }

    // draw overlay tiles once
    for (const auto& tile : m_overlayTiles) {
        drawTile(tile);
    }

    if (dialogueBox->isActive()) {
        dialogueBox->render(target);
    }

    for (const auto& item : m_objects) {
        sf::Sprite i = tileManager.getTextureById(static_cast<int>(item.gid));
        i.setPosition(sf::Vector2f({ static_cast<float>(item.position.x),
                                     static_cast<float>(item.position.y) +
                                         offset }));
        if (offset > 5.f) {
            dir = -1.f;
        }
        if (offset < 0.f) {
            dir = 1.f;
        }
        offset += dir * 0.05f;
        target.draw(i);

        auto playerPos = player.getPosition();
        auto itemPos = item.position;
        float dx = playerPos.x - static_cast<float>(itemPos.x);
        float dy = playerPos.y - static_cast<float>(itemPos.y);
        if (dx * dx + dy * dy <= 16.f * 16.f) {
            sf::Sprite indicator = tileManager.getTextureById(2919);
            indicator.setPosition(
                sf::Vector2f({ static_cast<float>(item.position.x),
                               static_cast<float>(item.position.y) - 10.f })
            );
            target.draw(indicator);
        }
    }
}