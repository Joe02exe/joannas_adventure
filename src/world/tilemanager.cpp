#include "joanna/world/tilemanager.h"
#include "joanna/entities/player.h"
#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"
#include "spdlog/spdlog.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <random>
#include <string>

TileManager::TileManager(sf::RenderWindow& renderWindow)
    : window(&renderWindow), tsonParser(), m_currentMap(nullptr), m_textures(),
      m_tiles() {
    if (!loadMap("./assets/environment/map/newmap.json")) {
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
    renderProgressBar("Start initializing...");
    processLayer("background");
    progress = 0.2f;
    renderProgressBar("Loading map...");
    processLayer("ground");
    progress = 0.4f;
    renderProgressBar("Loading map...");
    processLayer("decorations");
    progress = 0.6f;
    renderProgressBar("Loading assets...");
    processLayer("decoration_overlay");
    progress = 0.8f;
    renderProgressBar("Loading collision items...");
    processLayer("overlay");
    progress = 0.9f;
    renderProgressBar("Loading items...");
    processLayer("items");

    // Sort all collidable tiles by bottom y + offset
    std::stable_sort(
        m_collidables.begin(), m_collidables.end(),
        [](const TileRenderInfo& a, const TileRenderInfo& b) {
            return a.position.y < b.position.y;
        }
    );

    return true;
}

bool TileManager::checkLineOfSight(sf::Vector2f start, sf::Vector2f end, float stepSize) const {
    const sf::Vector2f direction = end - start;
    const float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (distance <= stepSize) return true; // too close to have obstacles
    
    const sf::Vector2f step = (direction / distance) * stepSize;
    const int steps = static_cast<int>(distance / stepSize);
    
    sf::Vector2f current = start;
    for (int i = 0; i < steps; ++i) {
        current += step;
        for (const auto& rect : m_collisionRects) {
            if (rect.contains(current)) {
                return false; // Obstacle found
            }
        }
    }
    
    return true; // No obstacles found
}

void TileManager::renderProgressBar(std::string message) const {
    auto center = window->getView().getCenter();

    sf::Vector2f barSize(400.f, 40.f);
    sf::Vector2f barPos(center.x - 200, center.y - 20);

    sf::RectangleShape barBackground(barSize);
    barBackground.setPosition(barPos);
    barBackground.setFillColor(sf::Color(50, 50, 50)); // Dark Grey
    barBackground.setOutlineThickness(2.f);
    barBackground.setOutlineColor(sf::Color::White);

    sf::RectangleShape barProgress(barSize);
    barProgress.setPosition(barPos);
    barProgress.setFillColor(sf::Color::Green);

    float percent = progress / 1.0f;
    float currentWidth = barSize.x * percent;

    // Update the size of the progress bar
    barProgress.setSize(sf::Vector2f(currentWidth, barSize.y));

    sf::Font font = ResourceManager<sf::Font>::getInstance()->get(
        "assets/font/minecraft.ttf"
    );
    sf::Text text(font);
    sf::Text title(font);
    text.setString(message.empty() ? "Loading..." : message);
    title.setString("Joanna's Adventure");
    text.setCharacterSize(18);
    title.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect textBounds = text.getLocalBounds();
    sf::FloatRect titleBounds = title.getLocalBounds();
    text.setOrigin({textBounds.size.x / 2.f, textBounds.size.y / 2.f});
    text.setPosition(
        {center.x, center.y + 40.f}
    );

    title.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
    title.setPosition(
        {center.x, center.y - 75.f}
    );

    window->clear(sf::Color::Black);

    window->draw(barBackground);
    window->draw(barProgress);
    window->draw(text);
    window->draw(title);

    window->display();
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
            sf::Color c = img.getPixel(
                { static_cast<unsigned int>(texRect.position.x + x),
                  static_cast<unsigned int>(texRect.position.y + y) }
            );
            if (c.a > 0) { // if pixel is not transparent
                hasOpaque = true;
                if (x < left) {
                    left = x;
                }
                if (x > right) {
                    right = x;
                }
                if (y < top) {
                    top = y;
                }
                if (y > bottom) {
                    bottom = y;
                }
            }
        }
    }

    if (!hasOpaque) {
        return { { 0.f, 0.f }, { 0.f, 0.f } };
    }

    // Translate local pixel bounds to world position
    return { { pos.x + static_cast<float>(left),
               pos.y + static_cast<float>(top) },
             { static_cast<float>(right - left + 1),
               static_cast<float>(bottom - top + 1) } };
}

void TileManager::randomlySelectItems(
    std::vector<tson::Object*> items, int count
) {
    static std::random_device rd;
    static std::mt19937 g(rd());

    std::shuffle(items.begin(), items.end(), g);

    int countToSpawn = std::min((int)items.size(), count);

    for (int i = 0; i < countToSpawn; ++i) {
        tson::Object* obj = items[i]; // Get the pointer back

        RenderObject object(
            obj->getId(), obj->getGid(),
            { obj->getPosition().x, obj->getPosition().y },
            getTextureById(static_cast<int>(obj->getGid()))
        );
        m_objects.push_back(object);
    }
}

void TileManager::processLayer(const std::string& layerName) {
    if (!m_currentMap) {
        return;
    }

    tson::Layer* layer = m_currentMap->getLayer(layerName);
    if (layer->getType() == tson::LayerType::ObjectGroup) {

        // TODO : refactor this if more items are needed
        std::vector<tson::Object*> carrots;
        std::vector<tson::Object*> swords;

        for (auto& obj : layer->getObjects()) {
            int currentGid = static_cast<int>(obj.getGid());

            if (currentGid == 691) {
                carrots.push_back(&obj);
            }
            if (currentGid == 3050) {
                swords.push_back(&obj);
            }
        }

        randomlySelectItems(carrots, 7);
        randomlySelectItems(swords, 1);
    }

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
                while (const std::optional<sf::Event> event =
                           window->pollEvent()) {
                    if (event->is<sf::Event::Closed>()) {
                        window->close();
                    }
                }
                // Logger::info(
                //     "Added collision rect at ({}, {})", pixelRect.position.x,
                //     pixelRect.position.y
                // );
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
    m_objects.clear();
    m_collidables.clear();
    m_textures.clear();
    m_collisionRects.clear();
    m_currentMap.reset();
}

sf::Sprite TileManager::getTextureById(const int id) {
    const sf::Texture& texture =
        ResourceManager<sf::Texture>::getInstance()->get(
            "assets/environment/map/tileset.png"
        );

    const int TILE_W = 16;
    const int TILE_H = 16;

    const int tilesPerRow = static_cast<int>(texture.getSize().x / TILE_W);

    int tx = (id % tilesPerRow) * TILE_W;
    int ty = (id / tilesPerRow) * TILE_H;

    // sprite from tileset
    sf::Sprite icon(texture);
    icon.setTextureRect(sf::IntRect({ tx, ty }, { TILE_W, TILE_H }));
    return icon;
}

bool TileManager::removeObjectById(int id) {
    auto before = m_objects.size();

    m_objects.erase(
        std::remove_if(
            m_objects.begin(), m_objects.end(),
            [id](const RenderObject& obj) { return obj.id == id; }
        ),
        m_objects.end()
    );

    return m_objects.size() < before; // true = something was removed
}

void TileManager::loadObjectsFromSaveGame(
    const std::vector<ObjectState>& objects
) {
    m_objects.clear();
    for (const auto& objState : objects) {
        RenderObject object(
            objState.id, objState.gid, { objState.x, objState.y },
            getTextureById(static_cast<int>(objState.gid))
        );
        m_objects.push_back(object);
    }
}

void TileManager::reloadObjectsFromTileson() {
    m_objects.clear();
    processLayer("items");
}