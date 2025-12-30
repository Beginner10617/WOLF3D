#include "Game.hpp"
#include <iostream>
void Game::update(float deltaTime)
{
    // Normalize movement direction
    float lengthSquared = playerMoveDirection.first * playerMoveDirection.first +
    playerMoveDirection.second * playerMoveDirection.second;

    if (lengthSquared > 0.0f) {
        float length = sqrt(lengthSquared);
        playerMoveDirection.first /= length;
        playerMoveDirection.second /= length;
    }

    // Collision detection and position update
    float newX = playerPosition.first + playerMoveDirection.first * playerSpeed * deltaTime;
    float newY = playerPosition.second + playerMoveDirection.second * playerSpeed * deltaTime;
    int mx = (int)newX;
    int my = (int)playerPosition.second;
    if (my < 0 || my >= Map.size()){
        std::cout << "Y out of bounds: " << my << "\n";
        return;
    }
    if (mx < 0 || mx >= Map[my].size()) {
        std::cout << "X out of bounds: " << mx << "\n";
        return;
    }
    int tileX = Map[(int)playerPosition.second][(int)(newX + playerSquareSize * (newX>playerPosition.first?1:-1))];
    int tileY = Map[(int)(newY + playerSquareSize * (newY>playerPosition.second?1:-1))][(int)playerPosition.first];
    if (tileX == 0 ||
        (isDoor(tileX) &&
        doors[{(int)playerPosition.second,
                (int)(newX + playerSquareSize * (newX > playerPosition.first ? 1 : -1))}]
            .openAmount > 0.5f))
    {
        if (!collidesWithEnemy(newX, playerPosition.second)) {
            playerPosition.first = newX;
        }
    }

    if (tileY == 0 ||
        (isDoor(tileY) &&
        doors[{(int)(newY + playerSquareSize * (newY > playerPosition.second ? 1 : -1)),
                (int)playerPosition.first}]
            .openAmount > 0.5f))
    {
        if (!collidesWithEnemy(playerPosition.first, newY)) {
            playerPosition.second = newY;
        }
    }

    // Update doors
    for (auto& [pos, d] : doors)
    {
        if (d.opening) {
            d.openAmount += 1.5f * deltaTime;
            if (d.openAmount >= 1.0f) {
                d.openAmount = 1.0f;
                d.opening = false;
            }
        }
    }

    // Update enemies
    for(const std::unique_ptr<Enemy>& e : enemies){
        auto epos = e->askGameToMove(deltaTime);
        if(canMoveTo(epos.first, epos.second)){
            e->allowWalkNextFrame();
        }
        else{
            e->cancelWalkThisFrame();
        }
        e->_process(deltaTime, playerPosition, playerAngle);

        // Update canSeePlayer
        bool x = rayCastEnemyToPlayer(*e);
        e->updateCanSeePlayer(x);
        int dmg = e->getDamageThisFrame();
        e->clearDamageThisFrame();
        if(x && dmg > 0){
            health -= dmg;
            if(health < 0) health = 0;
        }
        // Update Alerts
        if(shotThisFrame && currentWeapon > 1 && !e->isAlerted()){
            float dist = distSq(playerPosition, e->get_position());
            dist = pow(dist, 0.5f);
            if(dist <= weapons[currentWeapon].alertRadius)
                e->alert();
        }

        // Enemy position relative to player 
        auto [ex, ey] = e->get_position();

        float dx = ex - playerPosition.first;
        float dy = ey - playerPosition.second;

        float enemyDist = sqrt(dx*dx + dy*dy);

        // Angle between player view and enemy 
        float enemyAngle = atan2(dy, dx) - playerAngle;

        while (enemyAngle > PI)  enemyAngle -= 2 * PI;
        while (enemyAngle < -PI) enemyAngle += 2 * PI;

        // Check if enemy is inside FOV 
        if (fabs(enemyAngle) > halfFov){
            //std::cout<<"enemy out of FOV\n";
            continue; 
        }
        int frame = e->get_current_frame(), dir = e->get_dirn_num();
        auto it = enemyTextures.find({frame, dir});
        if (it == enemyTextures.end()) continue;
        AllSpriteTextures[e->get_spriteID()].texture = it->second;
        AllSpriteTextures[e->get_spriteID()].position = e->get_position();

    }
    if(hasShot){
        fireCooldown += deltaTime;
        if(fireCooldown >= weapons[currentWeapon].coolDownTime){
            hasShot = false;
            shotThisFrame = false;
        }
    }

    // Update keys pickup
    for (const auto& [keyType, pos] : keysPositions) {
        if(playerHasKey(keyType))
            continue;
        auto [kx, ky] = pos;
        float keyX = kx + 0.5f;
        float keyY = ky + 0.5f;
        
        float dx = keyX - playerPosition.first;
        float dy = keyY - playerPosition.second;
        float distanceSq = dx * dx + dy * dy;

        if (distanceSq < keyRadius * keyRadius) {
            acquireKey(keyType);
            // Remove key from map
            AllSpriteTextures[keyTypeToSpriteID[keyType]].active = false;
            break; // Exit loop since we modified the map
        }
    }

    // Update weapons pickup
    for (const auto& [weaponType, pos] : weaponsPositions) {
        if(playerHasWeapon(weaponType))
            continue;
        auto [wx, wy] = pos;
        float weaponX = wx + 0.5f;
        float weaponY = wy + 0.5f;

        float dx = weaponX - playerPosition.first;
        float dy = weaponY - playerPosition.second;
        float distanceSq = dx * dx + dy * dy;

        if (distanceSq < weaponRadius * weaponRadius) {
            acquireWeapon(weaponType);
            // Remove weapon from map
            AllSpriteTextures[weaponTypeToSpriteID[weaponType]].active = false;
            break; // Exit loop since we modified the map
        }
    }

        // ------- Update Textures to be rendered ---------
        renderOrder.clear();
        
        for (int i = 0; i < AllSpriteTextures.size(); ++i) {
            if (AllSpriteTextures[i].active){
                renderOrder.push_back(i);
            }
        }

}
