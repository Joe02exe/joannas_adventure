#include "joanna/entities/entityutils.h"

#include "joanna/utils/logger.h"
#include "joanna/utils/resourcemanager.h"

Animation::Animation() = default;

Animation::Animation(
    const std::string& path, const jo::Vector2i& frameSize, int frameCount
)
    : texture(&ResourceManager<jo::Texture>::getInstance()->get(path)) {
    frames.reserve(frameCount);
    // ---- DIAGNOSTIC LOG ----
    Logger::info(
        "[Anim] Loading: {} | frameSize={}x{} | frameCount={} | tex={}x{}",
        path, frameSize.x, frameSize.y, frameCount, texture->getSize().x,
        texture->getSize().y
    );
    for (int i = 0; i < frameCount; ++i) {
        jo::IntRect rect({ i * frameSize.x, 0 }, frameSize);
        // Warn if rect exceeds texture width
        if (rect.position.x + rect.size.x > (int)texture->getSize().x) {
            Logger::error(
                "[Anim] FRAME {} OUT OF BOUNDS: rect.x={} + w={} > tex.w={} in "
                "{}",
                i, rect.position.x, rect.size.x, texture->getSize().x, path
            );
        }
        frames.emplace_back(rect);
    }
    Logger::info(
        "[Anim] Done: {} frames generated for {}", frames.size(), path
    );
}
