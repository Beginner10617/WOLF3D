#include "Game.hpp"
#include "UIManager.hpp"
void Game::restart(){
    playerPosition = playerPositionOnLoad;
    playerAngle = 0.0f;
    health = 100;
    weapons.clear();
    currentWeapon=0;
    keysHeld.clear();
    musicTrack = 1;
    for (int i=0; i<enemies.size(); i++){
        enemies[i]->setPosition(enemyLoadLocations[i]);
        enemies[i]->reset();
    }
    for (auto& [pos, d] : doors){
        d.openAmount = 0.0f;
        d.vacant = true;
        d.opening = false;
        d.closing = false;
        d.openTimer = 0.0f;
    }
    std::sort(indexOfSpawnedAmmos.begin(), 
    indexOfSpawnedAmmos.end(), std::greater<int>());
    for (int idx : indexOfSpawnedAmmos) {
        if (idx >= 0 && idx < static_cast<int>(AllSpriteTextures.size())) {
            AllSpriteTextures.erase(AllSpriteTextures.begin() + idx);
        }
    }
    for (int i=0; i<AllSpriteTextures.size(); i++){
        AllSpriteTextures[i].active = true;
    }
    UIManager::reset();
    state = GameState::GAMEPLAY;
}