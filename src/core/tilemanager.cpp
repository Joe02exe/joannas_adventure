#include "./tilemanager.h"
#include "logger.h"
#include "spdlog/spdlog.h"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

TileManager::TileManager() = default;

bool TileManager::loadMap(const std::string& path) {
    std::unique_ptr<tson::Map> map = tsonParser.parse(fs::path(path));
    if (map->getStatus() != tson::ParseStatus::OK) {
        spdlog::error(
            "Failed to load map: {}", static_cast<int>(map->getStatus())
        );
        return false;
    }

    m_currentMap = std::move(map);

    // Process all tile layers and prepare rendering data
    processLayer("background");
    processLayer("ground");
    processLayer("decorations");

    return true;
}

void TileManager::processLayer(const std::string& layerName) {
    if (!m_currentMap)
        return;

    tson::Layer* layer = m_currentMap->getLayer(layerName);
    if (!layer || layer->getType() != tson::LayerType::TileLayer) {
        return;
    }

    const bool isCollidable = layer->get<bool>("collidable");

    for (auto& [pos, tileObject] : layer->getTileObjects()) {
        tson::Tileset* tileset = tileObject.getTile()->getTileset();
        tson::Rect drawingRect = tileObject.getDrawingRect();
        tson::Vector2f position = tileObject.getPosition();

        // Load texture if not already loaded
        std::string imagePath = tileset->getImage().u8string();
        loadTexture(imagePath);

        // Store tile rendering info
        TileRenderInfo info;
        info.texturePath = imagePath;
        info.textureRect = sf::IntRect(
            { drawingRect.x, drawingRect.y },
            { drawingRect.width, drawingRect.height }
        );
        info.position = sf::Vector2f(position.x, position.y);

        m_tiles.push_back(info);

        if (isCollidable) {
            // log this event
            Logger::info(
                "Collidable tile found at: ({}, {})", position.x, position.y
            );

            m_collisionRects.emplace_back(sf::FloatRect(
                { position.x, position.y },
                { static_cast<float>(drawingRect.width),
                  static_cast<float>(drawingRect.height) }
            ));
        }
    }
}

void TileManager::loadTexture(const std::string& imagePath) {
    // Check if texture is already loaded
    if (m_textures.count(imagePath) > 0) {
        return;
    }

    fs::path path = imagePath;
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        spdlog::error("Could not find texture: {}", path.generic_string());
        return;
    }

    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(path.generic_string())) {
        spdlog::error("Failed to load texture: {}", path.generic_string());
        return;
    }

    m_textures[imagePath] = std::move(tex);
}

void TileManager::render(sf::RenderTarget& target) {
    for (const auto& tile : m_tiles) {
        auto it = m_textures.find(tile.texturePath);
        if (it != m_textures.end()) {
            sf::Sprite sprite(*it->second);
            sprite.setTextureRect(tile.textureRect);
            sprite.setPosition(tile.position);
            target.draw(sprite);
        }
    }
}

void TileManager::clear() {
    m_tiles.clear();
    m_textures.clear();
    m_collisionRects.clear();
    m_currentMap.reset();
}