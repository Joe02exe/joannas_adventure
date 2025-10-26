#include "./tilemanager.h"

#include "spdlog/spdlog.h"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Window.hpp>

TileManager::TileManager() = default;

bool TileManager::loadMap(const std::string& path, sf::RenderWindow& window) {
    std::unique_ptr<tson::Map> map = tsonParser.parse(fs::path(path));
    if (map->getStatus() == tson::ParseStatus::OK) {
        // Gets the layer called "Object Layer" from the "ultimate_demo.json map
        tson::Layer* objectLayer =
            map->getLayer("background"); // This is an Object Layer

        // Example from an Object Layer.
        if (objectLayer->getType() == tson::LayerType::ObjectGroup) {
            tson::Object* goomba = objectLayer->firstObj("background"
            ); // Gets the first object with this name. This can be any object.

            // If you want to just go through every existing object in the
            // layer:
            for (auto& obj : objectLayer->getObjects()) {
                tson::Vector2i position = obj.getPosition();
                tson::Vector2i size = obj.getSize();
                tson::ObjectType objType = obj.getObjectType();

                // You may want to check the object type to make sure you use
                // the data right.
            }

            tson::ObjectType objType = goomba->getObjectType();

            /*!
             * tson::ObjectType is defined like this.
             * They are automatically detected based on what kind of object you
             have created
             * enum class Type : uint8_t
                {
                    Undefined = 0,
                    Object = 1,
                    Ellipse = 2, //<-- Circle
                    Rectangle = 3,
                    Point = 4,
                    Polygon = 5,
                    Polyline = 6,
                    Text = 7,
                    Template = 8
                };
             */

            if (objType == tson::ObjectType::Rectangle) {
                tson::Vector2i size = goomba->getSize();
                tson::Vector2i position = goomba->getPosition();

                // If you have set a custom property, you can also get this
                // int hp = goomba->get<int>("hp");

                // Using size and position you can can create a Rectangle object
                // by your library of choice. An example if you were about to
                // use SFML for drawing: sf::RectangleShape rect;
                // rect.setSize(sf::Vector2f(size.x, size.y));
                // rect.setPosition(sf::Vector2f(position.x, position.y));
            } else if (objType == tson::ObjectType::Polygon) {
                for (auto const& poly : goomba->getPolygons()) {
                    // Set a point on a shape taking polygons
                }
                tson::Vector2i position = goomba->getPosition();
            } else if (objType == tson::ObjectType::Polyline) {
                std::vector<tson::Vector2i> polys = goomba->getPolylines();
                for (auto const& poly : goomba->getPolylines()) {
                }
                tson::Vector2i position = goomba->getPosition();
            }
        }

        tson::Layer* tileLayer =
            map->getLayer("background"); // This is a Tile Layer.

        // You can get your tileset like this, but in v1.2.0
        // The tiles themselves holds a pointer to their related tileset.
        // tson::Tileset* tileset = map->getTileset("demo-tileset");

        // Example from a Tile Layer
        // I know for a fact that this is a Tile Layer, but you can check it
        // this way to be sure.
        if (tileLayer->getType() == tson::LayerType::TileLayer) {
            // pos = position in tile units
            for (auto& [pos, tileObject] : tileLayer->getTileObjects(
                 )) // Loops through absolutely all existing tileObjects
            {
                // spdlog::info("Tile\n");
                tson::Tileset* tileset = tileObject.getTile()->getTileset();
                tson::Rect drawingRect = tileObject.getDrawingRect();
                tson::Vector2f position = tileObject.getPosition();

                // Here you can determine the offset that should be set on a
                // sprite Example on how it would be done using SFML (where
                // sprite presumably is a member of a generated game object):
                sf::Sprite* sprite =
                    storeAndLoadImage(tileset->getImage().u8string(), { 0, 0 });
                if (sprite != nullptr) {
                    sprite->setTextureRect(sf::IntRect(
                        { drawingRect.x, drawingRect.y },
                        { drawingRect.width, drawingRect.height }
                    ));
                    sprite->setPosition({ position.x, position.y });
                    window.draw(*sprite);
                }
            }
        }

        tson::Layer* tileLayerG =
            map->getLayer("ground"); // This is a Tile Layer.

        // You can get your tileset like this, but in v1.2.0
        // The tiles themselves holds a pointer to their related tileset.
        // tson::Tileset* tileset = map->getTileset("demo-tileset");

        // Example from a Tile Layer
        // I know for a fact that this is a Tile Layer, but you can check it
        // this way to be sure.
        if (tileLayerG->getType() == tson::LayerType::TileLayer) {
            // pos = position in tile units
            for (auto& [pos, tileObject] : tileLayerG->getTileObjects(
                 )) // Loops through absolutely all existing tileObjects
            {
                // spdlog::info("Tile\n");
                tson::Tileset* tileset = tileObject.getTile()->getTileset();
                tson::Rect drawingRect = tileObject.getDrawingRect();
                tson::Vector2f position = tileObject.getPosition();

                // Here you can determine the offset that should be set on a
                // sprite Example on how it would be done using SFML (where
                // sprite presumably is a member of a generated game object):
                sf::Sprite* sprite =
                    storeAndLoadImage(tileset->getImage().u8string(), { 0, 0 });
                if (sprite != nullptr) {
                    sprite->setTextureRect(sf::IntRect(
                        { drawingRect.x, drawingRect.y },
                        { drawingRect.width, drawingRect.height }
                    ));
                    sprite->setPosition({ position.x, position.y });
                    window.draw(*sprite);
                }
            }
        }

    } else {
        spdlog::error(
            "Failed to load map: {}", static_cast<int>(map->getStatus())
        );
    }
    return true;
}

sf::Sprite* TileManager::storeAndLoadImage(
    const std::string& image, const sf::Vector2f& position
) {
    if (m_textures.count(image) == 0) {
        fs::path path = image;
        if (fs::exists(path) && fs::is_regular_file(path)) {
            std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>();
            bool imageFound = tex->loadFromFile(path.generic_string());
            if (imageFound) {
                std::unique_ptr<sf::Sprite> spr =
                    std::make_unique<sf::Sprite>(*tex);
                spr->setPosition(position);
                m_textures[image] = std::move(tex);
                m_sprites[image] = std::move(spr);
            }
        } else {
            std::cout << "Could not find: " << path.generic_string()
                      << std::endl;
        }
    }

    if (m_sprites.count(image) > 0) {
        return m_sprites[image].get();
    }

    return nullptr;
}