#include "Game.hpp"
#include "UIManager.hpp"

void Game::update(float deltaTime)
{
    // Normalize movement direction
    float lengthSquared = playerMoveDirection.first * playerMoveDirection.first +
    playerMoveDirection.second * playerMoveDirection.second;

    // Last frame posn
    int mapX = playerPosition.first;
    int mapY = playerPosition.second;

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
        doors[{
            (int)(newX + playerSquareSize * (newX > playerPosition.first ?1:-1)),
            (int)playerPosition.second
                }]
            .openAmount == 1.0f))
    {
        if (!collidesWithEnemy(newX, playerPosition.second)) {
            playerPosition.first = newX;
        }
    }

    if (tileY == 0 ||
        (isDoor(tileY) &&
        doors[{
            (int)playerPosition.first,
            (int)(newY + playerSquareSize * (newY > playerPosition.second ?1:-1)),
                }]
            .openAmount == 1.0f))
    {
        if (!collidesWithEnemy(playerPosition.first, newY)) {
            playerPosition.second = newY;
        }
    }

    // new posn
    int newMapX = playerPosition.first;
    int newMapY = playerPosition.second;

    if(newMapX != mapX || newMapY != mapY){
        if(isDoor(Map[mapY][mapX]) && 
        doors.count({mapX, mapY})){
            doors[{mapX, mapY}].vacant = true;
            std::cout << "Vacated\n";
        }
        if(isDoor(Map[newMapY][newMapX]) && 
        doors.count({newMapX, newMapY})){
            doors[{newMapX, newMapY}].vacant = false;
            std::cout<< "Filled\n";
        }
    }

    // Update enemies
    for(const std::unique_ptr<Enemy>& e : enemies){
        // last frame posn
        int EX = e->get_position().first;
        int EY = e->get_position().second;

        auto epos = e->askGameToMove(deltaTime);
        std::pair<int, int> coor;
        int hasWall = canMoveTo(epos.first, epos.second, e->get_size(), coor);
        if(hasWall > 0){
            e->allowWalkNextFrame();
        }
        else{
            e->cancelWalkThisFrame(hasWall);
        }
        e->_process(deltaTime, playerPosition, playerAngle);
        if(e->get_wantToOpenDoor()){
            std::cout<<"Enemy want to open door at ("<<coor.first<<", "
            <<coor.second<<")"<<std::endl;
            if(doors.count(coor) && !doors[coor].locked && 
            !(doors[coor].opening || doors[coor].closing)){
                doors[coor].opening = true;
            }
            else if(doors.count(coor)==0){
                std::cerr<<"No door at ("<<coor.first<<", "<<coor.second<<")\n";
            }
            e->reset_wantToOpenThisFrame();
        }
        // Update canSeePlayer
        bool x = rayCastEnemyToPlayer(*e, false);
        e->updateCanSeePlayer(x);
        //if(x) std::cout << "Enemy at (" << e->get_position().first << ", " << e->get_position().second << ") can see player.\n";
        int dmg = e->getDamageThisFrame();
        e->clearDamageThisFrame();
        if(x && dmg > 0){
            health -= dmg;
            if(health < 0) health = 0;
            std::cout << "health of player : "<<health<<std::endl;
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

        // new position;
        int newEX = e->get_position().first;
        int newEY = e->get_position().second;

        if(newEX != EX || newEY != EY || e->get_isDead()){
            if(isDoor(Map[EY][EX]) && 
            doors.count({EX, EY}) &&
            !doors[{EX, EY}].vacant){
                doors[{EX, EY}].vacant = true;
                std::cout << "ENEMY Vacated\n";
            }
            if(isDoor(Map[newEY][newEX]) && 
            doors.count({newEX, newEY}) && !e->get_isDead()){
                doors[{newEX, newEY}].vacant = false;
                std::cout<< "ENEMY Filled\n";
            }
        }
    }
    if(hasShot){
        fireCooldown += deltaTime;
        if(fireCooldown >= weapons[currentWeapon].coolDownTime){
            hasShot = false;
            shotThisFrame = false;
        }
    }


    // Update doors
    for (auto& [pos, d] : doors)
    {
        if (d.opening) {
            d.openAmount += d.transitionSpeed * deltaTime;
            if (d.openAmount >= 1.0f) {
                d.openAmount = 1.0f;
                d.opening = false;
            }
        }
        else if (d.openAmount == 1.0f){
            d.openTimer += deltaTime;
            if(d.openTimer > d.openDuration){
                    if(d.vacant){
                        d.closing = true; 
                        d.openTimer = 0.0f;
                        AudioManager::playSFX("door_close", MIX_MAX_VOLUME);
                    } else 
                    {d.openTimer = 0.0f; 
                    std::cout<<"Restarting timer\n";}
            } 
        }
        if(d.closing){
            d.openAmount -= d.transitionSpeed * deltaTime;
            if(d.openAmount < 0.0f){
                d.closing = false;
                d.openAmount = 0.0f;
            }
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
            UIManager::addKey(static_cast<KeyType>(keyType-1));
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

    // Update health pack pickup
    for (const auto& [key, pos] : HealthPackPositions) {
        if(health >= 100)
            continue;
        auto [hpType, spriteID] = key;
            
        if(!AllSpriteTextures[spriteID].active)
            continue; // already picked up
        auto [hx, hy] = pos;
        float healthPackX = hx + 0.5f;
        float healthPackY = hy + 0.5f;
        float dx = healthPackX - playerPosition.first;
        float dy = healthPackY - playerPosition.second;
        float distanceSq = dx * dx + dy * dy;
        if (distanceSq < healthPackRadius * healthPackRadius) {
            int healAmount = healAmounts[hpType];
            health += healAmount;
            if(health > 100) health = 100;
            std::cout<<"Health : "<<health<<std::endl;
            AudioManager::playSFX("pickup", MIX_MAX_VOLUME / 2);
            // Remove health pack from map
            AllSpriteTextures[spriteID].active = false;
            break; // Exit loop since we modified the map
        }
    }

    // Update Ammopack Pickups
    for (const auto& [key, pos] : AmmoPackPositions){
        auto [Type, spriteID] = key;
        int weaponType = 2 + ((Type+1) % 2);
        if(!playerHasWeapon(weaponType)){
            continue; // Player doesn't have the weapon
        }
        if(!AllSpriteTextures[spriteID].active){
            continue; // Already picked up
        }
        auto [ax, ay] = pos;
        float x = ax + 0.5f;
        float y = ay + 0.5f;
        float dx = x - playerPosition.first;
        float dy = y - playerPosition.second;
        float distanceSq = dx * dx + dy * dy;
        if(distanceSq < ammoPackRadius * ammoPackRadius){
            if(weapons[weaponType].ammo >= weapons[weaponType].maxAmmo){
                weapons[weaponType].ammo = weapons[weaponType].maxAmmo;
                continue;
            }
            int amt = ammoAmounts[Type];
            weapons[weaponType].ammo += amt;
            if(weapons[weaponType].ammo >= weapons[weaponType].maxAmmo){
                weapons[weaponType].ammo = weapons[weaponType].maxAmmo;
            }
            std::cout<<"Ammo of weapon num "<<weaponType<<" = "<<weapons[weaponType].ammo<<std::endl;
            AudioManager::playSFX("pickup", MIX_MAX_VOLUME / 2);
            // Remove health pack from map
            AllSpriteTextures[spriteID].active = false;
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

    if(currentWeapon == 1 && weaponChangedThisFrame){
        UIManager::setWeapon(WeaponType::Knife);
        weaponChangedThisFrame = false;
    }
    else if(currentWeapon == 2){
        UIManager::setAmmo('P', weapons[currentWeapon].ammo);
        if(weaponChangedThisFrame)
            UIManager::setWeapon(WeaponType::Pistol);
        weaponChangedThisFrame = false;
    }
    else if(currentWeapon == 3){
        UIManager::setAmmo('S', weapons[currentWeapon].ammo);
        if(weaponChangedThisFrame)
            UIManager::setWeapon(WeaponType::Rifle);
        weaponChangedThisFrame = false;
    }
    if(shotThisFrame){
        UIManager::animateOneShot();
    }
    UIManager::setHealth(health);
    UIManager::update(deltaTime);

}
