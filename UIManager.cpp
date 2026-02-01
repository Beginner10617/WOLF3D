#include "UIManager.hpp"
#include <fstream>
// ---- Static member definitions ----

WeaponType UIManager::currentWeapon = WeaponType::None;
std::vector<KeyType> UIManager::keysHeld = {};
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
UIAnimation UIManager::AvatarAnimation{};
float UIManager::avatarTimer = 0.0f;
int UIManager::avatarFrame = 0;
std::pair<int, int> UIManager::AvatarDimensions = {0, 0};
BitmapFont UIManager::font;
SDL_Rect UIManager::panel = { 0, 0, 0, 0 };

int UIManager::panelHeight = 100;
int UIManager::panelBorderThickness = 1;
SDL_Color UIManager::panelFillColor = {0, 0, 165 ,255};
SDL_Color UIManager::panelBorderColor = {255, 255, 255 ,255};
std::map<HUDSections, int> UIManager::panelSectionWidths = {
    {HUDSections::WEAPON, 192 + 20}, 
    {HUDSections::AMMO, 88}, 
    {HUDSections::AVATAR, 200}, 
    {HUDSections::HEALTH, 132}, 
    {HUDSections::KEYS, 148 + 20} 
};

std::map<WeaponType, SDLTexturePtr> UIManager::panelWeaponImage={};
std::map<WeaponType, std::pair<int, int>> UIManager::panelWeaponImageWH={};

std::map<KeyType, SDLTexturePtr> UIManager::keyUITextures={};
std::map<KeyType, std::pair<int, int>> UIManager::keyUITexturesWH={};

std::vector<Notif> UIManager::UINotification = {};
float UIManager::notifUpdateTimer = 0.0f;

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

    enum Section { NONE, KNIFE, PISTOL, RIFLE, FONT, WEAPONPANEL, KEYS, AVATAR};
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
        if (low == "[weapon]"){
            currentSection = WEAPONPANEL;
            continue;
        }
        if (low == "[keys]"){
            currentSection = KEYS;
            continue;
        }
        if (low == "[avatar]"){
            currentSection = AVATAR;
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
        else if (currentSection == WEAPONPANEL){
            addPanelTextureW(static_cast<WeaponType>(panelWeaponImage.size()+1)
            , line.c_str(), rend);
        }
        else if (currentSection == KEYS){
            addPanelTextureK(static_cast<KeyType>(keyUITextures.size())
            , line.c_str(), rend);
        }
        else if (currentSection == AVATAR){
            addAvatarFrame(line.c_str(), rend);
        }
        else {
            std::cerr << "Warning: Path found outside any valid section: " << line << "\n";
        }
    }
    std::cout<<"ALL HUD TEXTURES LOADED\n";
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

void UIManager::addAvatarFrame(const char* filePath, SDL_Renderer& renderer){
    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath);
    if (!raw) {
        std::cerr << "Failed to Avatar texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }
    if (AvatarDimensions.first == 0 && AvatarDimensions.second == 0){
        int width = 0, height = 0;
        if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
            std::cerr << "Failed to query texture: "
                      << SDL_GetError() << "\n";
            return;
        }
        AvatarDimensions = std::make_pair(width, height);
    }
    AvatarAnimation.frames.emplace_back(raw, SDL_DestroyTexture);
}

void UIManager::update(float deltaTime)
{
    if(UINotification.size())
        notifUpdateTimer += deltaTime;
    if(notifUpdateTimer > notifDuration){
        updateNotifications();
        notifUpdateTimer = 0.0f;
    }

    avatarTimer += deltaTime;
    while (avatarTimer >= frameDuration)
    {
        avatarTimer -= frameDuration;
        avatarFrame++;
        if(avatarFrame >= AvatarAnimation.frames.size()){
            avatarFrame = 0;
        }
    }

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

void UIManager::updateNotifications(){
    if(UINotification.size()){
        UINotification.erase(UINotification.begin());
    }
}

void UIManager::notify(std::string text, SDL_Color color){
    UINotification.push_back({text, color});
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

void UIManager::addKey(KeyType key){
    if(std::count(keysHeld.begin(), keysHeld.end(), key))
        return;
    keysHeld.push_back(key);
}

void UIManager::renderHUD(SDL_Renderer& rend, const std::pair<int,int>& screenSize) {
    // rendering notifs
    int x = 0, y = 0, scale = 1;
    for(auto notif : UINotification){
        renderText(rend, notif.txt, x, y, scale, notif.clr);
        y += font.glyphH * scale;
    }

    panel = {0, screenSize.second - panelHeight, screenSize.first, panelHeight};
    drawFilledRectWithBorder(rend, panel, panelFillColor, panelBorderColor, panelBorderThickness);
    renderWeapon(rend, screenSize, panelHeight);

    // Rendering Datas on Panel
    renderPanelWeaponImage(rend, screenSize);

    // AMMO AND HEALTH TEXT
    SDL_Color clr = {153, 159, 253, 255};
    int ax = 0, hx=0, i = 0, j = (int) HUDSections::HEALTH;y=0;
    while(i<j){
        if(i == (int) HUDSections::AMMO)
            ax = hx;
        hx += panelSectionWidths[static_cast<HUDSections>(i)];
        i++;
    }
    y = screenSize.second - panelHeight;
    drawFilledRectWithBorder(rend,
        {ax, y, panelSectionWidths[HUDSections::AMMO], panelHeight},
        panelFillColor, panelBorderColor, panelBorderThickness
    );
    renderText(rend, "AMMO", ax, y, 1, clr);

    drawFilledRectWithBorder(rend,
        {hx, y, panelSectionWidths[HUDSections::HEALTH], panelHeight},
        panelFillColor, panelBorderColor, panelBorderThickness
    );
    renderText(rend, "HEALTH", hx, y, 1, clr);
    
    // AMMO AND HEALTH VALUE
    y += font.glyphH;
    scale = (panelHeight - font.glyphH)/font.glyphH ; //size
    // centering Y
    y = (y + screenSize.second) / 2 - scale * font.glyphH / 2;

    // WRITING AMMOS
    std::string txt; int width; x = 0;
    //std::cout<<ammo.count(currentWeapon)<<"\n";
    if(ammo.count(currentWeapon)){
        txt = std::to_string(ammo[currentWeapon]);
        width = txt.size() * font.glyphW * scale;
        // centering X
        x = ax + panelSectionWidths[HUDSections::AMMO] / 2;
        x -= width / 2;
        renderText(rend, txt, x, y, scale, clr);
    }

    // WRITING HEALTH
    txt = std::to_string(health);
    width = txt.size() * font.glyphW * scale;
    // centering X
    x = hx + panelSectionWidths[HUDSections::HEALTH] / 2;
    x -= width / 2;
    renderText(rend, txt, x, y, scale, clr);

    // rendering keys
    renderKeys(rend, screenSize);

    // render avatar
    renderAvatar(rend, screenSize);
}

void UIManager::renderAvatar(SDL_Renderer& rend, 
    const std::pair<int,int>& screenSize){
    if(AvatarAnimation.frames.size()==0)
        return;
    int y = screenSize.second - panelHeight, h = panelHeight;
    int x = 0, w = panelSectionWidths[HUDSections::AVATAR];
    int i=0, j = (int) HUDSections::AVATAR;
    while(i<j){
        x += panelSectionWidths[static_cast<HUDSections>(i)];
        i++;
    }
    x+=1;y+=1;w-=2;h-=2;
    // Compute scale to fit inside w x h (no aspect ratio change)
    auto [texW, texH] = AvatarDimensions;
    float scaleX = static_cast<float>(w) / texW;
    float scaleY = static_cast<float>(h) / texH;
    float scale  = std::min(scaleX, scaleY);
    int drawW = static_cast<int>(texW * scale);
    int drawH = static_cast<int>(texH * scale);
    // Center inside panel
    x = x + (w - drawW) / 2;
    y = y + (h - drawH) / 2;
    SDL_Rect dstRect {x, y, drawW, drawH};
    SDL_RenderCopy(
        &rend,
        AvatarAnimation.frames[avatarFrame].get(), 
        nullptr,
        &dstRect
    );
}

void UIManager::renderKeys(
    SDL_Renderer& rend, 
    const std::pair<int,int>& screenSize)
{
    int y = screenSize.second - panelHeight, h = panelHeight / 3;
    int x = 0, w = panelSectionWidths[HUDSections::KEYS];
    int i=0, j = (int) HUDSections::KEYS;
    while(i<j){
        x += panelSectionWidths[static_cast<HUDSections>(i)];
        i++;
    }

    for (KeyType key : keysHeld) {
        int newY = y + ((int)key) * h;

        // Original texture size
        auto [texW, texH] = keyUITexturesWH[key];
        
        // Compute scale to fit inside w x h (no aspect ratio change)
        float scaleX = static_cast<float>(w) / texW;
        float scaleY = static_cast<float>(h) / texH;
        float scale  = std::min(scaleX, scaleY);
        
        int dstW = static_cast<int>(texW * scale);
        int dstH = static_cast<int>(texH * scale);

        // Center horizontally (and vertically within the box)
        int dstX = x + (w - dstW) / 2;
        int dstY = newY + (h - dstH) / 2;

        SDL_Rect dstRect { dstX, dstY, dstW, dstH };

        SDL_RenderCopy(
            &rend,
            keyUITextures[key].get(), 
            nullptr,
            &dstRect
        );
        }

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
        SDL_Rect destRect {destX, destY - yOffset, scaledSize, scaledSize};

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

void UIManager::renderPanelWeaponImage(
    SDL_Renderer& rend,
    const std::pair<int, int>& screenWH
){
    int x = 0;
    int j = static_cast<int>(HUDSections::WEAPON);
    int i = 0;

    // Accumulate X offset for WEAPON section
    while (i < j) {
        x += panelSectionWidths[static_cast<HUDSections>(i)];
        i++;
    }

    // ---------- Panel rect ----------
    SDL_Rect panelRect {
        x,
        screenWH.second - panelHeight,
        panelSectionWidths[HUDSections::WEAPON],
        panelHeight
    };
    drawFilledRectWithBorder(rend, panelRect, panelFillColor, panelBorderColor, panelBorderThickness);
    panelRect.x += 2 * panelBorderThickness;
    panelRect.w -= 4 * panelBorderThickness;

    // ---------- Fetch texture ----------
    auto texIt = panelWeaponImage.find(currentWeapon);
    if (texIt == panelWeaponImage.end())
        return;

    SDL_Texture* texture = texIt->second.get();

    // ---------- Fetch image size ----------
    auto whIt = panelWeaponImageWH.find(currentWeapon);
    if (whIt == panelWeaponImageWH.end())
        return;

    int imgW = whIt->second.first;
    int imgH = whIt->second.second;

    // ---------- Aspect-ratio fit ----------
    float scaleX = static_cast<float>(panelRect.w) / imgW;
    float scaleY = static_cast<float>(panelRect.h) / imgH;
    float scale  = std::min(scaleX, scaleY);

    int drawW = static_cast<int>(imgW * scale);
    int drawH = static_cast<int>(imgH * scale);

    // ---------- Center inside panel ----------
    int drawX = panelRect.x + (panelRect.w - drawW) / 2;
    int drawY = panelRect.y + (panelRect.h - drawH) / 2;

    SDL_Rect srcRect  { 0, 0, imgW, imgH };
    SDL_Rect destRect { drawX, drawY, drawW, drawH };

    SDL_RenderCopy(&rend, texture, &srcRect, &destRect);
}

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

// UI Helper
void UIManager::drawFilledRectWithBorder(
    SDL_Renderer& renderer,
    const SDL_Rect& rect,
    SDL_Color fillColor,
    SDL_Color borderColor,
    int borderThickness
) {
    // Clamp border thickness
    if (borderThickness < 0) borderThickness = 0;
    if (borderThickness * 2 > rect.w) borderThickness = rect.w / 2;
    if (borderThickness * 2 > rect.h) borderThickness = rect.h / 2;

    // ---------- Fill ----------
    SDL_SetRenderDrawColor(&renderer,
                           fillColor.r, fillColor.g,
                           fillColor.b, fillColor.a);
    SDL_RenderFillRect(&renderer, &rect);

    if (borderThickness == 0) return;

    // ---------- Border ----------
    SDL_SetRenderDrawColor(&renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);

    // Top
    SDL_Rect top = {
        rect.x,
        rect.y,
        rect.w,
        borderThickness
    };

    // Bottom
    SDL_Rect bottom = {
        rect.x,
        rect.y + rect.h - borderThickness,
        rect.w,
        borderThickness
    };

    // Left
    SDL_Rect left = {
        rect.x,
        rect.y + borderThickness,
        borderThickness,
        rect.h - 2 * borderThickness
    };

    // Right
    SDL_Rect right = {
        rect.x + rect.w - borderThickness,
        rect.y + borderThickness,
        borderThickness,
        rect.h - 2 * borderThickness
    };

    SDL_RenderFillRect(&renderer, &top);
    SDL_RenderFillRect(&renderer, &bottom);
    SDL_RenderFillRect(&renderer, &left);
    SDL_RenderFillRect(&renderer, &right);
}

void UIManager::addPanelTextureW(WeaponType weapon, const char* filePath, SDL_Renderer& renderer){
    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath);
    if (!raw) {
        std::cerr << "Failed to load Weapon Panel texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    panelWeaponImage.emplace(
        weapon,
        SDLTexturePtr(raw, SDL_DestroyTexture)
    );
    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        return;
    }

    panelWeaponImageWH[weapon] = std::make_pair(width, height);
}

void UIManager::addPanelTextureK(KeyType key, 
    const char* filePath, SDL_Renderer& renderer)
{
    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath);
    if (!raw) {
        std::cerr << "Failed to load Key Panel texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    keyUITextures.emplace(
        key,
        SDLTexturePtr(raw, SDL_DestroyTexture)
    );
    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        return;
    }

    keyUITexturesWH[key] = std::make_pair(width, height);
}

std::pair<int, int> UIManager::getGlyphSize(){
    return {font.glyphW, font.glyphH};
}

void UIManager::reset(){
    currentWeapon = WeaponType::None;
    keysHeld.clear();
    ammo.clear();
    
}