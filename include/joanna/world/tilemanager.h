#pragma once

#include "extern/tileson.hpp"
#include "joanna/core/graphics.h"
#include "joanna/entities/player.h"
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

struct TileRenderInfo {
    const jo::Texture* texture = nullptr;
    jo::IntRect textureRect;
    jo::Vector2f position;
    std::optional<jo::FloatRect> collisionBox;
};

struct RenderObject {
    uint32_t id = 0;
    uint32_t gid = 0;
    jo::Vector2i position;
    jo::Sprite sprite; // renamed from 'texture' for clarity

    // Explicit default constructor
    RenderObject(
        const uint32_t id, uint32_t gid, const jo::Vector2i pos,
        jo::Sprite sprite
    )
        : id(id), gid(gid), position(pos), sprite(std::move(sprite)) {
        // jo::Sprite is automatically default constructed here
    }
};

class TileManager {
  public:
    TileManager(jo::RenderWindow& window);

    bool loadMap(const std::string& path);
    [[nodiscard]] bool checkLineOfSight(
        jo::Vector2f start, jo::Vector2f end, float stepSize = 10.f
    ) const;
    // void render(jo::RenderTarget& target, Player& player);
    void clear();

    std::vector<jo::FloatRect>& getCollisionRects() {
        return m_collisionRects;
    }

    [[nodiscard]] const std::map<std::string, std::unique_ptr<jo::Texture>>&
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

    [[nodiscard]] const std::vector<RenderObject>& getRenderObjects() const {
        return m_objects;
    }

    [[nodiscard]] const std::vector<TileRenderInfo>& getOverlayTiles() const {
        return m_overlayTiles;
    }

    jo::Sprite getTextureById(int id);

    bool removeObjectById(int id);

    void loadObjectsFromSaveGame(const std::vector<ObjectState>& objects);

    void reloadObjectsFromTileson();

  private:
    void randomlySelectItems(std::vector<tson::Object*> carrots, int count);

    void processLayer(const std::string& layerName);
    void loadTexture(const std::string& imagePath);

    void renderProgressBar(const std::string& message) const;

    float progress = 0.0f;
    jo::RenderWindow* window;
    tson::Tileson tsonParser;
    std::vector<jo::FloatRect> m_collisionRects;
    std::unique_ptr<tson::Map> m_currentMap = nullptr;
    ;
    std::map<std::string, std::unique_ptr<jo::Texture>> m_textures;
    std::vector<TileRenderInfo> m_tiles;
    std::vector<TileRenderInfo> m_collidables;
    std::vector<TileRenderInfo> m_overlayTiles;
    std::vector<RenderObject> m_objects;
};