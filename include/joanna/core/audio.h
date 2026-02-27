#pragma once

#ifdef MIYOO_BUILD
#include <SDL_mixer.h>
#include <string>

namespace jo {

class SoundBuffer {
  public:
    bool loadFromFile(const std::string& filename) {
        return true;
    } // Dummy
};

class Sound {
  public:
    Sound() = default;

    explicit Sound(const SoundBuffer& buffer) {}

    void setBuffer(const SoundBuffer& buffer) {}

    void play() {}

    void stop() {}

    void setVolume(float volume) {}
};

class Music {
  public:
    enum class Status { Stopped, Paused, Playing };

    bool openFromFile(const std::string& filename) {
        return true;
    }

    void play() {}

    void pause() {}

    void stop() {}

    void setLooping(bool loop) {}

    void setVolume(float volume) {}

    Status getStatus() const {
        return Status::Stopped;
    }
};

} // namespace jo
#else
#include <SFML/Audio.hpp>

namespace jo {
using namespace sf;
}
#endif
