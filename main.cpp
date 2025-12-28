#include "WolfGame.hpp"

Game* game = nullptr;

int main(int argc, char* argv[]) {
    game = new Game();
    //game->loadEnemies("enemies.txt");
    game->init("My Game", 100, 100, 800, 600, false);
    AudioManager::loadAllAudios("audioConfig.txt");
    game->placePlayerAt(2, 2, 0.0f);
    game->loadMapDataFromFile("testMap.txt");
    game->loadAllTextures("textureMapping.txt");
    game->loadEnemyTextures("enemyFrames.txt");

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
        game->handleEvents();
        game->update(deltaTime);   
        game->render();

        // Frame Limiter 
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        
        if (frameTime < frameDelay) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    delete game;
    return 0;
}
