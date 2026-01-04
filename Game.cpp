#include "Game.hpp"
#include "AudioManager.hpp"

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
    playerPositionOnLoad = playerPosition;
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
            playerSquareSize, playerSquareSize,
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

int Game::canMoveTo(
    float x, float y,
    float width,
    std::pair<int,int>& cord)
{
    float half = width * 0.5f;

    // Object AABB
    float objX = x - half;
    float objY = y - half;
    float objW = width;
    float objH = width;

    // Expanded bounds check
    if (objX < 0 || objY < 0 ||
        objX + objW >= Map[0].size() ||
        objY + objH >= Map.size())
    {
        return 0;
    }

    // Tile range overlapped by the AABB
    int minX = (int)std::floor(objX);
    int maxX = (int)std::floor(objX + objW);
    int minY = (int)std::floor(objY);
    int maxY = (int)std::floor(objY + objH);

    for (int ty = minY; ty <= maxY; ++ty) {
        for (int tx = minX; tx <= maxX; ++tx) {

            int val = Map[ty][tx];
            if (val == 0)
                continue;

            // Tile AABB
            float tileX = (float)tx;
            float tileY = (float)ty;

            if (!aabbIntersect(
                    objX, objY, objW, objH,
                    tileX, tileY, 1.0f, 1.0f))
                continue;

            // Wall
            if (!isDoor(val)) {
                //std::cout<<"Enemy stopped by wall val="<<val
                //<<"at ("<<tileX<<", "<<tileY<<")"<<"\n";
                return 0;
            }

            // Door
            auto doorIt = doors.find({tx, ty});
            if (doorIt != doors.end()) {
                Door& door = doorIt->second;
                if (door.openAmount < 1.0f) {
                    cord = {tx, ty};
                    //std::cout<<"Enemy stopped by door\n";
                    return -1;
                }
            }
        }
    }

    return 1;
}

SDL_Renderer& Game::getRenderer() {
    return *renderer;
}

const SDL_Renderer& Game::getRenderer() const {
    return *renderer;
}
