#include "Game.hpp"

bool Game::canShootEnemy(float dist){
    const float MIN_DIST = 1.0f;
    const float MAX_DIST = 64.0f;   

    dist = std::clamp(dist, MIN_DIST, MAX_DIST);
    float t = (dist - MIN_DIST) / (MAX_DIST - MIN_DIST);

    // Quadratic falloff (feels very Wolf-like)
    if(weapons.find(currentWeapon) == weapons.end())
        return false;
    int errorDivisor = ((int) (weapons[currentWeapon].accuracy - 1) * (1.0f - t * t)) + 1;
    return (rand() % errorDivisor) != 0;
}


bool Game::rayCastEnemyToPlayer(const Enemy& enemy, bool isPlayer) {
    float ex = enemy.get_position().first;
    float ey = enemy.get_position().second;

    float px = playerPosition.first;
    float py = playerPosition.second;

    float r = 0.1f;

    std::vector<std::pair<float, float>> samplePoints;
    
    if(isPlayer){
        samplePoints = {
            std::make_pair(ex, ey),
        };
    }
    else{
        samplePoints = {
            std::make_pair(ex, ey),
            std::make_pair(ex+r, ey),
            std::make_pair(ex, ey+r),
            std::make_pair(ex-r, ey),
            std::make_pair(ex, ey-r)
        };
    }

    for(const auto& [x, y] : samplePoints)
    {
        // Direction vector
        float dx = px - x;
        float dy = py - y;

        float rayLength = std::hypot(dx, dy);
        if (rayLength < 0.0001f)
            return true;

        dx /= rayLength;
        dy /= rayLength;

        // Current grid square
        int mapX = int(std::floor(x));
        int mapY = int(std::floor(y));

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
            sideDistX = (x - mapX) * deltaDistX;
        else
            sideDistX = (mapX + 1.0f - x) * deltaDistX;

        if (dy < 0)
            sideDistY = (y - mapY) * deltaDistY;
        else
            sideDistY = (mapY + 1.0f - y) * deltaDistY;

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
            {
                break;
            }

            // Hit wall
            if (Map[mapY][mapX] > 0 && !isDoor(Map[mapY][mapX])){
                break;
            }
            else if (isDoor(Map[mapY][mapX])) {
                auto doorIt = doors.find({mapX, mapY});
                if (doorIt != doors.end()) {
                    Door& door = doorIt->second;
                    if (door.openAmount < 1.0f){
                        break;
                    }
                } else {
                    break;
                }
            }

            // Reached player cell
            if (mapX == targetX && mapY == targetY)
                return true;
        }
    }
    return false;
}

void Game::acquireWeapon(int weaponType) {
    if (weaponType < 1 || weaponType > 3) {
        std::cerr << "Invalid weapon type: " << weaponType << "\n";
        return;
    }
    weaponChangedThisFrame = true;
    switch(weaponType){
        case 1:
            weapons[1] = weapon({1, 100, 0, 2.0f, 0.1f, 0.0f, "knife"});
            break;
        case 2:
            weapons[2] = weapon({2, 4, 30, 70.0f, 0.3f, 16.0f, "pistol"});
            break;
        case 3:
            weapons[3] = weapon({3, 6, 50, 90.0f, 0.7f, 24.0f, "rifle"});
            break;
        default:
            std::cerr << "Unhandled weapon type: " << weaponType << "\n";
    }
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

void Game::spawnRandomAmmoPack(std::pair<int, int> pos){
    int type = 3 + (rand() % 2);
    int spriteID = AllSpriteTextures.size();
    AmmoPackPositions[{type, spriteID}] = pos;
    AllSpriteTextures.push_back(Sprite{ spriteID, pos, ammoPackTextures[2],
                 ammoPackWidthsHeights[3].first, ammoPackWidthsHeights[3].second});
}