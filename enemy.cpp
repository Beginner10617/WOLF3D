#include "enemy.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

bool parse_enemy_state(const std::string& name, EnemyState& out) {
    std::string n = to_lower(name);

    if (n == "enemy_idle" || n == "idle") {
        out = ENEMY_IDLE;
        return true;
    }
    if (n == "enemy_walk" || n == "walk") {
        out = ENEMY_WALK;
        return true;
    }
    return false;
}

void load_enemy_textures(
    const std::string& filename,
    std::map<std::pair<int, int>, std::string>& Textures
) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
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
            Textures[{a, b}] = path;
        }
        // else: silently ignore malformed / empty lines
    }
}

Enemy::Enemy(float x, float y, float theta)
    : position(x, y), angle(theta) {}

std::pair<float, float> Enemy::get_position(){
    return position;
}

float Enemy::get_angle(){
    return angle;
}

void Enemy::_process(float deltaTime) {
    fracTime += deltaTime;
    while (fracTime > DurationPerSprite) {
        currentFrame++;
        fracTime -= DurationPerSprite;
    }

    if (!Animations[state].empty()) {
        currentFrame %= Animations[state].size();
    }
}

void Enemy::addFrame(EnemyState s, int frame) {
    Animations[s].push_back(frame);
}

void Enemy::addFrames(std::map<EnemyState, std::vector<int>>& Anim) {
    for (int x : Anim[ENEMY_IDLE])
        Animations[ENEMY_IDLE].push_back(x);

    for (int x : Anim[ENEMY_WALK])
        Animations[ENEMY_WALK].push_back(x);
}
