#include "UIManager.hpp"
#include <fstream>
#include <sstream>
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

BitmapFont UIManager::font;

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

    enum Section { NONE, KNIFE, PISTOL, RIFLE, FONT};
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
        if (low == "[font]"){
            currentSection = FONT;
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
        else if (currentSection == FONT) {
            // Load and use only one FONT Everywhere
            loadFont(line.c_str(), rend);
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
    renderWeapon(rend, screenSize, 50);
    
    renderText(rend, "HELLO", 0, 0, 1, SDL_Color{255, 0, 0, 255});
}

void UIManager::renderWeapon(SDL_Renderer& rend, const std::pair<int,int>& screenSize, int yOffset){
    auto [screenWidth, screenHeight] = screenSize;
    auto& anim = weaponAnimations[currentWeapon];
    if(!anim.frames.empty()){
        // Draw Animation frame    
        int imgSize = anim.height; // square
        float scale = 0.6f * screenHeight / imgSize; 
        int scaledSize = static_cast<int>(imgSize * scale);
        int destX = (screenWidth - scaledSize) / 2;
        int destY = screenHeight - scaledSize; // bottom

        SDL_Rect srcRect {0, 0, imgSize, imgSize};
        SDL_Rect destRect {destX, destY - yOffset, scaledSize, scaledSize - yOffset};

        SDL_RenderCopy(&rend, anim.frames[currentFrame].get(), &srcRect, &destRect);
    }
}

void UIManager::renderText(
    SDL_Renderer& renderer,
    const std::string& text,
    int x, int y,
    int scale,
    SDL_Color color
) {
    // Apply color modulation ONCE
    SDL_SetTextureColorMod(
        font.texture.get(),
        color.r, color.g, color.b
    );
    SDL_SetTextureAlphaMod(font.texture.get(), color.a);

    int cursorX = x;

    for (char c : text) {
        int index = getGlyphIndex(c);
        if (index < 0) {
            cursorX += font.glyphW * scale;
            continue;
        }

        SDL_Rect src {
            index * font.glyphW,
            0,
            font.glyphW,
            font.glyphH
        };

        SDL_Rect dst {
            cursorX,
            y,
            font.glyphW * scale,
            font.glyphH * scale
        };

        SDL_RenderCopy(&renderer, font.texture.get(), &src, &dst);
        cursorX += dst.w;
    }

    // Reset modulation to avoid affecting other draws
    SDL_SetTextureColorMod(font.texture.get(), 255, 255, 255);
    SDL_SetTextureAlphaMod(font.texture.get(), 255);
}

// Font Utils

int UIManager::getGlyphIndex(char c) {
    c = std::toupper(c);
    auto pos = UIManager::font.charset.find(c);
    if (pos == std::string::npos)
        return -1;
    return static_cast<int>(pos);
}

void UIManager::loadFont(const char* charsetAndFilePath, SDL_Renderer& renderer)
{
    std::string input = charsetAndFilePath;

    // Split at last space
    size_t splitPos = input.find_last_of(' ');
    if (splitPos == std::string::npos) {
        std::cerr << "Font load error: Invalid format\n";
        return;
    }

    std::string charset = input.substr(0, splitPos);
    std::string filePath = input.substr(splitPos + 1);

    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath.c_str());
    if (!raw) {
        std::cerr << "Failed to load font texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    int texW, texH;
    SDL_QueryTexture(raw, nullptr, nullptr, &texW, &texH);

    int glyphCount = static_cast<int>(charset.size());
    if (glyphCount == 0) {
        std::cerr << "Font load error: Empty charset\n";
        SDL_DestroyTexture(raw);
        return;
    }

    // Assuming a single row font sheet
    int glyphW = texW / glyphCount;
    int glyphH = texH;

    font.texture = SDLTexturePtr(raw, SDL_DestroyTexture);
    font.glyphW  = glyphW;
    font.glyphH  = glyphH;
    font.charset = charset;

    std::cout << "Loaded bitmap font: "
              << glyphCount << " glyphs ("
              << glyphW << "x" << glyphH << ")\n";
}

