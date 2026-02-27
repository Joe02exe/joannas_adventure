#include "joanna/core/postprocessing.h"

PostProcessing::PostProcessing(unsigned int width, unsigned int height)
    : m_width(width), m_height(height),
      m_sceneTexture(jo::Vector2u(width, height)),
      m_sceneSprite(m_sceneTexture.getTexture()) {
    if (!m_shader.loadFromFile(
            "assets/shader/crt_shader.frag", jo::Shader::Type::Fragment
        )) {
        throw std::runtime_error("Failed to load CRT shader.");
    }

    m_shader.setUniform("scanline_color", jo::Glsl::Vec4(0.f, 0.f, 0.f, 1.f));
    m_shader.setUniform("flicker_color", jo::Glsl::Vec4(0.f, 0.f, 0.f, 1.f));
    m_shader.setUniform("scanlines_count", 10.f);
    m_shader.setUniform("scanlines_intensity", 0.05f);
    m_shader.setUniform("flicker_speed", 30.f);
    m_shader.setUniform("flicker_intensity", 0.00f);
    m_shader.setUniform("color_offset", 0.5f);
    m_shader.setUniform(
        "texture_size",
        jo::Vector2f(static_cast<float>(width), static_cast<float>(height))
    );
}

void PostProcessing::drawScene(
    const std::function<void(jo::RenderTarget&, const jo::View&)>& drawFunc,
    const jo::View* customView
) {
    m_sceneTexture.clear(jo::Color::Black);

    jo::View oldView = m_sceneTexture.getView();

    if (customView != nullptr) {
        m_sceneTexture.setView(*customView);
    } else {
        m_sceneTexture.setView(oldView);
    }

    drawFunc(m_sceneTexture, m_sceneTexture.getView());

    m_sceneTexture.setView(oldView);
    m_sceneTexture.display();
}

void PostProcessing::apply(jo::RenderTarget& target, float time) {
    m_shader.setUniform("texture", m_sceneTexture.getTexture());
    m_shader.setUniform("time", time);
    target.draw(m_sceneSprite, &m_shader);
}

void PostProcessing::resize(unsigned int width, unsigned int height) {
    m_width = width;
    m_height = height;

    m_sceneTexture = jo::RenderTexture({ width, height });
    m_sceneSprite.setTexture(m_sceneTexture.getTexture(), true);
    m_shader.setUniform(
        "texture_size",
        jo::Vector2f(static_cast<float>(width), static_cast<float>(height))
    );
}
