#pragma once
#include "vec2.h"
#include "quadtree.h"
#include "potential.h"
#include "entity.h"
#include "collision.h"
#include <vector>
#include <memory>
#include <random>

enum class GameMode {
    SOLO,
    COOP,
    VERSUS
};

struct PhysicsConfig {
    float dt;           // Fixed timestep
    float G;            // Gravitational constant
    float epsilon;      // Softening length
    float theta;        // Barnes-Hut opening angle

    PhysicsConfig()
        : dt(1.0f / 120.0f), G(100.0f), epsilon(5.0f), theta(0.5f) {}
};

struct DifficultyConfig {
    float bhSpawnRate;       // Black holes per second
    float bhMassMult;        // Black hole mass multiplier
    float bhAccRadius;       // Black hole accretion radius
    bool bhEnabled;          // Enable/disable black hole spawning
    float shipMass;          // Ship mass for N-body interactions
    float bulletMass;        // Bullet mass for N-body interactions
    float asteroidBaseMass;  // Large asteroid mass (size 0), others halve each size
    int asteroidCount;       // Number of asteroids per wave

    DifficultyConfig()
        : bhSpawnRate(0.0005f), bhMassMult(1.0f), bhAccRadius(25.0f), bhEnabled(true),
          shipMass(1500.0f), bulletMass(100.0f), asteroidBaseMass(6000.0f), asteroidCount(4) {}
};

struct InputState {
    bool left;
    bool right;
    bool thrust;
    bool brake;
    bool shoot;

    InputState() : left(false), right(false), thrust(false), brake(false), shoot(false) {}
};

class GameEngine {
public:
    GameEngine(float width, float height, uint32_t seed);
    ~GameEngine();

    void setMode(GameMode mode);
    void setLevel(int levelId);
    void setDifficulty(const DifficultyConfig& config);
    void setBlackHolesEnabled(bool enabled);
    void setShipMass(float mass);
    void setBulletMass(float mass);
    void setAsteroidBaseMass(float mass);
    void setInput(int playerId, const InputState& input);

    void step();
    void reset();

    // Getters for rendering
    const std::vector<Ship>& getShips() const { return ships; }
    const std::vector<Asteroid>& getAsteroids() const { return asteroids; }
    const std::vector<Bullet>& getBullets() const { return bullets; }
    const std::vector<BlackHole>& getBlackHoles() const { return blackHoles; }
    const std::vector<Particle>& getParticles() const { return particles; }

    float getWorldWidth() const { return worldWidth; }
    float getWorldHeight() const { return worldHeight; }
    float getTime() const { return time; }
    int getWave() const { return wave; }

    bool isGameOver() const;
    const IExternalPotential* getPotential() const { return potential.get(); }

private:
    float worldWidth, worldHeight;
    float time;
    int wave;
    uint32_t seed;
    std::mt19937 rng;

    GameMode mode;
    int currentLevel;
    PhysicsConfig physics;
    DifficultyConfig difficulty;

    std::unique_ptr<IExternalPotential> potential;
    std::unique_ptr<QuadTree> quadtree;
    std::unique_ptr<CollisionDetector> collisionDetector;
    std::unique_ptr<CollisionHandler> collisionHandler;

    std::vector<Ship> ships;
    std::vector<Asteroid> asteroids;
    std::vector<Bullet> bullets;
    std::vector<BlackHole> blackHoles;
    std::vector<Particle> particles;

    InputState inputs[2];

    int nextEntityId;

    // Game logic
    void spawnInitialAsteroids();
    void spawnAsteroid(Vec2 pos, Vec2 vel, int size);
    void spawnBlackHole();
    void updateEntities();
    void applyPhysics();
    void handleCollisions();
    void cleanupInactive();
    void checkWaveComplete();

    float randomFloat(float min, float max);
    Vec2 randomEdgePosition();
    Vec2 randomVelocity(float speed);
};
