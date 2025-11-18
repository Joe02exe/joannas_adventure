#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace fs {
sf::Texture getTextureFromPath(const std::string& path);
}