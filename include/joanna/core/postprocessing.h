#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#pragma once

#include "joanna/core/graphics.h"
#include <functional>

class PostProcessing {
  public:
    PostProcessing(unsigned int width, unsigned int height);

    // Draw the scene to the internal render texture
    void drawScene(const std::function<void(jo::RenderTarget&, const jo::View&)>& drawFunc, const jo::View* customView = nullptr);

    // Apply post-processing effects and render to target
    void apply(jo::RenderTarget& target, float time);

    void resize(unsigned int width, unsigned int height);

    // Get the internal render texture (if needed for direct access)
    jo::RenderTexture& getRenderTexture() { return m_sceneTexture; }

  private:
    unsigned int m_width;
    unsigned int m_height;

    jo::RenderTexture m_sceneTexture;
    jo::Sprite m_sceneSprite;
    jo::Shader m_shader;
};

#endif // POSTPROCESSING_H