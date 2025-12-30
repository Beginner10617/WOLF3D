#include "Game.hpp"
#include <iostream>
void Game::render()
{
    SDL_SetRenderDrawColor(renderer.get(), 40, 40, 40, 255);
    SDL_RenderClear(renderer.get());   
    std::vector<float> zBuffer(ScreenHeightWidth.first);

    // Draw floor
    if (floorTextures.size() == 0) {
        SDL_SetRenderDrawColor(renderer.get(), 100, 100, 100, 255);
        SDL_Rect floorRect = {0, ScreenHeightWidth.second / 2, ScreenHeightWidth.first, ScreenHeightWidth.second / 2};
        SDL_RenderFillRect(renderer.get(), &floorRect);
    }
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

        bool hitWall = false;
        int hitSide = 0; // 0 = vertical hit, 1 = horizontal hit

        while (!hitWall)
        {
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
            if (Map[mapY][mapX] > 0 && !isDoor(Map[mapY][mapX])) {
                hitWall = true;
            }
            else if (isDoor(Map[mapY][mapX]))
            {
                float open = 0.0f;
                auto it = doors.find({mapX, mapY});
                if (it != doors.end()) {
                    open = it->second.openAmount;
                } else {
                    std::cout << "Door at "<<mapY<<", "<<mapX<<" not found\n";
                    return;
                }
                
                // --- compute hit distance ---
                float hitDist = (hitSide == 0 ? sideDistX - deltaDistX
                                            : sideDistY - deltaDistY);

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

                if (blocks)
                    hitWall = true;      // ray stops here
                else
                    hitWall = false;     // ray passes through (door is open)
            }

        }

        // Distance to wall = distance to side where hit happened
        float distanceToWall;
        if (hitSide == 0)
            distanceToWall = sideDistX - deltaDistX;
        else
            distanceToWall = sideDistY - deltaDistY;

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

        if (drawStart < 0) drawStart = 0;
        if (drawEnd >= ScreenHeightWidth.second) drawEnd = ScreenHeightWidth.second - 1;

        // Wall Texture
        int texId = Map[mapY][mapX] - 1;
        int imgWidth = wallTextureWidths[texId], imgHeight = wallTextureHeights[texId];
        SDL_QueryTexture(wallTextures[texId].get(), NULL, NULL, &imgWidth, &imgHeight);

        // -------- distance-based shading --------
        float maxLightDist = 8.0f;
        float shade = 1.0f - std::min(correctedDistance / maxLightDist, 1.0f);
        Uint8 brightness = (Uint8)(40 + shade * 215);

        // Darken horizontal walls (classic Wolf3D trick)
        if (hitSide == 1) {
            brightness = (Uint8)(brightness * 0.7f);
        }
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
            SDL_RenderCopy(renderer.get(), wallTextures[texId].get(), &srcRect, &destRect);
        }
        else if (wallX > doors[{mapX, mapY}].openAmount) {

            wallX -= doors[{mapX, mapY}].openAmount;
            int texX = (int)(wallX * imgWidth);
            if(hitSide == 0 && rayDirX > 0) texX = imgWidth - texX - 1;
            if(hitSide == 1 && rayDirY < 0) texX = imgWidth - texX - 1;
            texX = std::clamp(texX, 0, imgWidth - 1);

            SDL_Rect srcRect  = { texX, 0, 1, imgHeight };
            SDL_Rect destRect = { ray, drawStart, 1, drawEnd - drawStart };
            SDL_RenderCopy(renderer.get(), wallTextures[texId].get(), &srcRect, &destRect);
        }

        // Draw floor texture
        if (floorTextures.size() > 0) {
            int floorScreenStart = drawEnd; // start drawing floor below the wall
            imgHeight = floorTextureHeights[0];
            imgWidth = floorTextureWidths[0];
            for (int y = drawEnd; y < ScreenHeightWidth.second; y++) {
                float rowDist = playerHeight / ((float)y / ScreenHeightWidth.second - 0.5f);

                // Interpolate floor coordinates
                float floorX = playerPosition.first + rowDist * rayDirX;
                float floorY = playerPosition.second + rowDist * rayDirY;

                int texX = ((int)(floorX * imgWidth)) % imgWidth;
                int texY = ((int)(floorY * imgHeight)) % imgHeight;

                SDL_Rect srcRect  = { texX, texY, 1, 1 };
                SDL_Rect destRect = { ray, y, 1, 1 };
                SDL_RenderCopy(renderer.get(), floorTextures[0].get(), &srcRect, &destRect);
            }
        }
        
        // Draw ceiling
        for(int y = 0; y < drawStart; y++) {
            if(ceilingTextures.size() > 0) {
                int imgWidth = ceilingTextureWidths[0];
                int imgHeight = ceilingTextureHeights[0];

                float rowDist = playerHeight / (0.5f - (float)y / ScreenHeightWidth.second);

                // Interpolate ceiling coordinates
                float ceilX = playerPosition.first + rowDist * rayDirX;
                float ceilY = playerPosition.second + rowDist * rayDirY;

                int texX = ((int)(ceilX * imgWidth)) % imgWidth;
                int texY = ((int)(ceilY * imgHeight)) % imgHeight;

                SDL_Rect srcRect  = { texX, texY, 1, 1 };
                SDL_Rect destRect = { ray, y, 1, 1 };
                SDL_RenderCopy(renderer.get(), ceilingTextures[0].get(), &srcRect, &destRect);
            } 
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
        float dx = sx - playerPosition.first;
        float dy = sy - playerPosition.second;
        float spriteDist = sqrt(dx*dx + dy*dy);
        if (spriteDist < playerSquareSize)
            continue; // skip sprite if too close to player
        // Angle between player view and sprite
        float spriteAngle = atan2(dy, dx) - playerAngle;
        while (spriteAngle > PI)  spriteAngle -= 2 * PI;
        while (spriteAngle < -PI) spriteAngle += 2 * PI;
        // Check if sprite is inside FOV
        if (fabs(spriteAngle) > halfFov)
            continue; // skip sprite if outside FOV
        
        // Project sprite onto screen
        int screenX = (int)((spriteAngle + halfFov) / fovRad * ScreenHeightWidth.first);

        // Perspective scaling
        int spriteHeight = (int)(ScreenHeightWidth.second / spriteDist);
        int spriteWidth  = (int)(spriteHeight * ((float)sprite.textureWidth / sprite.textureHeight));

        int drawStartY = -spriteHeight / 2 + ScreenHeightWidth.second / 2;;
        int drawEndY   =  spriteHeight / 2 + ScreenHeightWidth.second / 2;
        int drawStartX = screenX - spriteWidth / 2;
        int drawEndX   = screenX + spriteWidth / 2;

        int centreX = ScreenHeightWidth.first / 2;
        if(sprite.isEnemy && shotThisFrame && spriteDist < weapons[currentWeapon].range){
            enemyShotIndex = enemySpriteIDToindex.at(sprite.spriteID);
        }

        if (drawStartY < 0) drawStartY = 0;
        if (drawEndY >= ScreenHeightWidth.second) drawEndY = ScreenHeightWidth.second - 1;
        if (drawStartX < 0) drawStartX = 0;
        if (drawEndX >= ScreenHeightWidth.first) drawEndX = ScreenHeightWidth.first - 1;

        SDL_Texture* texture = sprite.texture.get();

        // Render sprite column by column
        for (int x = drawStartX; x < drawEndX; x++) {
            int texX = (int)((float)(x - (screenX - spriteWidth / 2)) / (float)spriteWidth * sprite.textureWidth);
            if (texX < 0 || texX >= sprite.textureWidth) continue;
            if (spriteDist < zBuffer[x]) {
                SDL_Rect srcRect  = { texX, 0, 1, sprite.textureHeight };
                SDL_Rect destRect = { x, drawStartY, 1, drawEndY - drawStartY };
                SDL_RenderCopy(renderer.get(), texture, &srcRect, &destRect);
            }
        }
    }
    if (enemyShotIndex != -1) {
        float dist = distSq(
            playerPosition,
            enemies[enemyShotIndex]->get_position()
        );
        dist = pow(dist, 0.5f); // sqrt
        int dmg=0;
        if(canShootEnemy(dist))
            dmg = (rand() & 31) * weapons[currentWeapon].multiplier;
        //std::cout << "Enemy at index " << enemyShotIndex << " shot for " << dmg << " damage.\n";
        if (rayCastEnemyToPlayer(*enemies[enemyShotIndex])){
            enemies[enemyShotIndex]->takeDamage(dmg); 
            float relativeAngle = atan2(
                enemies[enemyShotIndex]->get_position().second - playerPosition.second,
                enemies[enemyShotIndex]->get_position().first  - playerPosition.first
            ) - playerAngle;
            while (relativeAngle > PI)  relativeAngle -= 2 * PI;
            while (relativeAngle < -PI) relativeAngle += 2 * PI;
        }
        shotThisFrame = false;
    }
    else if (shotThisFrame) {
        shotThisFrame = false;
    }
    SDL_RenderPresent(renderer.get()); 
}
