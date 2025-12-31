#include "SDL.h"
#include "SDL_mixer.h"
#include <string>
#include <map>

using MixChunkPtr =
    std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)>;

using MixMusicPtr =
    std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>;

class AudioManager {
    static std::map<std::string, MixChunkPtr> soundEffects;
    static std::map<std::string, MixMusicPtr> musicTracks;
public:
    static void init();
    static void loadSoundEffect(const std::string& name, const std::string& path);
    static void loadMusic(const std::string& name, const std::string& path);
    static void loadAllAudios(const char* configFilePath);
    static void playSFX(const std::string& name, int volume);
    static void playSpatialSFX(const std::string& name,
        float distance,
        float relativeAngle);
    static void playMusic(const std::string& name, int loop = -1);
    static void stopMusic();
};