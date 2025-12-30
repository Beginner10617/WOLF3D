#include "Game.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
void Game::loadMapDataFromFile(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map data file: " << filename << std::endl;
        return;
    }

    Map.clear();
    doors.clear();

    std::string line;
    size_t rowIndex = 0;

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token) {                 // space-separated
            // Key handling
            //std::cout<<token<<"\n";
            if(token == "B"){
                keysPositions[1] = {row.size(), rowIndex};
                keyTypeToSpriteID[1] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, keysTextures[0],
                 keyWidthsHeights[1].first, keyWidthsHeights[1].second});
                row.push_back(0);
                continue;
            }
            else if(token == "R"){
                keysPositions[2] = {row.size(), rowIndex};
                keyTypeToSpriteID[2] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, keysTextures[1],
                 keyWidthsHeights[2].first, keyWidthsHeights[2].second});
                row.push_back(0);
                continue;
            }
            else if(token == "G"){
                keysPositions[3] = {row.size(), rowIndex};
                keyTypeToSpriteID[3] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, keysTextures[2], 
                 keyWidthsHeights[3].first, keyWidthsHeights[3].second});
                row.push_back(0);
                continue;
            }

            // Weapon handling
            if(token == "K"){
                weaponsPositions[1] = {row.size(), rowIndex};
                weaponTypeToSpriteID[1] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, weaponsTextures[0],
                 weaponWidthsHeights[1].first, weaponWidthsHeights[1].second});
                row.push_back(0);
                continue;
            }
            else if(token == "P"){
                weaponsPositions[2] = {row.size(), rowIndex};
                weaponTypeToSpriteID[2] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, weaponsTextures[1],
                 weaponWidthsHeights[2].first, weaponWidthsHeights[2].second});
                row.push_back(0);
                continue;
            }
            else if(token == "S"){
                weaponsPositions[3] = {row.size(), rowIndex};
                weaponTypeToSpriteID[3] = AllSpriteTextures.size();
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, weaponsTextures[2],
                 weaponWidthsHeights[3].first, weaponWidthsHeights[3].second});
                row.push_back(0);
                continue;
            }
            
                
            int value = std::stoi(token);      // parses 10, 12, etc.

            // Door handling
            if (value >= 6 && value <= 9) {
                Door d;
                d.openAmount = 0.0f;
                d.opening = false;

                if (value == 6) { d.locked = false; d.keyType = 0; }
                if (value == 7) { d.locked = true;  d.keyType = 1; }
                if (value == 8) { d.locked = true;  d.keyType = 2; }
                if (value == 9) { d.locked = true;  d.keyType = 3; }

                doors[{ rowIndex, row.size() }] = d;
            }

            row.push_back(value);
        }

        Map.push_back(row);
        rowIndex++;
    }
}


void Game::loadAllTextures(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open texture list file: " << filePath << "\n";
        return;
    }

    enum Section { NONE, WALLS, FLOORS, CEILS, KEYS, WEAPONS };
    Section currentSection = NONE;

    std::string line;
    while (std::getline(file, line)) {

        // Trim leading/trailing spaces
        auto trim = [](std::string &s) {
            while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
        };
        trim(line);
        if (line.empty()) continue;       // Skip blank lines

        std::string low = toLower(line);

        // Detect section headers case-insensitively
        if (low == "[walls]") {
            currentSection = WALLS;
            continue;
        }
        if (low == "[floors]") {
            currentSection = FLOORS;
            continue;
        }
        if (low == "[ceils]" || low == "[ceil]" || low == "[ceilings]") {
            currentSection = CEILS;
            continue;
        }
        if (low == "[keys]") {
            currentSection = KEYS;
            continue;
        }
        if (low == "[weapons]") {
            currentSection = WEAPONS;
            continue;
        }

        // If it’s not a section header, it must be a file path
        if (currentSection == WALLS) {
            addWallTexture(line.c_str());
        }
        else if (currentSection == FLOORS) {
            addFloorTexture(line.c_str());
        }
        else if (currentSection == CEILS) {
            addCeilingTexture(line.c_str());
        }
        else if (currentSection == KEYS) {
            loadKeysTexture(line.c_str());
        }
        else if (currentSection == WEAPONS) {
            loadWeaponsTexture(line.c_str());
        }
        else {
            std::cerr << "Warning: Path found outside any valid section: " << line << "\n";
        }
    }
}


void Game::loadEnemyTextures(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return;
    }

    std::string line;

    while (std::getline(file, line)) {

        // Remove UTF-8 BOM if present (important for first line)
        if (!line.empty() && static_cast<unsigned char>(line[0]) == 0xEF)
            line.erase(0, 3);

        // Remove comments
        auto comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
            line = line.substr(0, comment_pos);

        std::istringstream iss(line);

        int a, b;
        std::string path;

        // Expect: <int> <int> <string>
        if (iss >> a >> b >> path) {
            SDL_Texture* texture = IMG_LoadTexture(renderer.get(), path.c_str());
            if (!texture) {
                std::cerr << "Failed to load texture: " << filePath << " Error: " << IMG_GetError() << std::endl;
                return;
            }
            enemyTextures.insert_or_assign(
                {a, b},
                SDLTexturePtr(texture, SDL_DestroyTexture)
            );
        }
    }
}


void Game::loadEnemies(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open enemy file: " << filePath << '\n';
        return;
    }

    float x, y;
    std::string line;

    while (std::getline(file, line))
    {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        if (!(iss >> x >> y))
        {
            std::cerr << "Invalid enemy entry: " << line << '\n';
            continue;
        }

        addEnemy(x, y, 0.0f);
    }

    file.close();
}

void Game::loadKeysTexture(const char* filePath)
{
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load key texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }
    int keyType = keysTextures.size() + 1;
    int height = 0, width = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        SDL_DestroyTexture(raw);
        return;
    }
    keysTextures.emplace_back(raw, SDL_DestroyTexture);
    keyWidthsHeights[keyType] = std::make_pair(width, height);
}


void Game::loadWeaponsTexture(const char* filePath)
{
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load weapon texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }
    int height = 0, width = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        SDL_DestroyTexture(raw);
        return;
    }
    int weaponsType = weaponsTextures.size() + 1;
    weaponWidthsHeights[weaponsType] = std::make_pair(width, height);
    weaponsTextures.emplace_back(raw, SDL_DestroyTexture);
}   
