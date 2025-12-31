#include "UIManager.hpp"
#include <fstream>
// ---- Static member definitions ----

WeaponType UIManager::currentWeapon = WeaponType::None;
int UIManager::ammo = 0;
int UIManager::health = 100;

float UIManager::weaponAnimTimer = 0.0f;
int UIManager::currentFrame = IDLE_FRAME;
bool UIManager::animating = false;

std::map<WeaponType, UIAnimation> UIManager::weaponAnimations = {
    { WeaponType::Knife,  UIAnimation{} },
    { WeaponType::Pistol, UIAnimation{} },
    { WeaponType::Rifle,  UIAnimation{} }
};

void UIManager::loadTextures(const char* filePath){
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
            
        }
        else if (currentSection == PISTOL) {
            
        }
        else if (currentSection == RIFLE) {
            
        }
        else {
            std::cerr << "Warning: Path found outside any valid section: " << line << "\n";
        }
    }
}