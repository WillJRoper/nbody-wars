/**
 * @file engine.h
 * @brief Main game engine orchestrating physics, collisions, and gameplay
 *
 * The GameEngine class manages the complete simulation loop:
 * - N-body gravitational physics using Barnes-Hut algorithm
 * - External gravitational potentials (point mass, harmonic, logarithmic, NFW)
 * - Collision detection and response
 * - Entity lifecycle (spawning, updating, cleanup)
 * - Game state (waves, scoring, game over)
 * - Player input processing
 *
 * Integrates all subsystems into a cohesive game with configurable difficulty,
 * multiple game modes (solo, co-op, versus), and selectable physics levels.
 */

#pragma once
#include "vec2.h"
#include "quadtree.h"
#include "potential.h"
#include "entity.h"
#include "collision.h"
#include <vector>
#include <memory>
#include <random>

/**
 * @enum GameMode
 * @brief Determines number of players and win conditions
 */
enum class GameMode {
    SOLO,    ///< Single player - survive as long as possible
    COOP,    ///< Two players cooperate - shared score, game over if both die
    VERSUS   ///< Two players compete - highest score wins
};

/**
 * @struct PhysicsConfig
 * @brief Physics simulation parameters
 *
 * Controls the accuracy and behavior of the N-body gravitational simulation.
 * Default values are tuned for stable, visually appealing dynamics.
 */
struct PhysicsConfig {
    float dt;        ///< Fixed timestep (seconds) - default 1/120s
    float G;         ///< Gravitational constant - scales force strength
    float epsilon;   ///< Softening length - prevents singularities in close encounters
    float theta;     ///< Barnes-Hut opening angle - accuracy vs speed tradeoff (typical: 0.5)

    /**
     * @brief Default constructor with tuned physics parameters
     */
    PhysicsConfig()
        : dt(1.0f / 120.0f), G(100.0f), epsilon(5.0f), theta(0.5f) {}
};

/**
 * @struct DifficultyConfig
 * @brief Game difficulty and balance parameters
 *
 * Controls all tunable gameplay parameters including entity masses,
 * black hole spawn rates, and wave sizes. Allows fine-tuning of
 * difficulty through UI sliders.
 */
struct DifficultyConfig {
    float bhSpawnRate;       ///< Black hole spawn rate (probability per frame)
    float bhMassMult;        ///< Black hole mass multiplier (scales gravitational effect)
    float bhAccRadius;       ///< Black hole accretion radius (danger zone size)
    bool bhEnabled;          ///< Enable/disable black hole spawning
    float shipMass;          ///< Ship mass for N-body gravity interactions
    float bulletMass;        ///< Bullet mass for N-body gravity interactions
    float asteroidBaseMass;  ///< Large asteroid base mass (medium=half, small=quarter)
    int asteroidCount;       ///< Number of asteroids spawned per wave

    /**
     * @brief Default constructor with balanced difficulty
     */
    DifficultyConfig()
        : bhSpawnRate(0.0005f), bhMassMult(1.0f), bhAccRadius(25.0f), bhEnabled(true),
          shipMass(1500.0f), bulletMass(100.0f), asteroidBaseMass(8000.0f), asteroidCount(4) {}
};

/**
 * @struct InputState
 * @brief Player input snapshot for one frame
 *
 * Captures all player controls as boolean flags. Processed each frame
 * to update ship state (rotation, thrust, shooting).
 */
struct InputState {
    bool left;    ///< Rotate counter-clockwise
    bool right;   ///< Rotate clockwise
    bool thrust;  ///< Apply forward thrust
    bool brake;   ///< Apply braking force (opposite to velocity)
    bool shoot;   ///< Fire bullet

    /**
     * @brief Default constructor - all inputs released
     */
    InputState() : left(false), right(false), thrust(false), brake(false), shoot(false) {}
};

/**
 * @class GameEngine
 * @brief Main game simulation engine
 *
 * Orchestrates all game systems including physics, rendering data,
 * collision detection, entity management, and gameplay progression.
 * Runs at fixed timestep (120 Hz physics) with configurable difficulty
 * and multiple level types with different gravitational potentials.
 *
 * Simulation loop (step() method):
 * 1. Update entities (ship controls, bullet lifetime, particle decay)
 * 2. Apply N-body physics (Barnes-Hut + external potential)
 * 3. Detect and handle collisions
 * 4. Spawn new hazards (black holes)
 * 5. Clean up inactive entities
 * 6. Check wave completion and spawn new asteroids
 */
class GameEngine {
public:
    /**
     * @brief Construct game engine with world dimensions
     * @param width World width in pixels
     * @param height World height in pixels
     * @param seed Random seed for reproducible asteroid/black hole spawning
     */
    GameEngine(float width, float height, uint32_t seed);

    /**
     * @brief Destructor - cleans up all subsystems
     */
    ~GameEngine();

    /**
     * @brief Set game mode (solo, co-op, versus)
     * @param mode Desired game mode
     */
    void setMode(GameMode mode);

    /**
     * @brief Set gravitational potential level
     * @param levelId Level identifier (0-4):
     *   - 0: No Potential (pure N-body)
     *   - 1: Point Mass (Keplerian orbits)
     *   - 2: Harmonic Oscillator
     *   - 3: Logarithmic (flat rotation curve)
     *   - 4: NFW dark matter halo
     */
    void setLevel(int levelId);

    /**
     * @brief Set difficulty configuration
     * @param config Difficulty parameters (masses, spawn rates, etc.)
     */
    void setDifficulty(const DifficultyConfig& config);

    /**
     * @brief Enable/disable black hole spawning
     * @param enabled True to spawn black holes, false to disable
     */
    void setBlackHolesEnabled(bool enabled);

    /**
     * @brief Set ship mass for gravity calculations
     * @param mass Ship mass in arbitrary units
     */
    void setShipMass(float mass);

    /**
     * @brief Set bullet mass for gravity calculations
     * @param mass Bullet mass in arbitrary units
     */
    void setBulletMass(float mass);

    /**
     * @brief Set asteroid base mass (large size)
     * @param mass Large asteroid mass (medium=half, small=quarter)
     */
    void setAsteroidBaseMass(float mass);

    /**
     * @brief Set player input for current frame
     * @param playerId Player index (0 or 1)
     * @param input Input state (button presses)
     */
    void setInput(int playerId, const InputState& input);

    /**
     * @brief Advance simulation by one fixed timestep
     *
     * Complete simulation step including entity updates, physics,
     * collisions, spawning, and wave management. Should be called
     * at display refresh rate (typically 60 Hz).
     */
    void step();

    /**
     * @brief Reset game to initial state
     *
     * Clears all entities, resets scores and lives, starts at wave 1.
     * Preserves difficulty settings and selected level.
     */
    void reset();

    // Getters for rendering and UI

    /**
     * @brief Get all active ships
     * @return Vector of ships (1 for solo, 2 for co-op/versus)
     */
    const std::vector<Ship>& getShips() const { return ships; }

    /**
     * @brief Get all active asteroids
     * @return Vector of asteroids
     */
    const std::vector<Asteroid>& getAsteroids() const { return asteroids; }

    /**
     * @brief Get all active bullets
     * @return Vector of bullets
     */
    const std::vector<Bullet>& getBullets() const { return bullets; }

    /**
     * @brief Get all active black holes
     * @return Vector of black holes
     */
    const std::vector<BlackHole>& getBlackHoles() const { return blackHoles; }

    /**
     * @brief Get all active particles
     * @return Vector of explosion particles
     */
    const std::vector<Particle>& getParticles() const { return particles; }

    /**
     * @brief Get world width
     * @return Width in pixels
     */
    float getWorldWidth() const { return worldWidth; }

    /**
     * @brief Get world height
     * @return Height in pixels
     */
    float getWorldHeight() const { return worldHeight; }

    /**
     * @brief Get elapsed simulation time
     * @return Time in seconds since game start
     */
    float getTime() const { return time; }

    /**
     * @brief Get current wave number
     * @return Wave number (1-indexed)
     */
    int getWave() const { return wave; }

    /**
     * @brief Check if game is over
     * @return True if all players are dead (no lives remaining)
     */
    bool isGameOver() const;

    /**
     * @brief Get current external potential
     * @return Pointer to active potential (for UI display)
     */
    const IExternalPotential* getPotential() const { return potential.get(); }

private:
    // World properties
    float worldWidth, worldHeight;  ///< Simulation domain size
    float time;                     ///< Elapsed simulation time (seconds)
    int wave;                       ///< Current wave number (difficulty increases each wave)
    uint32_t seed;                  ///< Random seed for reproducibility
    std::mt19937 rng;               ///< Mersenne Twister RNG for spawning

    // Game configuration
    GameMode mode;                  ///< Current game mode (solo/co-op/versus)
    int currentLevel;               ///< Selected gravitational potential (0-4)
    PhysicsConfig physics;          ///< Physics simulation parameters
    DifficultyConfig difficulty;    ///< Gameplay balance parameters

    // Subsystems
    std::unique_ptr<IExternalPotential> potential;      ///< Active gravitational potential
    std::unique_ptr<QuadTree> quadtree;                 ///< Barnes-Hut tree for N-body gravity
    std::unique_ptr<CollisionDetector> collisionDetector;  ///< Collision detection system
    std::unique_ptr<CollisionHandler> collisionHandler;    ///< Collision response system

    // Entity collections
    std::vector<Ship> ships;            ///< Active ships (1-2 depending on mode)
    std::vector<Asteroid> asteroids;    ///< Active asteroids
    std::vector<Bullet> bullets;        ///< Active bullets
    std::vector<BlackHole> blackHoles;  ///< Active black holes
    std::vector<Particle> particles;    ///< Active explosion particles

    InputState inputs[2];  ///< Player inputs (index 0 and 1)

    int nextEntityId;  ///< Counter for unique entity IDs

    // Game logic methods

    /**
     * @brief Spawn asteroids at start of wave
     *
     * Creates asteroids at random edge positions with random velocities.
     * Number of asteroids determined by difficulty.asteroidCount.
     */
    void spawnInitialAsteroids();

    /**
     * @brief Spawn a single asteroid
     * @param pos Starting position
     * @param vel Initial velocity
     * @param size Size class (0=large, 1=medium, 2=small)
     */
    void spawnAsteroid(Vec2 pos, Vec2 vel, int size);

    /**
     * @brief Spawn a black hole at random edge position
     *
     * Called probabilistically each frame based on difficulty.bhSpawnRate.
     * Black hole mass and accretion radius determined by difficulty config.
     */
    void spawnBlackHole();

    /**
     * @brief Update all entity states
     *
     * Processes ship controls, updates bullet lifetimes, particle decay,
     * asteroid rotation, and ship cooldowns/invulnerability.
     */
    void updateEntities();

    /**
     * @brief Apply gravitational forces and integrate motion
     *
     * Uses leapfrog integrator with Barnes-Hut tree for N-body gravity
     * plus acceleration from external potential. Particles skip gravity.
     */
    void applyPhysics();

    /**
     * @brief Detect and respond to all collisions
     *
     * Runs collision detection then processes all collision pairs with
     * appropriate responses (damage, merging, splitting, scoring).
     */
    void handleCollisions();

    /**
     * @brief Remove inactive entities
     *
     * Removes entities marked as inactive (destroyed or expired) from
     * all entity vectors. Also removes off-screen black holes.
     */
    void cleanupInactive();

    /**
     * @brief Check if wave is complete and spawn next wave
     *
     * If no asteroids remain, increments wave counter and spawns new
     * wave with potentially more asteroids (progressive difficulty).
     */
    void checkWaveComplete();

    // Utility methods for random generation

    /**
     * @brief Generate random float in range
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Random float in [min, max)
     */
    float randomFloat(float min, float max);

    /**
     * @brief Generate random position on edge of screen
     * @return Position on boundary (for spawning asteroids/black holes)
     */
    Vec2 randomEdgePosition();

    /**
     * @brief Generate random velocity vector
     * @param speed Magnitude of velocity
     * @return Velocity vector with random direction and given magnitude
     */
    Vec2 randomVelocity(float speed);
};
