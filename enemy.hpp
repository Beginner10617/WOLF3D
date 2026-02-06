#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#define PI 3.1415926535f
// HERE ANGLES ARE TAKEN POSITIVE ANTI-CLOCKWISE FROM TOP CONTRARY TO THE PLAYER
enum EnemyState {
    ENEMY_IDLE,
    ENEMY_WALK,
    ENEMY_SHOOT,
    ENEMY_PAIN,
    ENEMY_DEAD
};

void load_enemy_textures(
    const std::string& filename,
    std::map<std::pair<int, int>, std::string>& Textures
);

class Enemy {
    EnemyState state; 
    bool walking = false;
    float walk_segment_length = 1.5f;
    int spriteID;

    // perception & memory
    bool alerted = false;
    bool canSeePlayer = false;
    bool justTookDamage = false;
    bool isDead = false;
    bool stateLocked = false;
    int canWalkThisFrame = 1;
    int damageThisFrame = 0;

    // combat stats
    int health = 100;
    int baseDamage = 10, damageSpread = 5;
    int attackChanceDivisor = 2;
    int accuracyDivisor = 3;   // 1 in 6 chance 
    int painChanceDivisor = 5; // 1 in 4 chance
    float walk_angle_error = 10.0f * M_PI / 180.0f; // ±10 degrees
    float attackRange = 7.0f;
    int doorOpenChanceDivisor = 3; // 1 in 3 chance

    // AI timing
    float thinkTimer = 0.0f;
    float thinkInterval = 0.5f;

    // Door opening
    bool wantToOpenThisFrame = false;

    std::pair<float, float> position, destinationOfWalk;
    float angle, sze=0.5f, moveSpeed = 1.0f, DurationPerSprite = 0.25f, fracTime = 0.0f;
    int currentFrame = 0, frameIndex = 0, directionNum;
    std::map<EnemyState, std::vector<int>> Animations;
public:
    Enemy(float x, float y, float theta);
    std::pair<float, float> get_position() const;
    float get_size() const;
    float get_angle() const;
    void _process(float deltaTime, const std::pair<float, float>& pos, float playerAngle);
    void addFrame(EnemyState s, int frame);
    void addFrames(const std::map<EnemyState, std::vector<int>>& Anim);
    void setAnimState(EnemyState s, bool );
    void init(int spriteID_);
    void updateDirnNumWrt(const std::pair<float, float>& pos);
    int get_current_frame() const;
    int get_dirn_num() const;
    void moveNextFrame();
    void walkTo(float x, float y);
    void alert();
    bool canEnterPain();
    bool randomAttackChance(int);
    void think(const std::pair<float, float>& pos, float playerAngle);
    void updateCanSeePlayer(bool);
    bool takeDamage(int);
    int computeEnemyHitChance(float dist);
    int rollEnemyDamage();
    int getDamageThisFrame() const { return damageThisFrame; }
    void clearDamageThisFrame() { damageThisFrame = 0; }
    bool isAlerted() const { return alerted; }
    int get_spriteID() const { return spriteID; }
    std::pair<float, float> askGameToMove(float deltaTime);
    void cancelWalkThisFrame(bool door);
    void allowWalkNextFrame() { canWalkThisFrame = 1; }
    bool get_isDead() const { return isDead; }
    bool canOpenDoor();
    void openDoor();
    bool get_wantToOpenDoor() const { return wantToOpenThisFrame; }
    std::pair<int, int> nearbyDoor();
    void reset_wantToOpenThisFrame() { wantToOpenThisFrame = false; }
    void reset(){
        state = ENEMY_IDLE;
        alerted = false;
        isDead = false;
        health = 100;
        thinkTimer = 0.0f;
        stateLocked = false;
    }
    void setPosition(std::pair<float, float> x){position=x;}
};
