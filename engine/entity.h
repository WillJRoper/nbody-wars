#pragma once
#include "vec2.h"
#include <vector>
#include <cstdint>

enum class EntityType {
    SHIP,
    ASTEROID,
    BULLET,
    BLACK_HOLE,
    PARTICLE
};

// Base physics body
struct Body {
    Vec2 pos;
    Vec2 vel;
    Vec2 acc;
    float mass;
    EntityType type;
    bool wraps;  // Whether this entity wraps at boundaries
    bool active;
    int id;

    Body() : mass(0), type(EntityType::ASTEROID), wraps(true), active(true), id(0) {}
};

struct Ship : public Body {
    int playerId;
    float angle;
    float radius;
    int lives;
    int score;
    bool thrusting;
    bool invulnerable;
    float invulnerableTime;
    float shootCooldown;

    Ship();
    void init(int id, Vec2 pos, int playerId);
    void rotate(float deltaAngle);
    void thrust(float power, float dt);
    bool canShoot() const { return shootCooldown <= 0; }
    void shoot() { shootCooldown = 0.2f; }
    void update(float dt);
};

struct Asteroid : public Body {
    float radius;
    int size;  // 0=large, 1=medium, 2=small
    int vertices;
    float rotation;
    float rotationSpeed;

    Asteroid();
    void init(int id, Vec2 pos, Vec2 vel, int size, float baseMass = 12000.0f);
    void update(float dt);
};

struct Bullet : public Body {
    int playerId;
    float lifetime;
    float maxLifetime;
    float radius;

    Bullet();
    void init(int id, Vec2 pos, Vec2 vel, int playerId);
    void update(float dt);
};

struct BlackHole : public Body {
    float accretionRadius;
    float visualRadius;

    BlackHole();
    void init(int id, Vec2 pos, Vec2 vel, float mass, float accretionRadius);
    bool isOffscreen(float worldWidth, float worldHeight) const;
};

struct Particle : public Body {
    float lifetime;
    float maxLifetime;
    int playerId;  // -1 for white (asteroids), 0/1 for ship colors

    Particle();
    void init(Vec2 pos, Vec2 vel, int playerId = -1);
    void update(float dt);
};
