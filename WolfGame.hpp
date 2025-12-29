#ifndef Game_hpp
#define Game_hpp
#include "SDL.h"
#include "SDL_image.h"
#include "enemy.hpp"
#include <stdio.h>
#include <vector>
#include <utility>
#include <map>
#include "AudioManager.hpp"
using SDLWindowPtr =
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

using SDLRendererPtr =
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

using SDLTexturePtr =
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

class Game{
public:
    Game();
    ~Game() ;
    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
    void handleEvents();
    void update(float deltaTime);
    void render();
    void clean();
    bool running(){return isRunning;}
    void loadMapDataFromFile(const char* filename);
    void loadColorConfigFromFile(const char* filename);
    void placePlayerAt(int x, int y, float angle);
    void printPlayerPosition();
    void addWallTexture(const char* filePath);
    void addFloorTexture(const char* filePath);
    void addCeilingTexture(const char* filePath);
    bool isDoor(int tileValue);
    bool playerHasKey(int keyType);
    void loadAllTextures(const char* filePath);
    void addEnemy(float x, float y, float angle);
    void loadEnemyTextures(const char* filePath);
    bool collidesWithEnemy(float x, float y);
    bool canShootEnemy(float dist);
    void loadEnemies(const char* filePath);
    void acquireKey(int keyType);
    void loadKeysTexture(const char* filePath);
    void acquireWeapon(int weaponType);
    bool playerHasWeapon(int weaponType);
    void loadWeaponsTexture(const char* filePath);
private:
    bool isRunning;
    SDLWindowPtr   window   {nullptr, SDL_DestroyWindow};
    SDLRendererPtr renderer {nullptr, SDL_DestroyRenderer};
    float playerAngle, FOV=45.0f, playerSpeed=2.0f, rotationSensitivity=0.05f;
    float playerHeight=0.5f, mouseSensitivity=0.002f;
    float playerSquareSize=1.0f;
    std::pair<float, float> playerPosition;
    std::pair<int, int> ScreenHeightWidth;
    std::pair<double, double> playerMoveDirection = {0.0, 0.0};
    std::vector<std::vector<int>> Map, floorMap, ceilingMap;
    std::vector<SDLTexturePtr> wallTextures;
    std::vector<SDLTexturePtr> floorTextures;
    std::vector<SDLTexturePtr> ceilingTextures;
    std::vector<int> wallTextureWidths, floorTextureWidths, ceilingTextureWidths;
    std::vector<int> wallTextureHeights, floorTextureHeights, ceilingTextureHeights;
    struct Door {
        float openAmount;   // 0 = closed, 1 = fully open
        bool opening;       // opening animation active
        bool locked;        // requires key?
        int keyType;        // 0 = none, 1 = blue, 2 = red, 3 = gold
    };

    std::map<std::pair<int,int>, Door> doors;  // key: (mapX,mapY)
    std::vector<int> keysHeld; // keys the player has collected
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::map<std::pair<int, int>, SDLTexturePtr> enemyTextures;

    int health = 100;

    // weapon (current)
    struct weapon {
        int multiplier; // damage multiplier
        int accuracy;   // higher is better (1=never hit, 2=50%, 3=66%, etc)
        int ammo;
        float range; // >90 for infinite (map size)
        float coolDownTime;
        float alertRadius;
        std::string soundName;
    };
    std::vector<weapon> weapons = {
        //{1, 100, 0, true, 2.0f, 0.0f, 8.0f, "knife"},   // knife (K)
        //{2, 4, 0, false, 70.0f, 0.2f, 16.0f, "pistol"},  // pistol (P)
        //{3, 6, 0, false, 90.0f, 0.5f, 24.0f, "rifle"}   // rifle (S)
    };
    int currentWeaponIndex;
    float fireCooldown = 0.2f;
    bool shotThisFrame = false, hasShot = false;
    bool rayCastEnemyToPlayer(const Enemy& enemy);

    std::map<int, std::pair<int, int>> keysPositions;
    std::vector<SDLTexturePtr> keysTextures;
    float KEY_SIZE = 0.4f, keyRadius = 1.0f;

    std::map<int, std::pair<int, int>> weaponsPositions;
    std::vector<SDLTexturePtr> weaponsTextures;
    float WEAPON_SIZE = 0.5f, weaponRadius = 1.9f;
};

#endif