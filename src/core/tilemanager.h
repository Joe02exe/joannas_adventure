#pragma once

#include "tileson.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct TileRenderInfo {
    std::string texturePath;
    sf::IntRect textureRect;
    sf::Vector2f position;
};

class TileManager {
  public:
    TileManager();

    // Load a map from file
    bool loadMap(const std::string& path);

    // Render all tiles to the window
    void render(sf::RenderWindow& window);

    // Clear all loaded map data
    void clear();

  private:
    void processLayer(const std::string& layerName);
    void loadTexture(const std::string& imagePath);

    tson::Tileson tsonParser;
    std::unique_ptr<tson::Map> m_currentMap;
    std::map<std::string, std::unique_ptr<sf::Texture>> m_textures;
    std::vector<TileRenderInfo> m_tiles;
};