#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>

enum class SfxId : std::uint8_t {
    Hit,
    Footstep,
    Dead,
    Surprise,
    Collect,
    Damage,
    Click
    // These are not all SFX. More in assets/sfx folder.
};

enum class MusicId : std::uint8_t { Overworld, Underworld, Beach, Combat };

class AudioManager {
  public:
    AudioManager();
    ~AudioManager() = default;

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = default;
    AudioManager& operator=(AudioManager&&) = default;

    void play_sfx(SfxId sfx_id);
    void set_current_music(MusicId music_id);

    void set_sfx_volume(float volume);
    void set_music_volume(float volume);
    void pause_music();
    void resume_music();
    void stop_music();

    [[nodiscard]] bool is_music_playing() const;

  private:
    [[nodiscard]] std::string get_sfx_path(SfxId sfx_id) const;
    [[nodiscard]] std::string get_music_path(MusicId music_id) const;

    std::array<std::unique_ptr<sf::Sound>, 7> sounds_;
    sf::Music current_music_;

    float sfx_volume_;
    float music_volume_;
};