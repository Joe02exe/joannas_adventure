#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <stdexcept>

class PostProcessing {
  public:
    PostProcessing(unsigned int width, unsigned int height);

    // Draw the scene to the internal render texture
    void drawScene(const std::function<void(sf::RenderTarget&, const sf::View&)>& drawFunc, const sf::View* customView = nullptr);

    // Apply post-processing effects and render to target
    void apply(sf::RenderTarget& target, float time);

    void resize(unsigned int width, unsigned int height);

    // Get the internal render texture (if needed for direct access)
    sf::RenderTexture& getRenderTexture() { return m_sceneTexture; }

  private:
    unsigned int m_width;
    unsigned int m_height;

    sf::RenderTexture m_sceneTexture;
    sf::Sprite m_sceneSprite;
    sf::Shader m_shader;
};

#endif // POSTPROCESSING_H