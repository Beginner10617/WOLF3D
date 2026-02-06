#include "AudioManager.hpp"
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

namespace {

// trim helpers
inline void ltrim(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         [](unsigned char c){ return !std::isspace(c); }));
}

inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char c){ return !std::isspace(c); }).base(),
            s.end());
}

inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

} 

int computeVolume(float dist)
{
    constexpr float maxDist = 12.0f;

    if (dist >= maxDist)
        return 0;

    float t = dist / maxDist;
    float volume = 1.0f - t;
    volume *= volume;

    return static_cast<int>(volume * MIX_MAX_VOLUME);
}

std::map<std::string, MixChunkPtr> AudioManager::soundEffects{};
std::map<std::string, MixMusicPtr> AudioManager::musicTracks{};
void AudioManager::init() {

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Error: %s\n", Mix_GetError());
    }

    Mix_AllocateChannels(16); // number of simultaneous SFX
}

void AudioManager::loadSoundEffect(const std::string& name, const std::string& path)
{
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound effect: " << path
                  << " | " << Mix_GetError() << '\n';
        return;
    }
    soundEffects.emplace(name, MixChunkPtr(chunk, Mix_FreeChunk));
}

void AudioManager::loadMusic(const std::string& name, const std::string& path)
{
    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        std::cerr << "Failed to load music: " << path
                  << " | " << Mix_GetError() << '\n';
        return;
    }
    musicTracks.emplace(name, MixMusicPtr(music, Mix_FreeMusic));
}

void AudioManager::loadAllAudios(std::string f)
{
    const char* filePath = f.c_str();
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio list: " << filePath << '\n';
        return;
    }

    enum class Section {
        None,
        Music,
        SFX
    };

    Section currentSection = Section::None;
    std::string line;

    while (std::getline(file, line)) {
        trim(line);

        if (line.empty())
            continue;

        // Section headers
        if (line.front() == '[' && line.back() == ']') {
            std::string header = toLower(line.substr(1, line.size() - 2));

            if (header == "music")
                currentSection = Section::Music;
            else if (header == "sfx")
                currentSection = Section::SFX;
            else
                currentSection = Section::None;

            continue;
        }

        // Expect: name : path
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string name = line.substr(0, colonPos);
        std::string path = line.substr(colonPos + 1);

        trim(name);
        trim(path);

        if (name.empty() || path.empty())
            continue;

        switch (currentSection) {
            case Section::Music:
                loadMusic(name, path);
                std::cout<<name<<" "<<name.size()<<std::endl;
                break;

            case Section::SFX:
                loadSoundEffect(name, path);
                break;

            default:
                // ignore entries outside known sections
                break;
        }
    }
}

void AudioManager::playSFX(const std::string& name, int volume)
{
    auto it = soundEffects.find(name);
    if (it == soundEffects.end()) {
        std::cerr << "SFX not found: " << name << "\n";
        return;
    }

    int channel = Mix_PlayChannel(-1, it->second.get(), 0);
    if (channel == -1) {
        std::cerr << "Failed to play SFX: "
                  << Mix_GetError() << "\n";
        return;
    }

    volume = std::clamp(volume, 0, MIX_MAX_VOLUME);
    Mix_Volume(channel, volume);
}

void AudioManager::playMusic(const std::string& name, int loop)
{
    auto it = musicTracks.find(name);
    if (it == musicTracks.end()) {
        std::cerr << "Music not found: " << name << "\n";
        return;
    }

    // Stop previous music if any
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    if (Mix_PlayMusic(it->second.get(), loop) == -1) {
        std::cerr << "Failed to play music: "
                  << Mix_GetError() << "\n";
    }
}
bool AudioManager::musicStopped(){
    return !Mix_PlayingMusic();
}
void AudioManager::playSpatialSFX(
    const std::string& name,
    float distance,
    float relativeAngle)
{
    auto it = soundEffects.find(name);
    if (it == soundEffects.end())
        return;

    int channel = Mix_PlayChannel(-1, it->second.get(), 0);
    if (channel == -1)
        return;

    // Distance attenuation
    int volume = computeVolume(distance);
    Mix_Volume(channel, volume);

    // Stereo panning
    float pan = std::sin(relativeAngle);
    Uint8 left  = static_cast<Uint8>((1.0f - pan) * 127.5f);
    Uint8 right = static_cast<Uint8>((1.0f + pan) * 127.5f);
    Mix_SetPanning(channel, left, right);
}

void AudioManager::stopMusic()
{
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}