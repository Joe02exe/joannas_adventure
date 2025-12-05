#pragma once

#include "extern/tileson.hpp"
#include "joanna/entities/player.h"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct TileRenderInfo {
    std::string texturePath;
    sf::IntRect textureRect;
    sf::Vector2f position;
    std::optional<sf::FloatRect> collisionBox;
};

class TileManager {
  public:
    TileManager();

    bool loadMap(const std::string& path);
    // void render(sf::RenderTarget& target, Player& player);
    void clear();

    std::vector<sf::FloatRect>& getCollisionRects() {
        return m_collisionRects;
    }

    [[nodiscard]] const std::map<std::string, std::unique_ptr<sf::Texture>>&
    getGroundTextures() const {
        return m_textures;
    }

    [[nodiscard]] const std::vector<TileRenderInfo>& getTiles() const {
        return m_tiles;
    }

    [[nodiscard]] const std::vector<TileRenderInfo>&
    getCollidableTiles() const {
        return m_collidables;
    }

    [[nodiscard]] const std::vector<TileRenderInfo>& getOverlayTiles() const {
        return m_overlayTiles;
    }

    sf::Sprite getTextureById(int id);

  private:
    void processLayer(const std::string& layerName);
    void loadTexture(const std::string& imagePath);

    tson::Tileson tsonParser;
    std::vector<sf::FloatRect> m_collisionRects;
    std::unique_ptr<tson::Map> m_currentMap = nullptr;
    ;
    std::map<std::string, std::unique_ptr<sf::Texture>> m_textures;
    std::vector<TileRenderInfo> m_tiles;
    std::vector<TileRenderInfo> m_collidables;
    std::vector<TileRenderInfo> m_overlayTiles;
};