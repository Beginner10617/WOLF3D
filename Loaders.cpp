#include "Game.hpp"
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
            
            // Health Pack handling
            if(token == "h"){
                int spriteID = AllSpriteTextures.size();
                HealthPackPositions[{1, spriteID}] = {row.size(), rowIndex};

                AllSpriteTextures.push_back(Sprite{ static_cast<int>(spriteID), std::pair<float, float>{row.size(), rowIndex}, healthPackTextures[0],
                 healthPackWidthsHeights[1].first, healthPackWidthsHeights[1].second});
                row.push_back(0);
                continue;
            }
            else if(token == "H"){
                int spriteID = AllSpriteTextures.size();
                HealthPackPositions[{2, spriteID}] = {row.size(), rowIndex};
                
                AllSpriteTextures.push_back(Sprite{ static_cast<int>(AllSpriteTextures.size()), std::pair<float, float>{row.size(), rowIndex}, healthPackTextures[1],
                 healthPackWidthsHeights[2].first, healthPackWidthsHeights[2].second});
                row.push_back(0);
                continue;
            }

            // Handling Ammos
            if(token == "a"){
                int spriteID = AllSpriteTextures.size();
                AmmoPackPositions[{1, spriteID}] = {row.size(), rowIndex};
                AllSpriteTextures.push_back(Sprite{ spriteID, std::pair<float, float>{row.size(), rowIndex}, ammoPackTextures[0],
                 ammoPackWidthsHeights[1].first, ammoPackWidthsHeights[1].second});
                row.push_back(0);
                continue;
            }
            else if(token == "A"){
                int spriteID = AllSpriteTextures.size();
                AmmoPackPositions[{2, spriteID}] = {row.size(), rowIndex};
                AllSpriteTextures.push_back(Sprite{ spriteID, std::pair<float, float>{row.size(), rowIndex}, ammoPackTextures[1],
                 ammoPackWidthsHeights[2].first, ammoPackWidthsHeights[2].second});
                row.push_back(0);
                continue;
            }
                
            // All other alphabets are taken as decoratives
            char x = '0';
            if(token.size()==1)
                x = token.c_str()[0];
            if((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z')){
                if (DecorationTextures.find(x) != DecorationTextures.end()) {
                    int spriteID = AllSpriteTextures.size();
                    AllSpriteTextures.push_back(
                        Sprite{
                            spriteID, 
                            std::pair<float, float>{row.size(), rowIndex},
                            DecorationTextures[x],
                            DecorationTextureWidthsHeights[x].first,
                            DecorationTextureWidthsHeights[x].second,
                        }
                    );
                    row.push_back(0);
                    continue;
                }
            }
            int value;
            try{
                value = std::stoi(token);      // parses 10, 12, etc.
            }
            catch(const std::runtime_error& e){
                std::cout << "Can't load map, erroneous data\n"<< e.what() << std::endl;
                continue;
            }
            // Door handling
            if (value >= 6 && value <= 9) {
                Door d;
                d.openAmount = 0.0f;
                d.opening = false;

                if (value == 6) { d.locked = false; d.keyType = 0; }
                if (value == 7) { d.locked = true;  d.keyType = 1; }
                if (value == 8) { d.locked = true;  d.keyType = 2; }
                if (value == 9) { d.locked = true;  d.keyType = 3; }

                doors[{ row.size(), rowIndex }] = d;
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

    enum Section { NONE, WALLS, KEYS, WEAPONS, HEALTHPACKS, AMMOPACKS, DOORFRAME };
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
        if (low == "[keys]") {
            currentSection = KEYS;
            continue;
        }
        if (low == "[weapons]") {
            currentSection = WEAPONS;
            continue;
        }
        if (low == "[healthpacks]") {
            currentSection = HEALTHPACKS;
            continue;
        }
        if (low == "[ammopacks]"){
            currentSection = AMMOPACKS;
            continue;
        }
        if (low == "[doorframe]"){
            currentSection = DOORFRAME;
            continue;
        }

        // If it’s not a section header, it must be a file path
        if (currentSection == WALLS) {
            addWallTexture(line.c_str());
        }
        else if (currentSection == KEYS) {
            loadKeysTexture(line.c_str());
        }
        else if (currentSection == WEAPONS) {
            loadWeaponsTexture(line.c_str());
        }
        else if (currentSection == HEALTHPACKS) {
            loadHealthPackTexture(line.c_str());
        }
        else if (currentSection == AMMOPACKS) {
            loadAmmoPackTexture(line.c_str());
        }
        else if (currentSection == DOORFRAME){
            loadDoorFrame(line.c_str());
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
        enemyLoadLocations.push_back(std::make_pair(x, y));
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

void Game::loadHealthPackTexture(const char* filePath)
{
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load health pack texture: "
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
    int healthPackType = healthPackTextures.size() + 1;
    healthPackWidthsHeights[healthPackType] = std::make_pair(width, height);
    healthPackTextures.emplace_back(raw, SDL_DestroyTexture);
}
void Game::loadAmmoPackTexture(const char* filePath){
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load health pack texture: "
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
    int ammoPackType = ammoPackTextures.size() + 1;
    ammoPackWidthsHeights[ammoPackType] = std::make_pair(width, height);
    ammoPackTextures.emplace_back(raw, SDL_DestroyTexture);
}

void Game::loadDecorationTextures(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open decoration texture file: "
                  << filePath << "\n";
        return;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;

        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos)
            continue;

        // Ignore comments
        if (line[start] == '#')
            continue;

        // Format: <char> : <filepath>
        size_t colon = line.find(':', start);
        if (colon == std::string::npos) {
            std::cerr << "Invalid format at line "
                      << lineNumber << "\n";
            continue;
        }

        // Extract key
        std::string keyStr = line.substr(start, colon - start);
        // Trim whitespace from keyStr (both ends)
        size_t keyStart = keyStr.find_first_not_of(" \t");
        size_t keyEnd   = keyStr.find_last_not_of(" \t");

        if (keyStart == std::string::npos) {
            std::cerr << "Invalid key at line "
                    << lineNumber << "\n";
            continue;
        }

        keyStr = keyStr.substr(keyStart, keyEnd - keyStart + 1);

        if (keyStr.length() != 1) {
            std::cerr << "Invalid key at line "
                      << lineNumber << "\n";
            continue;
        }

        char key = keyStr[0];

        // Extract filepath
        size_t pathStart = line.find_first_not_of(" \t", colon + 1);
        if (pathStart == std::string::npos) {
            std::cerr << "Missing filepath at line "
                      << lineNumber << "\n";
            continue;
        }

        std::string texturePath = line.substr(pathStart);

        // Load the texture
        addDecorationTexture(key, texturePath.c_str());
    }

    std::cout << "Decoration textures loaded successfully\n";
}
void Game::loadDoorFrame(const char* filePath)
{
    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filePath);
    if (!raw) {
        std::cerr << "Failed to load Door frame texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        SDL_DestroyTexture(raw);
        return;
    }

    // Wrap with shared_ptr + SDL_DestroyTexture deleter
    DOOR_FRAME = SDLTexturePtr(raw, SDL_DestroyTexture);

    // store dimensions
    doorFrameWidthHeight  = std::make_pair(width, height);
}