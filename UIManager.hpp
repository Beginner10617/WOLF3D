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
    float frameDuration = 0.2f;
    int width = 64;
    int height = 64;
};

class UIManager {
public:
    static void loadTextures(const char* filePath);
    static void update(float deltaTime);
    static void renderHUD(SDL_Renderer* renderer);
    static void renderPauseMenu(SDL_Renderer* renderer);

    static void setWeapon(WeaponType weapon);
    static void setAmmo(int current, int max);
    static void setHealth(int hp);
    static void animateOneShot();
    static void addTexture(WeaponType weapon, SDLTexturePtr tex);

private:
    static WeaponType currentWeapon;
    static int ammo;
    static int health;

    static std::map<WeaponType, UIAnimation> weaponAnimations;

    static float weaponAnimTimer;
    static int currentFrame;
    static bool animating;
    static const int IDLE_FRAME = 1;
};
