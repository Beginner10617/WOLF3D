#pragma once
#include <map>
#include <vector>
#include "SDL.h"
#include "Game.hpp"

enum class WeaponType {
    None,
    Knife,
    Pistol,
    Rifle
};

struct UIAnimation {
    std::vector<SDLTexturePtr> frames;
    int width = 64;
    int height = 64;
};

struct BitmapFont {
    SDLTexturePtr texture;
    int glyphW;
    int glyphH;
    std::string charset;
};

class UIManager {
public:
    static void loadTextures(const char* filePath, SDL_Renderer& r);
    static void loadFont(const char* charsetAndFilePath, SDL_Renderer& renderer);
    static int getGlyphIndex(char c);

    static void update(float deltaTime);
    
    static void renderHUD(
        SDL_Renderer& rend, 
        const std::pair<int, int>& WH);

    static void renderWeapon(
        SDL_Renderer& rend, 
        const std::pair<int,int>& screenSize, 
        int yOffset);

    static void renderText(
        SDL_Renderer& renderer,
        const std::string& text,
        int x, int y,
        int scale,
        SDL_Color color
    );

    static void renderPauseMenu(SDL_Renderer& renderer, const std::pair<int, int>& WH);

    static void setWeapon(WeaponType weapon);
    static void setAmmo(const char, int num);
    static void setHealth(int hp);
    static void animateOneShot();
    static void addTexture(WeaponType weapon, const char* filePath, SDL_Renderer& rend);

private:
    static WeaponType currentWeapon;
    static std::map<WeaponType, int> ammo;
    static int health;

    static std::map<WeaponType, UIAnimation> weaponAnimations;
    static constexpr float frameDuration = 0.1f;
    static float weaponAnimTimer;
    static int currentFrame;
    static bool animating;
    static const int IDLE_FRAME = 1;

    static BitmapFont font;
};
