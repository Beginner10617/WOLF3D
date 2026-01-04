#include "Game.hpp"
#include "AudioManager.hpp"
#include "UIManager.hpp"
#include "MenuManager.hpp"
#include <iostream>
Game* game = nullptr;

int main(int argc, char* argv[]) {
    // Initialisation
    game = new Game();
    // Loading Enemies
    game->loadEnemies("enemies.txt");

    // Initialize Game (player and enemies)
    game->init("My Game", 100, 100, 800, 600, false);
    
    // Load Map, Textures and Audio
    game->loadAllTextures("textureMapping.txt");
    game->loadEnemyTextures("enemyFrames.txt");
    game->loadDecorationTextures("Decorations.txt");
    AudioManager::loadAllAudios("audioConfig.txt");
    UIManager::loadTextures("HUD.txt", game->getRenderer());
    game->loadMapDataFromFile("testMap.txt");

    // Place Player
    game->placePlayerAt(2, 2, 0.0f);

    const int FPS = 60;
    const float frameDelay = 1000.0f / FPS;

    Uint32 lastTicks = SDL_GetTicks();

    while (game->running()) {
        Uint32 frameStart = SDL_GetTicks();

        // Delta Time calculation
        Uint32 currentTicks = frameStart;
        float deltaTime = (currentTicks - lastTicks) / 1000.0f;  
        lastTicks = currentTicks;

        // Game Loop 
        switch (game->getState())
        {
        case GameState::GAMEPLAY:
            game->handleEvents();
            game->update(deltaTime);
            game->render();
            break;
        
        default:
            if (MenuManager::handleEvents(game->state))
                game->quit();
            MenuManager::renderMenu(game->getRenderer(), {800, 600});
            break;
        }
        
        // Frame Limiter 
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        
        if (frameTime < frameDelay) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    delete game;
    return 0;
}
