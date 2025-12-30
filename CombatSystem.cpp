#include "Game.hpp"

bool Game::canShootEnemy(float dist){
    const float MIN_DIST = 1.0f;
    const float MAX_DIST = 64.0f;   

    dist = std::clamp(dist, MIN_DIST, MAX_DIST);
    float t = (dist - MIN_DIST) / (MAX_DIST - MIN_DIST);

    // Quadratic falloff (feels very Wolf-like)
    int errorDivisor = ((int) (weapons[currentWeapon].accuracy - 1) * (1.0f - t * t)) + 1;
    return (rand() % errorDivisor) != 0;
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
        {
        //    std::cout << "Ray out of bounds at (" << mapX << ", " << mapY << ")\n";
            return false;
        }

        // Hit wall
        if (Map[mapY][mapX] > 0 && !isDoor(Map[mapY][mapX])){
        //    std::cout << "Ray blocked by wall at (" << mapX << ", " << mapY << ")\n";
            return false;
        }
        else if (isDoor(Map[mapY][mapX])) {
            auto doorIt = doors.find({mapX, mapY});
            if (doorIt != doors.end()) {
                Door& door = doorIt->second;
                if (door.openAmount < 1.0f){
                //    std::cout << "Ray blocked by closed door at (" << mapX << ", " << mapY << ")\n";
                    return false;
                }
            } else {
                //std::cout << "Ray blocked by unknown door at (" << mapX << ", " << mapY << ")\n";
                return false;
            }
        }

        // Reached player cell
        if (mapX == targetX && mapY == targetY)
            return true;
    }
}

void Game::acquireWeapon(int weaponType) {
    if (weaponType < 1 || weaponType > 3) {
        std::cerr << "Invalid weapon type: " << weaponType << "\n";
        return;
    }
    switch(weaponType){
        case 1:
            weapons[1] = weapon({1, 100, 0, 2.0f, 0.1f, 0.0f, "knife"});
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
