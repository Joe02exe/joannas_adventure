#pragma once

#include "../entities/player/player.h"
#include "tileson.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
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
    void render(sf::RenderTarget& target, Player& player);
    void clear();

    const std::vector<sf::FloatRect>& getCollisionRects() const {
        return m_collisionRects;
    }

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