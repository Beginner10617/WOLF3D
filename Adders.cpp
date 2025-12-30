#include "Game.hpp"

void Game::addEnemy(float x, float y, float angle) {
    enemies.push_back(std::make_unique<Enemy>(x, y, angle));
}

void Game::addWallTexture(const char* filePath)
{
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load wall texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    wallTextures.emplace_back(raw, SDL_DestroyTexture);

    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        wallTextures.pop_back();   // keep vectors in sync
        return;
    }

    wallTextureWidths.push_back(width);
    wallTextureHeights.push_back(height);
}

void Game::addFloorTexture(const char* filePath) {
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load floor texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    floorTextures.emplace_back(raw, SDL_DestroyTexture);

    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        floorTextures.pop_back();   // keep vectors in sync
        return;
    }

    floorTextureWidths.push_back(width);
    floorTextureHeights.push_back(height);
}

void Game::addCeilingTexture(const char* filePath) {
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load ceiling texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    ceilingTextures.emplace_back(raw, SDL_DestroyTexture);

    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        ceilingTextures.pop_back();   // keep vectors in sync
        return;
    }

    ceilingTextureWidths.push_back(width);
    ceilingTextureHeights.push_back(height);
}
