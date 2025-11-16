#pragma once

#include "joanna/utils/logger.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

template <typename Resource> class ResourceManager {
  public:
    ResourceManager(const ResourceManager& obj) = delete;

    static ResourceManager* getInstance() {
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mtx);
            if (instance == nullptr) {
                instance = new ResourceManager();
            }
        }
        return instance;
    }

    Resource& get(const std::string& filename) {

        auto it = resources.find(filename);
        if (it != resources.end()) {
            return *(it->second);
        }

        std::unique_ptr<Resource> res = std::make_unique<Resource>();
        if (!res->loadFromFile(filename)) {
            Logger::error("Failed to load resource: {}", filename);
            throw std::runtime_error("Failed to open font: " + filename);
        }

        Resource& ref = *res;
        resources[filename] = std::move(res);
        return ref;
    }

    bool contains(const std::string& filename) const {
        return resources.count(filename) > 0;
    }

    void unload(const std::string& filename) {
        resources.erase(filename);
    }

    void clear() {
        resources.clear();
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<Resource>> resources;
    static ResourceManager* instance;
    static std::mutex mtx;

    ResourceManager() {};
};

template <typename Resource>
ResourceManager<Resource>* ResourceManager<Resource>::instance = nullptr;

template <typename Resource> std::mutex ResourceManager<Resource>::mtx;

// specialization for sf::Font, because it uses openFromFile
template <>
inline sf::Font& ResourceManager<sf::Font>::get(const std::string& filename) {
    auto it = resources.find(filename);
    if (it != resources.end())
        return *(it->second);

    auto res = std::make_unique<sf::Font>();
    if (!res->openFromFile(filename)) {
        Logger::error("Failed to load resource: {}", filename);
        throw std::runtime_error("Failed to open font: " + filename);
    }

    sf::Font& ref = *res;
    resources[filename] = std::move(res);
    return ref;
}
