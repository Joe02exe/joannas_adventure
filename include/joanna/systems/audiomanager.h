#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>

enum class SfxId {
    Hit,
    Footstep,
    Dead,
    Surprise,
    Collect,
    Damage
    // These are not all SFX. More in assets/sfx folder.
};

enum class MusicId {
    Overworld,
    Underworld,
    Beach
};

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

    bool is_music_playing() const;

  private:
    std::string get_sfx_path(SfxId sfx_id) const;
    std::string get_music_path(MusicId music_id) const;

    std::unique_ptr<sf::Sound> sounds_[6];
    sf::Music current_music_;
    
    float sfx_volume_;
    float music_volume_;
};