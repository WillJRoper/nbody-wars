#include "entity.h"
#include <cmath>

// Ship implementation
Ship::Ship() {
    type = EntityType::SHIP;
    wraps = true;
    radius = 10.0f;
    mass = 1500.0f;  // Comparable to Tiny asteroid for N-body interactions
    lives = 3;
    score = 0;
    angle = -1.57079632f;  // Point up
    thrusting = false;
    invulnerable = false;
    invulnerableTime = 0;
    shootCooldown = 0;
}

void Ship::init(int id, Vec2 position, int player) {
    this->id = id;
    pos = position;
    vel = Vec2(0, 0);
    acc = Vec2(0, 0);
    playerId = player;
    active = true;
    lives = 3;
    score = 0;
    invulnerable = true;
    invulnerableTime = 3.0f;
}

void Ship::rotate(float deltaAngle) {
    angle += deltaAngle;
}

void Ship::thrust(float power, float dt) {
    Vec2 direction(std::cos(angle), std::sin(angle));
    vel += direction * (power * dt);
}

void Ship::update(float dt) {
    if (invulnerable) {
        invulnerableTime -= dt;
        if (invulnerableTime <= 0) {
            invulnerable = false;
            invulnerableTime = 0;
        }
    }
    if (shootCooldown > 0) {
        shootCooldown -= dt;
    }
}

// Asteroid implementation
Asteroid::Asteroid() {
    type = EntityType::ASTEROID;
    wraps = true;
    vertices = 8;
    rotation = 0;
    rotationSpeed = 0;
}

void Asteroid::init(int entityId, Vec2 position, Vec2 velocity, int asteroidSize, float baseMass) {
    id = entityId;
    pos = position;
    vel = velocity;
    acc = Vec2(0, 0);
    size = asteroidSize;
    active = true;

    // Size determines radius and mass (mass halves with each size)
    switch (size) {
        case 0: // Large
            radius = 40.0f;
            mass = baseMass;
            break;
        case 1: // Medium
            radius = 25.0f;
            mass = baseMass * 0.5f;
            break;
        case 2: // Small
            radius = 15.0f;
            mass = baseMass * 0.25f;
            break;
        case 3: // Tiny
            radius = 10.0f;
            mass = baseMass * 0.125f;
            break;
        case 4: // Micro
            radius = 6.0f;
            mass = baseMass * 0.0625f;
            break;
        case 5: // Dust
            radius = 3.0f;
            mass = baseMass * 0.03125f;
            break;
        default:
            radius = 30.0f;
            mass = baseMass * 0.75f;
    }

    rotationSpeed = (rand() % 100 - 50) * 0.01f;
}

void Asteroid::update(float dt) {
    rotation += rotationSpeed * dt;
}

// Bullet implementation
Bullet::Bullet() {
    type = EntityType::BULLET;
    wraps = true;
    radius = 2.0f;
    mass = 100.0f;  // Light but participates in N-body
    maxLifetime = 3.0f;
}

void Bullet::init(int entityId, Vec2 position, Vec2 velocity, int player) {
    id = entityId;
    pos = position;
    vel = velocity;
    acc = Vec2(0, 0);
    playerId = player;
    lifetime = maxLifetime;
    active = true;
}

void Bullet::update(float dt) {
    lifetime -= dt;
    if (lifetime <= 0) {
        active = false;
    }
}

// BlackHole implementation
BlackHole::BlackHole() {
    type = EntityType::BLACK_HOLE;
    wraps = false;  // Black holes don't wrap
    visualRadius = 15.0f;
}

void BlackHole::init(int entityId, Vec2 position, Vec2 velocity, float bhMass, float bhAccretionRadius) {
    id = entityId;
    pos = position;
    vel = velocity;
    acc = Vec2(0, 0);
    mass = bhMass;
    accretionRadius = bhAccretionRadius;
    active = true;
}

bool BlackHole::isOffscreen(float worldWidth, float worldHeight) const {
    float margin = 100.0f;
    return pos.x < -margin || pos.x > worldWidth + margin ||
           pos.y < -margin || pos.y > worldHeight + margin;
}

// Particle implementation
Particle::Particle() {
    type = EntityType::PARTICLE;
    wraps = false;
    mass = 0.1f;
    maxLifetime = 1.0f;
    playerId = -1;  // Default: white
}

void Particle::init(Vec2 position, Vec2 velocity, int particlePlayerId) {
    pos = position;
    vel = velocity;
    acc = Vec2(0, 0);
    lifetime = maxLifetime;
    active = true;
    playerId = particlePlayerId;
}

void Particle::update(float dt) {
    // Move particle based on initial velocity (no gravity)
    pos += vel * dt;

    // Decrease lifetime and deactivate when expired
    lifetime -= dt;
    if (lifetime <= 0) {
        active = false;
    }
}
