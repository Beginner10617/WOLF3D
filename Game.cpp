#include "Game.hpp"


Game::Game(){

}

Game::~Game(){
    clean();
}

bool aabbIntersect(
    float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh
) {
    return ax < bx + bw &&
           ax + aw > bx &&
           ay < by + bh &&
           ay + ah > by;
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
        if (e->get_isDead()) continue;
        if (aabbIntersect(
            x, y,
            playerSquareSize * 0.5f, playerSquareSize * 0.5f,
            e->get_position().first, e->get_position().second,
            e->get_size(), e->get_size()
        )) {
            return true;
        }
    }
    return false;
}

void Game::acquireKey(int keyType) {
    if (!playerHasKey(keyType)) {
        keysHeld.push_back(keyType);
        std::cout << "Acquired key of type: " << keyType << "\n";
        AudioManager::playSFX("pickup", MIX_MAX_VOLUME / 2);
    }
}

bool Game::canMoveTo(float x, float y) {
    // Check map boundaries
    if (x < 0 || x >= Map[0].size() || y < 0 || y >= Map.size()) {
        return false;
    }

    int tileX = Map[(int)y][(int)x];
    int tileY = Map[(int)y][(int)x];
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
        if (door.openAmount < 1.0f) {
            return false;
        }
    }

    return true;
}