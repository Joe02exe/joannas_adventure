#pragma once
#include "string"

#include "../include/tileson.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Window.hpp>

class TileManager {
  public:
    TileManager();
    bool loadMap(const std::string& path, sf::RenderWindow& window);
    sf::Sprite*
    storeAndLoadImage(const std::string& image, const sf::Vector2f& position);

  private:
    tson::Tileson tsonParser;
    tson::Map* map = nullptr;
    std::map<std::string, std::unique_ptr<sf::Texture>> m_textures;
    std::map<std::string, std::unique_ptr<sf::Sprite>> m_sprites;
};