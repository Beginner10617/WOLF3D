#include "Game.hpp"
#include <iostream>

bool aabbIntersect(
    float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh
) {
    return ax < bx + bw &&
           ax + aw > bx &&
           ay < by + bh &&
           ay + ah > by;
}

Game::Game(){

}

Game::~Game(){
    clean();
}

void Game::placePlayerAt(int x, int y, float angle) {
    playerPosition = {static_cast<double>(x), static_cast<double>(y)};
    playerAngle = angle;
}

void Game::printPlayerPosition(){
    std::cout << "Player Position: (" << playerPosition.first << ", " << playerPosition.second << ")\n";
}
bool Game::isDoor(int tile) {
    return tile >= 6 && tile <=9;
}
bool Game::playerHasKey(int keyType) {
    if(keyType == 0) return true; // no key needed
    for (int key : keysHeld) {
        if (key == keyType) {
            return true;
        }
    }
    return false;
}
void Game::clean()
{
    enemyTextures.clear();
    wallTextures.clear();
    floorTextures.clear();
    ceilingTextures.clear();
    doors.clear();
    enemies.clear();

    renderer.reset();
    window.reset();

    IMG_Quit();
    SDL_Quit();

}

bool Game::collidesWithEnemy(float x, float y) {
    for (const auto& e : enemies)
    {
        if (aabbIntersect(
            x, y,
            playerSquareSize, playerSquareSize,
            e->get_position().first, e->get_position().second,
            e->get_size(), e->get_size()
        )) {
            return true;
        }
    }
    return false;
}

bool Game::rayCastEnemyToPlayer(const Enemy& enemy) {
    float ex = enemy.get_position().first;
    float ey = enemy.get_position().second;

    float px = playerPosition.first;
    float py = playerPosition.second;

    // Direction vector
    float dx = px - ex;
    float dy = py - ey;

    float rayLength = std::hypot(dx, dy);
    if (rayLength < 0.0001f)
        return true;

    dx /= rayLength;
    dy /= rayLength;

    // Current grid square
    int mapX = int(std::floor(ex));
    int mapY = int(std::floor(ey));

    int targetX = int(std::floor(px));
    int targetY = int(std::floor(py));

    // Ray step direction
    int stepX = (dx < 0) ? -1 : 1;
    int stepY = (dy < 0) ? -1 : 1;

    // Distance to first grid boundary
    float sideDistX;
    float sideDistY;

    float deltaDistX = (dx == 0) ? 1e30f : std::abs(1.0f / dx);
    float deltaDistY = (dy == 0) ? 1e30f : std::abs(1.0f / dy);

    if (dx < 0)
        sideDistX = (ex - mapX) * deltaDistX;
    else
        sideDistX = (mapX + 1.0f - ex) * deltaDistX;

    if (dy < 0)
        sideDistY = (ey - mapY) * deltaDistY;
    else
        sideDistY = (mapY + 1.0f - ey) * deltaDistY;

    // DDA loop
    while (true) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
        }

        // Bounds check
        if (mapY < 0 || mapY >= (int)Map.size() ||
            mapX < 0 || mapX >= (int)Map[0].size())
            return false;

        // Hit wall
        if (Map[mapY][mapX] != 0)
            return false;

        // Reached player cell
        if (mapX == targetX && mapY == targetY)
            return true;
    }
}

bool Game::canShootEnemy(float dist){
    const float MIN_DIST = 1.0f;
    const float MAX_DIST = 64.0f;   

    dist = std::clamp(dist, MIN_DIST, MAX_DIST);
    float t = (dist - MIN_DIST) / (MAX_DIST - MIN_DIST);

    // Quadratic falloff (feels very Wolf-like)
    int errorDivisor = ((int) (weapons[currentWeapon].accuracy - 1) * (1.0f - t * t)) + 1;
    return (rand() % errorDivisor) != 0;
}

void Game::acquireKey(int keyType) {
    if (!playerHasKey(keyType)) {
        keysHeld.push_back(keyType);
        std::cout << "Acquired key of type: " << keyType << "\n";
        AudioManager::playSFX("pickup", MIX_MAX_VOLUME / 2);
    }
}
void Game::acquireWeapon(int weaponType) {
    if (weaponType < 1 || weaponType > 3) {
        std::cerr << "Invalid weapon type: " << weaponType << "\n";
        return;
    }
    switch(weaponType){
        case 1:
            weapons[1] = weapon({1, 100, 0, 2.0f, 0.0f, 0.0f, "knife"});
            break;
        case 2:
            weapons[2] = weapon({2, 4, 30, 70.0f, 0.2f, 16.0f, "pistol"});
            break;
        case 3:
            weapons[3] = weapon({3, 6, 50, 90.0f, 0.5f, 24.0f, "rifle"});
            break;
        default:
            std::cerr << "Unhandled weapon type: " << weaponType << "\n";
    }
    //{1, 100, 0, true, 2.0f, 0.0f, 8.0f, "knife"},   // knife (K)
    //{2, 4, 0, false, 70.0f, 0.2f, 16.0f, "pistol"},  // pistol (P)
    //{3, 6, 0, false, 90.0f, 0.5f, 24.0f, "rifle"}   // rifle (S)
    currentWeapon = weaponType;
    AudioManager::playSFX("pickup", MIX_MAX_VOLUME / 2);
}

bool Game::playerHasWeapon(int weaponType) {
    for (const auto& [i, w] : weapons) {
        if (i == weaponType) {
            return true;
        }
    }
    return false;
}

bool Game::canMoveTo(float x, float y) {
    // Check map boundaries
    if (x < 0 || x >= Map[0].size() || y < 0 || y >= Map.size()) {
        return false;
    }

    int tileX = Map[(int)y][(int)(x + playerSquareSize * (x>playerPosition.first?1:-1))];
    int tileY = Map[(int)(y + playerSquareSize * (y>playerPosition.second?1:-1))][(int)x];

    // Check for walls
    if (tileX != 0 || tileY != 0) {
        return false;
    }

    // Check for doors
    int mapX = static_cast<int>(x);
    int mapY = static_cast<int>(y);
    auto doorIt = doors.find({mapX, mapY});
    if (doorIt != doors.end()) {
        Door& door = doorIt->second;
        if (door.locked && !playerHasKey(door.keyType)) {
            return false;
        }
    }

    return true;
}