#include "engine.h"
#include <algorithm>
#include <cmath>

GameEngine::GameEngine(float width, float height, uint32_t gameSeed)
    : worldWidth(width), worldHeight(height), time(0), wave(1),
      seed(gameSeed), rng(gameSeed), mode(GameMode::SOLO),
      currentLevel(0), nextEntityId(0) {

    quadtree = std::make_unique<QuadTree>(width, height);
    collisionDetector = std::make_unique<CollisionDetector>(width, height);
    collisionHandler = std::make_unique<CollisionHandler>(width, height);
    potential = createPotential(0, Vec2(width * 0.5f, height * 0.5f), width);

    reset();
}

GameEngine::~GameEngine() = default;

void GameEngine::setMode(GameMode newMode) {
    mode = newMode;
    reset();
}

void GameEngine::setLevel(int levelId) {
    currentLevel = levelId;
    potential = createPotential(levelId, Vec2(worldWidth * 0.5f, worldHeight * 0.5f), worldWidth);
}

void GameEngine::setDifficulty(const DifficultyConfig& config) {
    difficulty = config;
}

void GameEngine::setBlackHolesEnabled(bool enabled) {
    difficulty.bhEnabled = enabled;
}

void GameEngine::setShipMass(float mass) {
    difficulty.shipMass = mass;
    // Update existing ships
    for (auto& ship : ships) {
        ship.mass = mass;
    }
}

void GameEngine::setBulletMass(float mass) {
    difficulty.bulletMass = mass;
    // Update existing bullets
    for (auto& bullet : bullets) {
        bullet.mass = mass;
    }
}

void GameEngine::setAsteroidBaseMass(float mass) {
    difficulty.asteroidBaseMass = mass;
    // Update existing asteroids - recalculate their mass based on size
    for (auto& asteroid : asteroids) {
        float divisor = (1 << asteroid.size);  // 2^size
        asteroid.mass = mass / divisor;
    }
}

void GameEngine::setInput(int playerId, const InputState& input) {
    if (playerId >= 0 && playerId < 2) {
        inputs[playerId] = input;
    }
}

void GameEngine::reset() {
    time = 0;
    wave = 1;
    nextEntityId = 0;
    rng.seed(seed);

    ships.clear();
    asteroids.clear();
    bullets.clear();
    blackHoles.clear();
    particles.clear();

    // Create ships based on mode
    int numShips = (mode == GameMode::SOLO) ? 1 : 2;
    for (int i = 0; i < numShips; i++) {
        Ship ship;
        Vec2 pos = Vec2(worldWidth * (0.3f + i * 0.4f), worldHeight * 0.5f);
        ship.init(nextEntityId++, pos, i);
        ship.mass = difficulty.shipMass;  // Apply configurable mass
        ships.push_back(ship);
    }

    spawnInitialAsteroids();
}

void GameEngine::spawnInitialAsteroids() {
    int count = difficulty.asteroidCount + wave * 2;
    for (int i = 0; i < count; i++) {
        Vec2 pos = randomEdgePosition();
        Vec2 vel = randomVelocity(20.0f + wave * 5.0f);
        spawnAsteroid(pos, vel, 0);  // Spawn large asteroids
    }
}

void GameEngine::spawnAsteroid(Vec2 pos, Vec2 vel, int size) {
    Asteroid asteroid;
    asteroid.init(nextEntityId++, pos, vel, size, difficulty.asteroidBaseMass);
    asteroids.push_back(asteroid);
}

void GameEngine::spawnBlackHole() {
    BlackHole bh;

    // Spawn from a random edge
    int edge = rng() % 4;
    Vec2 pos, vel;

    switch (edge) {
        case 0: // Top
            pos = Vec2(randomFloat(0, worldWidth), -50);
            vel = Vec2(randomFloat(-50, 50), randomFloat(80, 150));
            break;
        case 1: // Right
            pos = Vec2(worldWidth + 50, randomFloat(0, worldHeight));
            vel = Vec2(randomFloat(-150, -80), randomFloat(-50, 50));
            break;
        case 2: // Bottom
            pos = Vec2(randomFloat(0, worldWidth), worldHeight + 50);
            vel = Vec2(randomFloat(-50, 50), randomFloat(-150, -80));
            break;
        case 3: // Left
            pos = Vec2(-50, randomFloat(0, worldHeight));
            vel = Vec2(randomFloat(80, 150), randomFloat(-50, 50));
            break;
    }

    float mass = (5000.0f + wave * 500.0f) * difficulty.bhMassMult;
    bh.init(nextEntityId++, pos, vel, mass, difficulty.bhAccRadius);
    blackHoles.push_back(bh);
}

void GameEngine::step() {
    // Update entity timers
    updateEntities();

    // Apply inputs to ships
    for (size_t i = 0; i < ships.size(); i++) {
        if (!ships[i].active) continue;

        const InputState& input = inputs[i];

        if (input.left) {
            ships[i].rotate(-3.0f * physics.dt);
        }
        if (input.right) {
            ships[i].rotate(3.0f * physics.dt);
        }
        if (input.thrust) {
            ships[i].thrust(500.0f, physics.dt);
        }
        if (input.brake) {
            // Apply deceleration (negative thrust)
            float speed = ships[i].vel.length();
            if (speed > 1.0f) {
                Vec2 decelDirection = ships[i].vel.normalized() * -1.0f;
                ships[i].vel += decelDirection * (500.0f * physics.dt);
            } else {
                // Stop completely if moving slowly
                ships[i].vel = Vec2(0, 0);
            }
        }
        if (input.shoot && ships[i].canShoot()) {
            // Spawn bullet
            Bullet bullet;
            Vec2 direction(std::cos(ships[i].angle), std::sin(ships[i].angle));
            Vec2 bulletPos = ships[i].pos + direction * (ships[i].radius + 5.0f);
            Vec2 bulletVel = ships[i].vel + direction * 300.0f;
            bullet.init(nextEntityId++, bulletPos, bulletVel, i);
            bullet.mass = difficulty.bulletMass;  // Apply configurable mass
            bullets.push_back(bullet);
            ships[i].shoot();
        }
    }

    // Apply physics
    applyPhysics();

    // Handle collisions
    handleCollisions();

    // Spawn black holes
    if (difficulty.bhEnabled && randomFloat(0, 1) < difficulty.bhSpawnRate) {
        spawnBlackHole();
    }

    // Cleanup
    cleanupInactive();

    // Check wave progression
    checkWaveComplete();

    time += physics.dt;
}

void GameEngine::updateEntities() {
    for (auto& ship : ships) {
        if (ship.active) ship.update(physics.dt);
    }
    for (auto& asteroid : asteroids) {
        if (asteroid.active) asteroid.update(physics.dt);
    }
    for (auto& bullet : bullets) {
        if (bullet.active) bullet.update(physics.dt);
    }
    for (auto& particle : particles) {
        if (particle.active) particle.update(physics.dt);
    }
}

void GameEngine::applyPhysics() {
    // Collect all bodies for N-body gravity
    std::vector<Body*> bodies;
    for (auto& ship : ships) {
        if (ship.active) bodies.push_back(&ship);
    }
    for (auto& asteroid : asteroids) {
        if (asteroid.active) bodies.push_back(&asteroid);
    }
    for (auto& bullet : bullets) {
        if (bullet.active) bodies.push_back(&bullet);
    }
    for (auto& bh : blackHoles) {
        if (bh.active) bodies.push_back(&bh);
    }

    // Build quadtree
    if (!bodies.empty()) {
        quadtree->build(bodies);
    }

    // Leapfrog integration (kick-drift-kick / velocity Verlet)
    // First half-kick: v += a * dt/2
    for (Body* body : bodies) {
        Vec2 acc(0, 0);

        // N-body gravity
        if (!bodies.empty()) {
            acc += quadtree->calculateAcceleration(body->pos, body->mass,
                                                   physics.theta, physics.epsilon, physics.G);
        }

        // External potential
        if (potential) {
            acc += potential->accelerationAt(body->pos);
        }

        body->acc = acc;
        body->vel += acc * (physics.dt * 0.5f);
    }

    // Drift: x += v * dt
    for (Body* body : bodies) {
        body->pos += body->vel * physics.dt;

        // Apply wrapping for entities that wrap
        if (body->wraps) {
            body->pos = wrapPosition(body->pos, worldWidth, worldHeight);
        }
    }

    // Rebuild quadtree after drift
    if (!bodies.empty()) {
        quadtree->build(bodies);
    }

    // Second half-kick: v += a * dt/2
    for (Body* body : bodies) {
        Vec2 acc(0, 0);

        // N-body gravity
        if (!bodies.empty()) {
            acc += quadtree->calculateAcceleration(body->pos, body->mass,
                                                   physics.theta, physics.epsilon, physics.G);
        }

        // External potential
        if (potential) {
            acc += potential->accelerationAt(body->pos);
        }

        body->acc = acc;
        body->vel += acc * (physics.dt * 0.5f);
    }

    // Remove black holes that went offscreen
    for (auto& bh : blackHoles) {
        if (bh.active && bh.isOffscreen(worldWidth, worldHeight)) {
            bh.active = false;
        }
    }
}

void GameEngine::handleCollisions() {
    std::vector<CollisionPair> collisions;
    collisionDetector->detectCollisions(ships, asteroids, bullets, blackHoles, collisions);

    for (const auto& collision : collisions) {
        Body* a = collision.a;
        Body* b = collision.b;

        if (!a->active || !b->active) continue;

        // Determine collision type
        if (a->type == EntityType::SHIP && b->type == EntityType::ASTEROID) {
            collisionHandler->handleShipAsteroid(static_cast<Ship*>(a),
                                                static_cast<Asteroid*>(b), particles);
        } else if (a->type == EntityType::ASTEROID && b->type == EntityType::SHIP) {
            collisionHandler->handleShipAsteroid(static_cast<Ship*>(b),
                                                static_cast<Asteroid*>(a), particles);
        } else if (a->type == EntityType::SHIP && b->type == EntityType::SHIP) {
            collisionHandler->handleShipShip(static_cast<Ship*>(a), static_cast<Ship*>(b));
        } else if (a->type == EntityType::ASTEROID && b->type == EntityType::ASTEROID) {
            collisionHandler->handleAsteroidAsteroid(static_cast<Asteroid*>(a),
                                                     static_cast<Asteroid*>(b));
        } else if (a->type == EntityType::BULLET && b->type == EntityType::ASTEROID) {
            Bullet* bullet = static_cast<Bullet*>(a);
            Asteroid* asteroid = static_cast<Asteroid*>(b);
            collisionHandler->handleBulletAsteroid(bullet, asteroid, particles, asteroids, nextEntityId);

            // Award points
            if (bullet->playerId >= 0 && bullet->playerId < (int)ships.size()) {
                ships[bullet->playerId].score += 10;
            }
        } else if (a->type == EntityType::ASTEROID && b->type == EntityType::BULLET) {
            Bullet* bullet = static_cast<Bullet*>(b);
            Asteroid* asteroid = static_cast<Asteroid*>(a);
            collisionHandler->handleBulletAsteroid(bullet, asteroid, particles, asteroids, nextEntityId);

            // Award points
            if (bullet->playerId >= 0 && bullet->playerId < (int)ships.size()) {
                ships[bullet->playerId].score += 10;
            }
        } else if (b->type == EntityType::BLACK_HOLE) {
            collisionHandler->handleBlackHoleAccretion(a, static_cast<BlackHole*>(b), particles, asteroids, nextEntityId, collision.distance);
        } else if (a->type == EntityType::BLACK_HOLE) {
            collisionHandler->handleBlackHoleAccretion(b, static_cast<BlackHole*>(a), particles, asteroids, nextEntityId, collision.distance);
        }
    }
}

void GameEngine::cleanupInactive() {
    asteroids.erase(
        std::remove_if(asteroids.begin(), asteroids.end(),
                      [](const Asteroid& a) { return !a.active; }),
        asteroids.end());

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
                      [](const Bullet& b) { return !b.active; }),
        bullets.end());

    blackHoles.erase(
        std::remove_if(blackHoles.begin(), blackHoles.end(),
                      [](const BlackHole& bh) { return !bh.active; }),
        blackHoles.end());

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                      [](const Particle& p) { return !p.active; }),
        particles.end());
}

void GameEngine::checkWaveComplete() {
    // Check if all asteroids are destroyed
    bool hasActiveAsteroids = false;
    for (const auto& asteroid : asteroids) {
        if (asteroid.active) {
            hasActiveAsteroids = true;
            break;
        }
    }

    if (!hasActiveAsteroids && asteroids.empty()) {
        wave++;
        spawnInitialAsteroids();
    }
}

bool GameEngine::isGameOver() const {
    for (const auto& ship : ships) {
        if (ship.active) return false;
    }
    return true;
}

float GameEngine::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

Vec2 GameEngine::randomEdgePosition() {
    int edge = rng() % 4;
    switch (edge) {
        case 0: return Vec2(randomFloat(0, worldWidth), 0);              // Top
        case 1: return Vec2(worldWidth, randomFloat(0, worldHeight));    // Right
        case 2: return Vec2(randomFloat(0, worldWidth), worldHeight);    // Bottom
        case 3: return Vec2(0, randomFloat(0, worldHeight));             // Left
    }
    return Vec2(0, 0);
}

Vec2 GameEngine::randomVelocity(float speed) {
    float angle = randomFloat(0, 6.28318f);
    return Vec2(std::cos(angle) * speed, std::sin(angle) * speed);
}
