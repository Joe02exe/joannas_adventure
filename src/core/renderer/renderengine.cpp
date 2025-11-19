#include "joanna/core/renderengine.h"
#include "joanna/utils/logger.h"

RenderEngine::RenderEngine() {}

void RenderEngine::render(
    sf::RenderTarget& target, Player& player, TileManager& tileManager,
    std::list<std::unique_ptr<Interactable>>& interactables, std::shared_ptr<DialogueBox> dialogueBox
) {
    const auto& m_textures = tileManager.getGroundTextures();
    const auto& m_tiles = tileManager.getTiles();
    const auto& m_collidables = tileManager.getCollidableTiles();
    const auto& m_overlayTiles = tileManager.getOverlayTiles();

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
    float playerBottom = player.getPosition().y + 32.f;
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

        // If the player is still not drawn (player above all tiles)
        if (!playerDrawn) {
            Logger::info("Player drawn last in tile rendering");
            player.draw(target);
        }

        for (auto& entity : interactables) {
            if (entity->canPlayerInteract(player.getPosition())) {
                entity->renderButton(target);
            }
        }

        // draw overlay tiles
        for (const auto& tile : m_overlayTiles) {
            drawTile(tile);
        }
    }

    if (dialogueBox->isActive()) {
        dialogueBox->render(target);
    }
}
