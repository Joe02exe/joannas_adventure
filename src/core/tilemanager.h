#pragma once

#include "../include/tileson.hpp"
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
};

class TileManager {
  public:
    TileManager();

    bool loadMap(const std::string& path);
    void render(sf::RenderTarget& target);
    void clear();

  private:
    void processLayer(const std::string& layerName);
    void loadTexture(const std::string& imagePath);

    tson::Tileson tsonParser{};
    std::unique_ptr<tson::Map> m_currentMap = nullptr;
    std::map<std::string, std::unique_ptr<sf::Texture>> m_textures{};
    std::vector<TileRenderInfo> m_tiles{};
};