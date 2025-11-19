#include "joanna/systems/audiomanager.h"
#include "joanna/utils/resourcemanager.h"
#include "joanna/utils/logger.h"
#include <SFML/Audio.hpp>

AudioManager::AudioManager()
    : sfx_volume_(100.0f)
      , music_volume_(75.0f) {
    
    auto* buffer_manager = ResourceManager<sf::SoundBuffer>::getInstance();
    
    const std::array<SfxId, 6> all_sfx = {
        SfxId::Hit, SfxId::Footstep, SfxId::Dead,
        SfxId::Surprise, SfxId::Collect, SfxId::Damage
    };

    for (size_t i = 0; i < all_sfx.size(); ++i) {
        try {
            sf::SoundBuffer& buffer = buffer_manager->get(get_sfx_path(all_sfx[i]));
            sounds_[i] = std::make_unique<sf::Sound>(buffer);
            sounds_[i]->setVolume(sfx_volume_);
        } catch (const std::exception& e) {
            Logger::error("Failed to load sound effect: {}", e.what());
        }
    }
}

std::string AudioManager::get_sfx_path(SfxId sfx_id) const {
    switch (sfx_id) {
        case SfxId::Hit:      return "assets/sfx/hit.wav";
        case SfxId::Footstep: return "assets/sfx/footstep.wav";
        case SfxId::Dead:     return "assets/sfx/dead.wav";
        case SfxId::Surprise: return "assets/sfx/surprise.wav";
        case SfxId::Collect:     return "assets/sfx/collect.wav";
        case SfxId::Damage:      return "assets/sfx/damage.wav";
        default:              throw std::invalid_argument("Invalid SfxId");
    }
}

std::string AudioManager::get_music_path(MusicId music_id) const {
    switch (music_id) {
        case MusicId::Overworld: return "assets/music/overworld.ogg";
        case MusicId::Underworld: return "assets/music/underworld.ogg";
        case MusicId::Beach:     return "assets/music/beach.ogg";
        default:                 throw std::invalid_argument("Invalid MusicId");
    }
}

void AudioManager::play_sfx(SfxId sfx_id) {
    size_t index = static_cast<size_t>(sfx_id);
    if (index < 6 && sounds_[index]) {
        sounds_[index]->play();
    } else {
        Logger::error("Sound effect not loaded: {}", static_cast<int>(sfx_id));
    }
}

void AudioManager::set_current_music(MusicId music_id) {
    if (current_music_.getStatus() == sf::Music::Status::Playing) {
        current_music_.stop();
    }
    Logger::info("Setting current music to ID: {}", static_cast<int>(music_id));
    try {
        if (!current_music_.openFromFile(get_music_path(music_id))) {
            throw sf::Exception("Failed to open music file");
        }
        current_music_.setVolume(music_volume_);
        current_music_.setLooping(true);
        current_music_.play();
    } catch (const sf::Exception& e) {
        Logger::error("Failed to load music: {}", get_music_path(music_id));
    }
}

void AudioManager::set_sfx_volume(float volume) {
    sfx_volume_ = std::min(std::max(volume, 0.0f), 100.0f);
    for (auto& sound : sounds_) {
        if (sound) {
            sound->setVolume(sfx_volume_);
        }
    }
}

void AudioManager::set_music_volume(float volume) {
    music_volume_ = std::min(std::max(volume, 0.0f), 100.0f);
    current_music_.setVolume(music_volume_);
}

void AudioManager::pause_music() {
    if (current_music_.getStatus() == sf::Music::Status::Playing) {
        current_music_.pause();
    }
}

void AudioManager::resume_music() {
    if (current_music_.getStatus() == sf::Music::Status::Paused) {
        current_music_.play();
    }
}

void AudioManager::stop_music() {
    current_music_.stop();
}

bool AudioManager::is_music_playing() const {
    return current_music_.getStatus() == sf::Music::Status::Playing;
}