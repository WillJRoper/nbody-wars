/**
 * @file collision.cpp
 * @brief Implementation of collision detection and response systems
 *
 * Handles all collision logic for the game including:
 * - Detection using radius-based overlap tests with periodic boundaries
 * - Response for all entity pair types (elastic collisions, merging, damage)
 * - Explosion particle generation with color coding
 * - Asteroid splitting mechanics
 * - Black hole accretion
 *
 * Uses minimum image convention for periodic boundaries to correctly
 * detect collisions across domain wraps.
 */

#include "collision.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// CollisionDetector implementation
// ============================================================================

/**
 * @brief Construct collision detector
 * @param worldWidth Width of periodic domain
 * @param worldHeight Height of periodic domain
 */
CollisionDetector::CollisionDetector(float worldWidth, float worldHeight)
    : worldWidth(worldWidth), worldHeight(worldHeight) {}

/**
 * @brief Get nearest periodic image of a position relative to reference
 * @param pos Position to adjust
 * @param reference Reference position
 * @return Adjusted position using minimum image convention
 *
 * Used to correctly compute distances when objects may be closer through
 * periodic wrapping than through direct distance.
 */
Vec2 CollisionDetector::getMinimumImagePos(const Vec2& pos, const Vec2& reference) {
    Vec2 dr = pos - reference;
    dr = minimumImage(dr, worldWidth, worldHeight);
    return reference + dr;
}

bool CollisionDetector::checkCollision(Body* a, Body* b, float radiusA, float radiusB, float& outDistance) {
    Vec2 posA = a->pos;
    Vec2 posB = b->pos;

    // Use minimum image convention if both wrap
    if (a->wraps && b->wraps) {
        posB = getMinimumImagePos(posB, posA);
    }

    Vec2 dr = posB - posA;
    float dist2 = dr.lengthSquared();
    float minDist = radiusA + radiusB;

    outDistance = std::sqrt(dist2);
    return dist2 < minDist * minDist;
}

void CollisionDetector::detectCollisions(
    std::vector<Ship>& ships,
    std::vector<Asteroid>& asteroids,
    std::vector<Bullet>& bullets,
    std::vector<BlackHole>& blackHoles,
    std::vector<CollisionPair>& outCollisions
) {
    outCollisions.clear();

    // Ship-Asteroid collisions
    for (auto& ship : ships) {
        if (!ship.active || ship.invulnerable) continue;
        for (auto& asteroid : asteroids) {
            if (!asteroid.active) continue;
            float dist;
            if (checkCollision(&ship, &asteroid, ship.radius, asteroid.radius, dist)) {
                outCollisions.push_back({&ship, &asteroid, dist});
            }
        }
    }

    // Ship-Ship collisions
    for (size_t i = 0; i < ships.size(); i++) {
        if (!ships[i].active) continue;
        for (size_t j = i + 1; j < ships.size(); j++) {
            if (!ships[j].active) continue;
            float dist;
            if (checkCollision(&ships[i], &ships[j], ships[i].radius, ships[j].radius, dist)) {
                outCollisions.push_back({&ships[i], &ships[j], dist});
            }
        }
    }

    // Asteroid-Asteroid collisions
    for (size_t i = 0; i < asteroids.size(); i++) {
        if (!asteroids[i].active) continue;
        for (size_t j = i + 1; j < asteroids.size(); j++) {
            if (!asteroids[j].active) continue;
            float dist;
            if (checkCollision(&asteroids[i], &asteroids[j],
                             asteroids[i].radius, asteroids[j].radius, dist)) {
                outCollisions.push_back({&asteroids[i], &asteroids[j], dist});
            }
        }
    }

    // Bullet-Asteroid collisions
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;
        for (auto& asteroid : asteroids) {
            if (!asteroid.active) continue;
            float dist;
            if (checkCollision(&bullet, &asteroid, bullet.radius, asteroid.radius, dist)) {
                outCollisions.push_back({&bullet, &asteroid, dist});
            }
        }
    }

    // Black hole accretion
    for (auto& bh : blackHoles) {
        if (!bh.active) continue;

        // Check ships
        for (auto& ship : ships) {
            if (!ship.active) continue;
            Vec2 dr = ship.pos - bh.pos;
            if (ship.wraps) {
                dr = minimumImage(dr, worldWidth, worldHeight);
            }
            float dist = dr.length();
            if (dist < bh.accretionRadius) {
                outCollisions.push_back({&ship, &bh, dist});
            }
        }

        // Check asteroids
        for (auto& asteroid : asteroids) {
            if (!asteroid.active) continue;
            Vec2 dr = asteroid.pos - bh.pos;
            if (asteroid.wraps) {
                dr = minimumImage(dr, worldWidth, worldHeight);
            }
            float dist = dr.length();
            if (dist < bh.accretionRadius) {
                outCollisions.push_back({&asteroid, &bh, dist});
            }
        }

        // Check bullets
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            Vec2 dr = bullet.pos - bh.pos;
            if (bullet.wraps) {
                dr = minimumImage(dr, worldWidth, worldHeight);
            }
            float dist = dr.length();
            if (dist < bh.accretionRadius) {
                outCollisions.push_back({&bullet, &bh, dist});
            }
        }
    }
}

// ============================================================================
// CollisionHandler implementation
// ============================================================================

/**
 * @brief Construct collision handler
 * @param worldWidth Width of simulation domain
 * @param worldHeight Height of simulation domain
 */
CollisionHandler::CollisionHandler(float worldWidth, float worldHeight)
    : worldWidth(worldWidth), worldHeight(worldHeight) {}

void CollisionHandler::handleShipAsteroid(Ship* ship, Asteroid* asteroid, std::vector<Particle>& particles) {
    // Calculate collision point (between ship and asteroid centers)
    Vec2 dr = minimumImage(asteroid->pos - ship->pos, worldWidth, worldHeight);
    float dist = dr.length();
    Vec2 collisionPoint = ship->pos;
    if (dist > 1e-6f) {
        // Point on ship's surface towards asteroid
        Vec2 direction = dr / dist;
        collisionPoint = ship->pos + direction * ship->radius;
    }

    // Ship loses a life
    ship->lives--;
    if (ship->lives <= 0) {
        ship->active = false;
        // Massive death explosion: collision + ship breakup (with ship's color)
        createExplosion(collisionPoint, 50, particles, 150.0f, 350.0f, 1.3f, ship->playerId);
        createExplosion(ship->pos, 40, particles, 100.0f, 300.0f, 1.5f, ship->playerId);
    } else {
        // Respawn with invulnerability
        ship->invulnerable = true;
        ship->invulnerableTime = 3.0f;
        // Single impact explosion at collision point only (with ship's color)
        createExplosion(collisionPoint, 40, particles, 150.0f, 350.0f, 1.3f, ship->playerId);
        // Respawn at center (no explosion here)
        ship->pos = Vec2(worldWidth * 0.5f, worldHeight * 0.5f);
        ship->vel = Vec2(0, 0);
    }
}

void CollisionHandler::handleShipShip(Ship* ship1, Ship* ship2) {
    // Elastic bounce
    Vec2 pos1 = ship1->pos;
    Vec2 pos2 = ship2->pos;

    // Minimum image
    Vec2 dr = minimumImage(pos2 - pos1, worldWidth, worldHeight);
    float dist = dr.length();
    if (dist < 1e-6f) return;

    Vec2 normal = dr / dist;

    // Relative velocity
    Vec2 relVel = ship2->vel - ship1->vel;
    float velAlongNormal = relVel.dot(normal);

    // Don't resolve if velocities are separating
    if (velAlongNormal > 0) return;

    // Equal mass elastic collision
    float impulse = -2.0f * velAlongNormal / 2.0f;

    ship1->vel -= normal * impulse;
    ship2->vel += normal * impulse;

    // Separate ships to avoid overlap
    float overlap = (ship1->radius + ship2->radius) - dist;
    if (overlap > 0) {
        Vec2 separation = normal * (overlap * 0.5f);
        ship1->pos -= separation;
        ship2->pos += separation;
        ship1->pos = wrapPosition(ship1->pos, worldWidth, worldHeight);
        ship2->pos = wrapPosition(ship2->pos, worldWidth, worldHeight);
    }
}

void CollisionHandler::mergeAsteroids(Asteroid* a1, Asteroid* a2) {
    // Inelastic merger with momentum conservation
    Vec2 pos1 = a1->pos;
    Vec2 pos2 = a2->pos;

    // Minimum image for position
    Vec2 dr = minimumImage(pos2 - pos1, worldWidth, worldHeight);
    pos2 = pos1 + dr;

    float m1 = a1->mass;
    float m2 = a2->mass;
    float totalMass = m1 + m2;

    // New velocity (momentum conservation)
    Vec2 newVel = (a1->vel * m1 + a2->vel * m2) / totalMass;

    // New position (center of mass)
    Vec2 newPos = (pos1 * m1 + pos2 * m2) / totalMass;
    newPos = wrapPosition(newPos, worldWidth, worldHeight);

    // Update first asteroid
    a1->pos = newPos;
    a1->vel = newVel;
    a1->mass = totalMass;
    a1->radius = std::sqrt(totalMass / 100.0f) * 40.0f;  // Scale with sqrt(mass)

    // Deactivate second
    a2->active = false;
}

void CollisionHandler::handleAsteroidAsteroid(Asteroid* a1, Asteroid* a2) {
    // Elastic bounce with mass-dependent response
    Vec2 pos1 = a1->pos;
    Vec2 pos2 = a2->pos;

    // Minimum image
    Vec2 dr = minimumImage(pos2 - pos1, worldWidth, worldHeight);
    float dist = dr.length();
    if (dist < 1e-6f) return;

    Vec2 normal = dr / dist;

    // Relative velocity
    Vec2 relVel = a2->vel - a1->vel;
    float velAlongNormal = relVel.dot(normal);

    // Don't resolve if velocities are separating
    if (velAlongNormal > 0) return;

    // Elastic collision with different masses
    float m1 = a1->mass;
    float m2 = a2->mass;
    float totalMass = m1 + m2;

    // Coefficient of restitution (1.0 = perfectly elastic)
    float restitution = 1.0f;

    // Calculate impulse magnitude
    float impulse = -(1.0f + restitution) * velAlongNormal / (1.0f/m1 + 1.0f/m2);

    // Apply impulse
    a1->vel -= normal * (impulse / m1);
    a2->vel += normal * (impulse / m2);

    // Separate asteroids to avoid overlap
    float overlap = (a1->radius + a2->radius) - dist;
    if (overlap > 0) {
        // Mass-proportional separation (lighter object moves more)
        float separation1 = overlap * (m2 / totalMass);
        float separation2 = overlap * (m1 / totalMass);

        a1->pos -= normal * separation1;
        a2->pos += normal * separation2;
        a1->pos = wrapPosition(a1->pos, worldWidth, worldHeight);
        a2->pos = wrapPosition(a2->pos, worldWidth, worldHeight);
    }
}

void CollisionHandler::mergeBulletIntoAsteroid(Bullet* bullet, Asteroid* asteroid) {
    // Inelastic merger
    float m1 = asteroid->mass;
    float m2 = bullet->mass;
    float totalMass = m1 + m2;

    // New velocity
    asteroid->vel = (asteroid->vel * m1 + bullet->vel * m2) / totalMass;
    asteroid->mass = totalMass;
    asteroid->radius = std::sqrt(totalMass / 100.0f) * 40.0f;

    bullet->active = false;
}

void CollisionHandler::handleBulletAsteroid(Bullet* bullet, Asteroid* asteroid, std::vector<Particle>& particles, std::vector<Asteroid>& asteroids, int& nextId) {
    // Destroy bullet
    bullet->active = false;

    // Split asteroid if not the dust level
    if (asteroid->size < 5) {
        // Spawn exactly 2 fragments that fly apart
        for (int i = 0; i < 2; i++) {
            Asteroid newAst;

            // Fragments fly in opposite directions
            float baseAngle = (rand() % 360) * 3.14159f / 180.0f;
            float angle = baseAngle + i * 3.14159f;  // 180 degrees apart

            // Position offset - make them clearly separated
            Vec2 offset(std::cos(angle) * asteroid->radius * 1.5f,
                       std::sin(angle) * asteroid->radius * 1.5f);
            Vec2 newPos = asteroid->pos + offset;
            newPos = wrapPosition(newPos, worldWidth, worldHeight);

            // Velocity - fragments fly apart at high speed
            float speed = 100.0f + (rand() % 100);  // Much faster separation
            Vec2 separationVel(std::cos(angle) * speed, std::sin(angle) * speed);
            Vec2 newVel = asteroid->vel * 0.3f + separationVel;  // Less parent velocity, more separation

            // Calculate base mass from current asteroid mass
            // If current is size N with mass M, then baseMass = M * 2^N
            float baseMass = asteroid->mass * (1 << asteroid->size);  // 2^size
            newAst.init(nextId++, newPos, newVel, asteroid->size + 1, baseMass);
            asteroids.push_back(newAst);
        }

        // Create explosion
        createExplosion(asteroid->pos, 8, particles);
    } else {
        // Dust-level asteroids just explode with more particles
        createExplosion(asteroid->pos, 15, particles);
    }

    // Destroy original asteroid
    asteroid->active = false;
}

void CollisionHandler::handleBlackHoleAccretion(Body* body, BlackHole* blackHole, std::vector<Particle>& particles) {
    // Save original position before any modifications
    Vec2 accretionPos = body->pos;

    if (body->type == EntityType::SHIP) {
        Ship* ship = static_cast<Ship*>(body);
        ship->lives--;
        if (ship->lives <= 0) {
            ship->active = false;
            // Dramatic death by black hole: many particles sucked in (with ship's color)
            createExplosion(accretionPos, 60, particles, 50.0f, 250.0f, 2.0f, ship->playerId);
        } else {
            ship->invulnerable = true;
            ship->invulnerableTime = 3.0f;
            // Explosion at accretion point only (not at respawn location, with ship's color)
            createExplosion(accretionPos, 40, particles, 50.0f, 200.0f, 1.5f, ship->playerId);
            // Respawn at center (no explosion here)
            ship->pos = Vec2(worldWidth * 0.5f, worldHeight * 0.5f);
            ship->vel = Vec2(0, 0);
        }
    } else {
        // Other entities get sucked in (white particles)
        body->active = false;
        createExplosion(accretionPos, 20, particles, 50.0f, 150.0f, 1.0f, -1);
    }
}

void CollisionHandler::createExplosion(Vec2 pos, int count, std::vector<Particle>& particles, float speedMin, float speedMax, float lifetimeMultiplier, int playerId) {
    for (int i = 0; i < count; i++) {
        Particle p;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speedRange = speedMax - speedMin;
        float speed = speedMin + (rand() % (int)(speedRange + 1));
        Vec2 vel(std::cos(angle) * speed, std::sin(angle) * speed);
        p.init(pos, vel, playerId);
        p.maxLifetime *= lifetimeMultiplier;
        p.lifetime = p.maxLifetime;
        particles.push_back(p);
    }
}
