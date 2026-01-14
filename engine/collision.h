#pragma once
#include "entity.h"
#include "quadtree.h"
#include <vector>

struct CollisionPair {
    Body* a;
    Body* b;
    float distance;
};

class CollisionDetector {
public:
    CollisionDetector(float worldWidth, float worldHeight);

    // Detect all collisions
    void detectCollisions(
        std::vector<Ship>& ships,
        std::vector<Asteroid>& asteroids,
        std::vector<Bullet>& bullets,
        std::vector<BlackHole>& blackHoles,
        std::vector<CollisionPair>& outCollisions
    );

private:
    float worldWidth, worldHeight;

    bool checkCollision(Body* a, Body* b, float radiusA, float radiusB, float& outDistance);
    Vec2 getMinimumImagePos(const Vec2& pos, const Vec2& reference);
};

// Collision response handler
class CollisionHandler {
public:
    CollisionHandler(float worldWidth, float worldHeight);

    // Handle different collision types
    void handleShipAsteroid(Ship* ship, Asteroid* asteroid, std::vector<Particle>& particles);
    void handleShipShip(Ship* ship1, Ship* ship2);
    void handleAsteroidAsteroid(Asteroid* a1, Asteroid* a2);
    void handleBulletAsteroid(Bullet* bullet, Asteroid* asteroid, std::vector<Particle>& particles, std::vector<Asteroid>& asteroids, int& nextId);
    void handleBlackHoleAccretion(Body* body, BlackHole* blackHole, std::vector<Particle>& particles);

private:
    float worldWidth, worldHeight;

    void mergeAsteroids(Asteroid* a1, Asteroid* a2);
    void mergeBulletIntoAsteroid(Bullet* bullet, Asteroid* asteroid);
    void createExplosion(Vec2 pos, int count, std::vector<Particle>& particles);
};
