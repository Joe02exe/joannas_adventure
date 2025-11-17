#include "joanna/utils/filesystem.h"
#include "joanna/utils/logger.h"

namespace fs {

sf::Texture getTextureFromPath(const std::string& path) {
    sf::Texture texture;
    Logger::info("sssLoading texture from path: {}", path);
    const std::string fullPath = "assets/" + path;
    if (!texture.loadFromFile(fullPath)) {
        throw std::runtime_error(
            "Failed to load texture from path: " + fullPath
        );
    }
    return texture;
}
} // namespace fs