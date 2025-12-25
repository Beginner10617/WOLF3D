#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#define PI 3.1415926535f
// HERE ANGLES ARE TAKEN POSITIVE ANTI-CLOCKWISE FROM TOP CONTRARY TO THE PLAYER
enum EnemyState {
    ENEMY_IDLE,
    ENEMY_WALK,
    ENEMY_SHOOT,
    ENEMY_PAIN,
    ENEMY_DEAD
};

bool parse_enemy_state(const std::string& name, EnemyState& out);

void load_enemy_textures(
    const std::string& filename,
    std::map<std::pair<int, int>, std::string>& Textures
);

class Enemy {
    EnemyState state; 
    bool walking = false;

    // perception & memory
    bool alerted = false;
    bool canSeePlayer = false;
    bool justTookDamage = false;

    // combat stats
    int health = 100;
    int baseDamage = 10;
    int accuracyDivisor = 6;   // 1 in 6 chance 
    int painChanceDivisor = 4; // 1 in 4 chance

    // AI timing
    float thinkTimer = 0.0f;
    float thinkInterval = 0.2f;

    // attack pacing
    float attackCooldown = 0.0f;
    float attackDelay = 0.6f;

    // pain control
    float painTimer = 0.0f;
    float painDuration = 0.25f;

    std::pair<float, float> position, destinationOfWalk;
    float angle, sze=1.0f, moveSpeed = 1.0f, DurationPerSprite = 0.25f, fracTime = 0.0f;
    int currentFrame = 0, frameIndex = 0, directionNum;
    std::map<EnemyState, std::vector<int>> Animations;
public:
    Enemy(float x, float y, float theta);
    std::pair<float, float> get_position();
    float get_size();
    float get_angle();
    void _process(float deltaTime);
    void addFrame(EnemyState s, int frame);
    void addFrames(const std::map<EnemyState, std::vector<int>>& Anim);
    void setAnimState(EnemyState s);
    void init();
    void updateDirnNumWrt(std::pair<float, float> pos);
    int get_current_frame();
    int get_dirn_num();
    void moveNextFrame();
    void walkTo(float x, float y);
};
