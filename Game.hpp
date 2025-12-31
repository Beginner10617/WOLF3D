#ifndef Game_hpp
#define Game_hpp
#include "SDL.h"
#include "SDL_image.h"
#include "enemy.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include "AudioManager.hpp"

// Utilities
using SDLWindowPtr =
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

using SDLRendererPtr =
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

using SDLTexturePtr = std::shared_ptr<SDL_Texture>;

struct Sprite {
    int spriteID;
    std::pair<float, float> position;
    SDLTexturePtr texture;
    int textureWidth;
    int textureHeight;
    bool isEnemy = false;
    bool active = true;
};

auto distSq = [](const std::pair<float, float>& a,
                 const std::pair<float, float>& b)
{
    float dx = a.first  - b.first;
    float dy = a.second - b.second;
    return dx * dx + dy * dy;
};

static std::string toLower(const std::string &s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return r;
}

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
    bool canMoveTo(float x, float y);
    void loadHealthPackTexture(const char* filePath);
    void loadAmmoPackTexture(const char* filePath);
    void spawnRandomAmmoPack(std::pair<int, int>);
    void addDecorationTexture(char x, const char* filePath);
    void loadDecorationTextures(const char* filePath);

    SDL_Renderer& getRenderer();
    const SDL_Renderer& getRenderer() const;

private:
    bool isRunning;
    SDLWindowPtr   window   {nullptr, SDL_DestroyWindow};
    SDLRendererPtr renderer {nullptr, SDL_DestroyRenderer};
    float playerAngle, FOV=45.0f, playerSpeed=2.0f, rotationSensitivity=0.05f;
    float fovRad = FOV * (PI / 180.0f);
    float halfFov = fovRad / 2.0f;

    float playerHeight=0.5f, mouseSensitivity=0.002f;
    float playerSquareSize=1.0f;
    std::pair<float, float> playerPosition;
    std::pair<int, int> ScreenHeightWidth;
    std::pair<double, double> playerMoveDirection = {0.0, 0.0};
    std::vector<std::vector<int>> Map;
    std::vector<SDLTexturePtr> wallTextures;
    std::vector<int> wallTextureWidths;
    std::vector<int> wallTextureHeights;
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
    std::map<int, int> enemySpriteIDToindex;
    int enemyTextureWidth = 64;
    int enemyTextureHeight = 64;
    int health = 100;

    // weapon (current)
    float enemyBoundBox = 0.27f;
    struct weapon {
        int multiplier; // damage multiplier
        int accuracy;   // higher is better (1=never hit, 2=50%, 3=66%, etc)
        int ammo;
        float range; // >90 for infinite (map size)
        float coolDownTime;
        float alertRadius;
        std::string soundName;
    };
    std::map<int, weapon> weapons;
    int currentWeapon = 0;
    float fireCooldown = 0.2f;
    bool shotThisFrame = false, hasShot = false, weaponChangedThisFrame = false;
    bool rayCastEnemyToPlayer(const Enemy& enemy, bool isPlayer);

    std::map<int, std::pair<int, int>> keysPositions, keyWidthsHeights;
    std::vector<SDLTexturePtr> keysTextures;
    float keyRadius = 1.0f;
    std::map<int, int> keyTypeToSpriteID; // B R G

    std::map<int, std::pair<int, int>> weaponsPositions, weaponWidthsHeights;
    std::vector<SDLTexturePtr> weaponsTextures;
    float weaponRadius = 1.0f;
    std::map<int, int> weaponTypeToSpriteID; // K P S

    // healthPack Type and its index 
    //(index is nothing but the spriteID)
    //(there can be multiple pack of same type)
    std::map<std::pair<int, int>, std::pair<int, int>> HealthPackPositions;
    
    // Keys for the following are health pack type only 
    std::map<int, std::pair<int, int>> healthPackWidthsHeights;
    std::vector<SDLTexturePtr> healthPackTextures; // except vectors ofcourse
    std::map<int, int> healAmounts = {{1,10}, {2,25}};    
    float healthPackRadius = 1.0f;
    // health packs: 1 = small (+10), 2 = large (+25) 
    // H h

    // Same Logic for Ammo packs
    std::map<std::pair<int, int>, std::pair<int, int>> AmmoPackPositions;

    // Ammo Pack Types 1 = small (+15 pistol), 2 = large (+10 rifle)
    // 3/4 -> random spawn on enemy death (+5 pistol/rifle)
    std::map<int, std::pair<int, int>> ammoPackWidthsHeights;
    std::vector<SDLTexturePtr> ammoPackTextures;
    std::map<int, int> ammoAmounts = {{1,15}, {2,10}, {3,5}, {4,5}};
    float ammoPackRadius = 1.0f;
    // A a

    std::map<char, SDLTexturePtr> DecorationTextures;
    std::map<char, std::pair<int, int>> DecorationTextureWidthsHeights;

    std::vector<Sprite> AllSpriteTextures;
    std::vector<int> renderOrder; // holds spriteIDs

};

#endif