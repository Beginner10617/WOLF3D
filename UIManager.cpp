#include "UIManager.hpp"
#include <fstream>
// ---- Static member definitions ----

WeaponType UIManager::currentWeapon = WeaponType::None;
std::map<WeaponType, int> UIManager::ammo = {
    { WeaponType::Pistol, 0 },
    { WeaponType::Rifle,  0 }
};
int UIManager::health = 100;

float UIManager::weaponAnimTimer = 0.0f;
int UIManager::currentFrame = IDLE_FRAME;
bool UIManager::animating = false;

std::map<WeaponType, UIAnimation> UIManager::weaponAnimations = {
    { WeaponType::Knife,  UIAnimation{} },
    { WeaponType::Pistol, UIAnimation{} },
    { WeaponType::Rifle,  UIAnimation{} }
};

static const char* weaponTypeToString(WeaponType w)
{
    switch (w) {
        case WeaponType::Knife:  return "Knife";
        case WeaponType::Pistol: return "Pistol";
        case WeaponType::Rifle:  return "Rifle";
        case WeaponType::None:   return "None";
        default:                 return "Unknown";
    }
}

void UIManager::loadTextures(const char* filePath, SDL_Renderer& rend){
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open texture list file: " << filePath << "\n";
        return;
    }

    enum Section { NONE, KNIFE, PISTOL, RIFLE};
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
        if (low == "[knife]") {
            currentSection = KNIFE;
            continue;
        }
        if (low == "[pistol]") {
            currentSection = PISTOL;
            continue;
        }
        if (low == "[rifle]") {
            currentSection = RIFLE;
            continue;
        }

        // If it’s not a section header, it must be a file path
        if (currentSection == KNIFE) {
            addTexture(WeaponType::Knife, line.c_str(), rend);
        }
        else if (currentSection == PISTOL) {
            addTexture(WeaponType::Pistol, line.c_str(), rend);
        }
        else if (currentSection == RIFLE) {
            addTexture(WeaponType::Rifle, line.c_str(), rend);   
        }
        else {
            std::cerr << "Warning: Path found outside any valid section: " << line << "\n";
        }
    }
}

void UIManager::addTexture(WeaponType weapon, const char* filePath, SDL_Renderer& renderer){
    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath);
    if (!raw) {
        std::cerr << "Failed to load Weapon HUD texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    weaponAnimations[weapon].frames.emplace_back(raw, SDL_DestroyTexture);
}

void UIManager::update(float deltaTime)
{
    if (!animating) return;

    auto& anim = weaponAnimations[currentWeapon];
    if (anim.frames.empty()) return;

    weaponAnimTimer += deltaTime;
    
    while (weaponAnimTimer >= frameDuration)
    {
        weaponAnimTimer -= frameDuration;
        currentFrame++;

        // Stop exactly at idle frame
        if (currentFrame >= anim.frames.size())
        {
            currentFrame = IDLE_FRAME;
            animating = false;
            break;
        }
    }
}

void UIManager::animateOneShot()
{
    auto& anim = weaponAnimations[currentWeapon];
    if (anim.frames.size() <= IDLE_FRAME + 1)
        return;

    currentFrame = IDLE_FRAME + 1;
    weaponAnimTimer = 0.0f;
    animating = true;
}

void UIManager::setWeapon(WeaponType weapon){
    currentWeapon = weapon;
    currentFrame = IDLE_FRAME;
    animating = false;
}
void UIManager::setAmmo(const char weaponChar, int num){
    if(weaponChar == 'P')
        ammo[WeaponType::Pistol] = num;
    else if(weaponChar == 'S')
        ammo[WeaponType::Rifle] = num;
}
void UIManager::setHealth(int hp){health = hp;}

void UIManager::renderHUD(SDL_Renderer& rend, const std::pair<int,int>& screenSize) {
    auto [screenWidth, screenHeight] = screenSize;
    auto& anim = weaponAnimations[currentWeapon];
    if(anim.frames.empty()){
        //std::cout << "Animation frames are empty for weapon: "
        //  << weaponTypeToString(currentWeapon) << "\n";

        return;
    }
    int imgSize = anim.height; // square
    float scale = 0.6f * screenHeight / imgSize; 
    int scaledSize = static_cast<int>(imgSize * scale);
    int destX = (screenWidth - scaledSize) / 2;
    int destY = screenHeight - scaledSize; // bottom

    SDL_Rect srcRect {0, 0, imgSize, imgSize};
    SDL_Rect destRect {destX, destY, scaledSize, scaledSize};

    SDL_RenderCopy(&rend, anim.frames[currentFrame].get(), &srcRect, &destRect);
}
