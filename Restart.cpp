#include "Game.hpp"
#include "UIManager.hpp"
void Game::restart(){
    playerPosition = playerPositionOnLoad;
    playerAngle = 0.0f;
    health = 100;
    weapons.clear();
    currentWeapon=0;
    keysHeld.clear();
    for (int i=0; i<enemies.size(); i++){
        enemies[i]->setPosition(enemyLoadLocations[i]);
        enemies[i]->reset();
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