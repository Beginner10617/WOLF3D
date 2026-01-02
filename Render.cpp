#include "Game.hpp"
#include "UIManager.hpp"

void Game::render()
{
    SDL_SetRenderDrawColor(renderer.get(), 40, 40, 40, 255);
    SDL_RenderClear(renderer.get());   
    std::vector<float> zBuffer(ScreenHeightWidth.first);

    // Draw floor
    SDL_SetRenderDrawColor(renderer.get(), 100, 100, 100, 255);
    SDL_Rect floorRect = {0, ScreenHeightWidth.second / 2, ScreenHeightWidth.first, ScreenHeightWidth.second / 2};
    SDL_RenderFillRect(renderer.get(), &floorRect);
    
    // Raycasting for walls
    int raysCount = ScreenHeightWidth.first;
    
    for (int ray = 0; ray < raysCount; ray++)
    {
        // Angle of this ray
        float rayAngle = playerAngle - halfFov + ray * (fovRad / raysCount);

        // Ray direction
        float rayDirX = cos(rayAngle);
        float rayDirY = sin(rayAngle);

        // Map tile the ray starts in
        int mapX = (int)playerPosition.first;
        int mapY = (int)playerPosition.second;

        // Length of ray from one x-side to next x-side
        float deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0f / rayDirX);
        // Length of ray from one y-side to next y-side
        float deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0f / rayDirY);

        int stepX, stepY;
        float sideDistX, sideDistY;

        // Step direction and initial side distances
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (playerPosition.first - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - playerPosition.first) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (playerPosition.second - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - playerPosition.second) * deltaDistY;
        }

        bool hitWall = false, doorSide = false, wallISDoor=false;
        int hitSide = 0; // 0 = vertical hit, 1 = horizontal hit
        int MapWidth = Map[0].size();
        int MapHeight= Map.size();
        while (!hitWall)
        {
            doorSide = isDoor(Map[mapY][mapX]);
            // Jump to next grid square
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                hitSide = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                hitSide = 1;
            }
            // Check if the ray hit a wall
            if (mapX < 0 || mapX >= MapWidth ||
                mapY < 0 || mapY >= MapHeight) {
                hitWall = true;
                break;
            }

            if (Map[mapY][mapX] > 0 && !isDoor(Map[mapY][mapX])) {
                hitWall = true;
            }
            else if (isDoor(Map[mapY][mapX]))
            {
                if(
                (hitSide == 0 && (sideDistY < sideDistX - deltaDistX/2))||
                (hitSide == 1 && (sideDistX < sideDistY - deltaDistY/2))
                )
                {
                    hitWall = false;
                    continue;
                }
                float open = 0.0f;
                auto it = doors.find({mapX, mapY});
                if (it != doors.end()) {
                    open = it->second.openAmount;
                } else {
                    std::cout << "Door at "<<mapY<<", "<<mapX<<" not found\n";
                    return;
                }
                // --- compute hit distance ---
                float hitDist = (hitSide == 0 ? sideDistX - deltaDistX/2
                                            : sideDistY - deltaDistY/2);

                // exact float hit position
                float hitX = playerPosition.first  + rayDirX * hitDist;
                float hitY = playerPosition.second + rayDirY * hitDist;

                // local coords inside tile (0..1)
                float localX = hitX - floor(hitX);
                float localY = hitY - floor(hitY);

                bool blocks = false;

                if (hitSide == 0) {
                    // vertical → opening affects Y movement
                    // door blocks if hit Y is beyond open amount
                    blocks = (localY >= open);
                } else {
                    // horizontal → opening affects X movement
                    blocks = (localX >= open);
                }

                if (blocks){
                    hitWall = true;      // ray stops here
                    wallISDoor = true;
                }
            }
        }

        // Distance to wall = distance to side where hit happened
        float distanceToWall;
        if (hitSide == 0)
            distanceToWall = sideDistX - deltaDistX;
        else
            distanceToWall = sideDistY - deltaDistY;
        if (wallISDoor)
            distanceToWall += (hitSide==0)? deltaDistX/2: deltaDistY/2;
        float hitX = playerPosition.first  + rayDirX * distanceToWall;
        float hitY = playerPosition.second + rayDirY * distanceToWall;
        float wallX;
        if (hitSide == 0)
            wallX = hitY - floor(hitY);
        else
            wallX = hitX - floor(hitX);
        float deltaAngle = rayAngle - playerAngle;
        float correctedDistance = distanceToWall * cos(deltaAngle);
        zBuffer[ray] = correctedDistance;

        // Calculate wall height
        int lineHeight = (int)(ScreenHeightWidth.second / correctedDistance);
        int drawStart = -lineHeight / 2 + ScreenHeightWidth.second / 2;
        int drawEnd   =  lineHeight / 2 + ScreenHeightWidth.second / 2;

        // Wall Texture
        int texId = Map[mapY][mapX] - 1;
        if (texId < 0 || texId >= wallTextures.size())
            continue;
        int imgWidth = wallTextureWidths[texId], imgHeight = wallTextureHeights[texId];
        
        // -------- distance-based shading --------
        float maxLightDist = 8.0f;
        float shade = 1.0f - std::min(correctedDistance / maxLightDist, 1.0f);
        Uint8 brightness = (Uint8)(40 + shade * 215);

        // Darken horizontal walls (classic Wolf3D trick)
        if (hitSide == 1) {
            brightness = (Uint8)(brightness * 0.7f);
        }
        if(doorSide)
            SDL_SetTextureColorMod(DOOR_FRAME.get(),
                            brightness, brightness, brightness);
        else
            SDL_SetTextureColorMod(wallTextures[texId].get(),
                            brightness, brightness, brightness);
        // --------------------------------------

        if(!isDoor(texId+1)){
            int texX = (int)(wallX * imgWidth);
            if(hitSide == 0 && rayDirX > 0) texX = imgWidth - texX - 1;
            if(hitSide == 1 && rayDirY < 0) texX = imgWidth - texX - 1;
            texX = std::clamp(texX, 0, imgWidth - 1);

            SDL_Rect srcRect  = { texX, 0, 1, imgHeight };
            SDL_Rect destRect = { ray, drawStart, 1, drawEnd - drawStart };
            if(doorSide)
                SDL_RenderCopy(renderer.get(), 
                DOOR_FRAME.get(), &srcRect, &destRect);
            else
                SDL_RenderCopy(renderer.get(), 
                wallTextures[texId].get(), &srcRect, &destRect);
        }
        else if (wallX > doors[{mapX, mapY}].openAmount)
        {
            wallX -= doors[{mapX, mapY}].openAmount;

            int texX = int(wallX * imgWidth);
            texX = std::clamp(texX, 0, imgWidth - 1);

            SDL_Rect srcRect  = { texX, 0, 1, imgHeight };
            SDL_Rect destRect = { ray, drawStart, 1, drawEnd - drawStart };

            SDL_RenderCopy(renderer.get(),
                        wallTextures[texId].get(),
                        &srcRect,
                        &destRect);
        }


        
    }
    // Rendering Sprites
    // Sort sprites by distance from player (far to near)
    std::sort(renderOrder.begin(), renderOrder.end(),
        [&](int a, int b) {
            return distSq(playerPosition, AllSpriteTextures[a].position) >
           distSq(playerPosition, AllSpriteTextures[b].position);
        });
    int enemyShotIndex = -1;
    for (int i=0; i < renderOrder.size(); i++) {
        int id = renderOrder[i];
        const Sprite& sprite = AllSpriteTextures[id];
        if (!sprite.texture){
            //std::cout << "Skipping sprite ID " << sprite.spriteID << " due to null texture.\n";
            //std::cout << sprite.isEnemy << "\n";
            continue; // skip if texture is null
        }
        // Sprite position relative to player
        auto [sx, sy] = sprite.position;
        if(!sprite.isEnemy){
            sx += 0.5f;
            sy += 0.5f;
        }
        float dx = sx - playerPosition.first;
        float dy = sy - playerPosition.second;
        float spriteDist = sqrt(dx*dx + dy*dy);
        if(spriteDist < playerSquareSize / 2)
            continue;

        // Angle between player view and sprite
        float spriteAngle = atan2(dy, dx) - playerAngle;
        while (spriteAngle > PI)  spriteAngle -= 2 * PI;
        while (spriteAngle < -PI) spriteAngle += 2 * PI;
        // Check if sprite is inside FOV
        if (fabs(spriteAngle) > halfFov)
            continue; // skip sprite if outside FOV
        
        // Project sprite onto screen
        int screenX = (int)((spriteAngle + halfFov) / fovRad * ScreenHeightWidth.first);
        screenX = std::clamp(screenX, 0, ScreenHeightWidth.first - 1);

        // Perspective scaling
        int spriteHeight = (int)(ScreenHeightWidth.second / spriteDist);
        int spriteWidth  = (int)(spriteHeight * ((float)sprite.textureWidth / sprite.textureHeight));

        int drawStartY = -spriteHeight / 2 + ScreenHeightWidth.second / 2;;
        int drawEndY   =  spriteHeight / 2 + ScreenHeightWidth.second / 2;
        int drawStartX = screenX - spriteWidth / 2;
        int drawEndX   = screenX + spriteWidth / 2;

        int centreX = ScreenHeightWidth.first / 2;

        if(sprite.isEnemy && shotThisFrame && spriteDist < weapons[currentWeapon].range
            && drawStartX + spriteWidth * (1-enemyBoundBox)/2 < centreX
            && drawEndX - spriteWidth * (1-enemyBoundBox)/2 > centreX
            && zBuffer[centreX] > spriteDist
        ){
            enemyShotIndex = enemySpriteIDToindex.at(sprite.spriteID);
            std::cout << drawStartX << ", " << drawEndX << " " << centreX << "\n";
        }
        else if(shotThisFrame && sprite.isEnemy && 
            ( drawStartX + spriteWidth * (1-enemyBoundBox) > centreX
            || drawEndX - spriteWidth * (1-enemyBoundBox) < centreX)
        ){
            std::cout << "Enemy at sprite ID " << sprite.spriteID << " not at centre\n";
        }
        else if(sprite.isEnemy && shotThisFrame){ 
            std::cout << "Enemy at sprite ID " << sprite.spriteID << " out of range for shooting,\n dist=" << spriteDist << " and range=" << weapons[currentWeapon].range << "\n";
        }

        if(sprite.isEnemy){

            SDL_RenderDrawLine(renderer.get(),
                (int)(drawStartX + spriteWidth * (1-enemyBoundBox)/2), 0,
                (int)(drawStartX + spriteWidth * (1-enemyBoundBox)/2), ScreenHeightWidth.second
            );

            SDL_RenderDrawLine(renderer.get(),
                (int)(drawEndX - spriteWidth * (1-enemyBoundBox)/2), 0,
                (int)(drawEndX - spriteWidth * (1-enemyBoundBox)/2), ScreenHeightWidth.second
            );
        }

        SDL_Texture* texture = sprite.texture.get();

        // Render sprite column by column
        for (int x = drawStartX; x < drawEndX; x++) {
            int texX = (int)((float)(x - (screenX - spriteWidth / 2)) / (float)spriteWidth * sprite.textureWidth);
            if (texX < 0 || texX >= sprite.textureWidth) continue;
            if(x < 0 || x >= zBuffer.size()) continue;
            if (spriteDist < zBuffer[x]) {
                SDL_Rect srcRect  = { texX, 0, 1, sprite.textureHeight };
                SDL_Rect destRect = { x, drawStartY, 1, drawEndY - drawStartY };
                SDL_RenderCopy(renderer.get(), texture, &srcRect, &destRect);
            }
        }
    }
    if (enemyShotIndex != -1) {
        auto [x, y] = enemies[enemyShotIndex]->get_position();
        float dist = distSq(
            playerPosition,
            enemies[enemyShotIndex]->get_position()
        );
        dist = sqrt(dist); // sqrt
        int dmg=0;
        if(canShootEnemy(dist))
            dmg = (rand() & 31) * weapons[currentWeapon].multiplier;
        std::cout << "Enemy at index " << enemyShotIndex << " shot for " << dmg << " damage.\n";
        if (rayCastEnemyToPlayer(*enemies[enemyShotIndex], true)){
            if(enemies[enemyShotIndex]->takeDamage(dmg)){
                
                spawnRandomAmmoPack(std::make_pair((int)x, (int)y)); 
            }
        }
        shotThisFrame = false;
    }
    else if (shotThisFrame) {
        shotThisFrame = false;
    }
        SDL_RenderDrawLine(renderer.get(),
            ScreenHeightWidth.first/2, 0,
            ScreenHeightWidth.first/2, ScreenHeightWidth.second
            );

    UIManager::renderHUD(
        getRenderer(),
        ScreenHeightWidth
    );
    SDL_RenderPresent(renderer.get()); 
}
