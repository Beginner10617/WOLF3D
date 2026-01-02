#include "Game.hpp"
#include "UIManager.hpp"
void Game::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            isRunning = false;

        // Hide cursor and lock on first click
        if (event.type == SDL_MOUSEBUTTONDOWN  && event.button.button == SDL_BUTTON_LEFT)
        {
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_TRUE);   // capture mouse
            //std::cout << "Mouse captured\n";
            if(!hasShot && weapons.size() > 0){
                if(weapons[currentWeapon].ammo == 0 && currentWeapon > 1){
                    std::cout << "Out of ammo!\n";
                }
                else{
                    shotThisFrame = true;
                    hasShot = true;
                    fireCooldown = 0.0f;
                    std::cout << "Fired weapon " << currentWeapon << "\n";
                    if(currentWeapon > 1){
                        weapons[currentWeapon].ammo--;
                    }
                    AudioManager::playSFX(weapons[currentWeapon].soundName, MIX_MAX_VOLUME);
                }
            }
            else{
            //    std::cout << "Weapon still cooling down\n";
            }
        }

        // Mouse movement → rotate player
        if (event.type == SDL_MOUSEMOTION)
        {
            // event.motion.xrel = delta X since last frame
            playerAngle += event.motion.xrel * mouseSensitivity;
            playerAngle = fmod(playerAngle, 2 * PI);
        }

        if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
        {
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                int tx = (int)(playerPosition.first  + cos(playerAngle));
                int ty = (int)(playerPosition.second + sin(playerAngle));

                auto key = std::make_pair(tx, ty);
                if (doors.count(key)) {
                    Door& d = doors[key];

                    if (d.locked && !playerHasKey(d.keyType))
                    {   // Do nothing
                        std::cout << "Door is locked! Need key type: " << d.keyType << "\n";
                        std::string name = ""; SDL_Color color;
                        switch(d.keyType){
                            case 1:
                                name = "BLUE";
                                color= {0, 252, 252, 255};
                                break;
                            case 2:
                                name = "RED";
                                color= {164, 0, 0, 255};
                                break;
                            case 3:
                                name = "GOLD";
                                color= {204, 196, 0, 255};
                                break;
                            default:
                                color = {0,0,0,0}; 
                        }
                        UIManager::notify("NEED "+name+" KEY TO OPEN", color);
                    }
                    else if (d.openAmount == 0.0f){
                        AudioManager::playSFX("door_open", MIX_MAX_VOLUME);
                        d.opening = true;
                    }
                }
            }
        }

    }

    // Keyboard movement detection
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    playerMoveDirection = {0.0f, 0.0f};

    // Forward
    if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP]) {
        playerMoveDirection.first += cos(playerAngle);
        playerMoveDirection.second += sin(playerAngle);
    }

    // Backward
    if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN]) {
        playerMoveDirection.first -= cos(playerAngle);
        playerMoveDirection.second -= sin(playerAngle);
    }

    // Strafe Left (A)
    if (keystate[SDL_SCANCODE_A]) {
        playerMoveDirection.first += cos(playerAngle - 3.14159f/2);
        playerMoveDirection.second += sin(playerAngle - 3.14159f/2);
    }

    // Strafe Right (D)
    if (keystate[SDL_SCANCODE_D]) {
        playerMoveDirection.first += cos(playerAngle + 3.14159f/2);
        playerMoveDirection.second += sin(playerAngle + 3.14159f/2);
    }

    // Optional keyboard turning (can keep or remove)
    if (keystate[SDL_SCANCODE_LEFT])
        playerAngle -= rotationSensitivity;

    if (keystate[SDL_SCANCODE_RIGHT])
        playerAngle += rotationSensitivity;

    // Weapon switching (number keys)
    if(keystate[SDL_SCANCODE_1]){
        if(playerHasWeapon(1)){
            currentWeapon = 1;
            weaponChangedThisFrame = true;
        }
    }
    if(keystate[SDL_SCANCODE_2]){
        if(playerHasWeapon(2)){
            currentWeapon = 2;
            weaponChangedThisFrame = true;
        }
    }
    if(keystate[SDL_SCANCODE_3]){
        if(playerHasWeapon(3)){
            currentWeapon = 3;
            weaponChangedThisFrame = true;
        }
    }

}
