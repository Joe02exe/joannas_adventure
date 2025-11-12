#include "joanna/world/tilemanager.h"
#include "joanna/entities/player.h"
#include "joanna/utils/logger.h"
#include "spdlog/spdlog.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

TileManager::TileManager()
    : tsonParser(), m_currentMap(nullptr), m_textures(), m_tiles() {
    if (!loadMap("./assets/environment/map/map_village3.json")) {
        Logger::error("Failed to load map!");
    }
    Logger::info("Map loaded successfully");
}

bool TileManager::loadMap(const std::string& path) {
    std::unique_ptr<tson::Map> map = tsonParser.parse(fs::path(path));
    if (map->getStatus() != tson::ParseStatus::OK) {
        Logger::error(
            "Failed to load map: {}", static_cast<int>(map->getStatus())
        );
        return false;
    }

    m_currentMap = std::move(map);

    // Process all tile layers and prepare rendering data
    processLayer("background");
    processLayer("ground");
    processLayer("decorations");
    processLayer("decoration_overlay");
    processLayer("overlay");

    // Sort all collidable tiles by bottom y + offset
    std::stable_sort(
        m_collidables.begin(), m_collidables.end(),
        [](const TileRenderInfo& a, const TileRenderInfo& b) {
            return a.position.y < b.position.y;
        }
    );

    return true;
}

// rendering renderEngine
// maps
// ogg for sound SountBufferManager
// texturemanager as singleton

sf::FloatRect calculatePixelRect(
    const sf::Texture& tex, const sf::IntRect& texRect, const sf::Vector2f& pos
) {
    sf::Image img = tex.copyToImage();

    int left = texRect.size.x;
    int right = 0;
    int top = texRect.size.y;
    int bottom = 0;
    bool hasOpaque = false;

    for (int y = 0; y < texRect.size.y; ++y) {
        for (int x = 0; x < texRect.size.x; ++x) {
            sf::Color c = img.getPixel({ texRect.position.x + x,
                                         texRect.position.y + y });
            if (c.a > 0) { // if pixel is not transparent
                hasOpaque = true;
                if (x < left)
                    left = x;
                if (x > right)
                    right = x;
                if (y < top)
                    top = y;
                if (y > bottom)
                    bottom = y;
            }
        }
    }

    if (!hasOpaque) {
        return sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
    }

    // Translate local pixel bounds to world position
    return sf::FloatRect(
        { pos.x + static_cast<float>(left), pos.y + static_cast<float>(top) },
        { static_cast<float>(right - left + 1),
          static_cast<float>(bottom - top + 1) }
    );
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
        const tson::Tileset* tileset = tileObject.getTile()->getTileset();
        const tson::Rect drawingRect = tileObject.getDrawingRect();
        const tson::Vector2f position = tileObject.getPosition();

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

        if (layerName == "decorations" || layerName == "decoration_overlay") {
            sf::FloatRect pixelRect = calculatePixelRect(
                *m_textures[imagePath], info.textureRect, info.position
            );
            if (pixelRect.size.x > 0.f && pixelRect.size.y > 0.f &&
                isCollidable) {
                m_collisionRects.push_back(pixelRect);
                Logger::info(
                    "Added collision rect at ({}, {})", pixelRect.position.x,
                    pixelRect.position.y
                );
            }
            info.collisionBox = pixelRect;
            m_collidables.push_back(info);
        } else if (layerName == "overlay") {
            m_overlayTiles.push_back(info);
        } else {
            m_tiles.push_back(info);
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
        Logger::error("Could not find texture: {}", path.generic_string());
        return;
    }

    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(path.generic_string())) {
        Logger::error("Failed to load texture: {}", path.generic_string());
        return;
    }

    m_textures[imagePath] = std::move(tex);
}

void TileManager::clear() {
    m_tiles.clear();
    m_collidables.clear();
    m_textures.clear();
    m_collisionRects.clear();
    m_currentMap.reset();
}