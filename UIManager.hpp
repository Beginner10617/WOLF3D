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

enum class HUDSections {
    WEAPON,
    AMMO,
    AVATAR,
    HEALTH,
    KEYS
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

void drawFilledRectWithBorder(
    SDL_Renderer& renderer,
    const SDL_Rect& rect,
    SDL_Color fillColor,
    SDL_Color borderColor,
    int borderThickness
);

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

    static void addPanelTexture(WeaponType weapon, const char* filePath, SDL_Renderer& rend);
    static void renderPanelWeaponImage(
        SDL_Renderer& rend,
        const std::pair<int, int>& screenWH
    );
private:
    static WeaponType currentWeapon;
    static std::map<WeaponType, int> ammo;
    static int health;

    static std::map<WeaponType, UIAnimation> weaponAnimations;
    static constexpr float frameDuration = 0.1f;
    static float weaponAnimTimer;
    static int currentFrame;
    static bool animating;
    static const int IDLE_FRAME = 0;

    static BitmapFont font;
    static SDL_Rect panel;
    static int panelHeight, panelBorderThickness;
    static SDL_Color panelFillColor, panelBorderColor;
    static std::map<HUDSections, int> panelSectionWidths;

    static std::map<WeaponType, SDLTexturePtr> panelWeaponImage;
    static std::map<WeaponType, std::pair<int, int>> panelWeaponImageWH;
};
