#include "Game.hpp"

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0){
        int flags = 0;
        if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
            SDL_Log("IMG init error: %s", IMG_GetError());
            isRunning = false;
            return;
        }
        if(fullscreen){
            flags = SDL_WINDOW_FULLSCREEN;
        }
        window.reset(SDL_CreateWindow(title, xpos, ypos, width, height, flags));
        ScreenHeightWidth = std::make_pair(width, height);
        if(window){
            renderer.reset(SDL_CreateRenderer(window.get(), -1, 0));
            if(renderer.get()){
                SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
            }
            isRunning = true;
        }
    } else {
        isRunning = false;
    }
    int i=0;
    for(const std::unique_ptr<Enemy>& e : enemies){
        e->init(static_cast<int>(AllSpriteTextures.size()));
        AllSpriteTextures.push_back(Sprite{static_cast<int>(AllSpriteTextures.size()), 
            e->get_position(), nullptr
            , enemyTextureWidth, enemyTextureHeight,
            true
            });
        enemySpriteIDToindex[e->get_spriteID()] = i;
        i++;
    }
    AudioManager::init();
}
