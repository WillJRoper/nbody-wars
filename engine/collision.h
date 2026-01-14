/**
 * @file collision.h
 * @brief Collision detection and response for all game entities
 *
 * Handles both collision detection using periodic boundary conditions
 * and collision response for all entity pairs. Supports merging asteroids,
 * splitting asteroids when hit by bullets, ship damage, and black hole
 * accretion. Creates explosion particle effects for dramatic visual feedback.
 */

#pragma once
#include "entity.h"
#include "quadtree.h"
#include <vector>

/**
 * @struct CollisionPair
 * @brief Records a detected collision between two bodies
 *
 * Used by CollisionDetector to pass collision information to
 * CollisionHandler for response processing.
 */
struct CollisionPair {
    Body* a;           ///< First colliding body
    Body* b;           ///< Second colliding body
    float distance;    ///< Distance between centers (should be < sum of radii)
};

/**
 * @class CollisionDetector
 * @brief Detects collisions between all entity types
 *
 * Uses brute-force O(N²) collision detection with periodic boundary
 * condition support via minimum image convention. Checks all pairs of
 * entities that can meaningfully collide (e.g., ships with asteroids,
 * bullets with asteroids, but not bullets with bullets).
 */
class CollisionDetector {
public:
    /**
     * @brief Construct collision detector for a world
     * @param worldWidth Width of simulation domain
     * @param worldHeight Height of simulation domain
     */
    CollisionDetector(float worldWidth, float worldHeight);

    /**
     * @brief Detect all collisions in current frame
     * @param ships Active ships
     * @param asteroids Active asteroids
     * @param bullets Active bullets
     * @param blackHoles Active black holes
     * @param outCollisions Output vector to receive detected collision pairs
     *
     * Checks the following collision types:
     * - Ship vs Asteroid
     * - Ship vs Ship
     * - Bullet vs Asteroid
     * - Asteroid vs Asteroid
     * - All entities vs Black Hole accretion radius
     */
    void detectCollisions(
        std::vector<Ship>& ships,
        std::vector<Asteroid>& asteroids,
        std::vector<Bullet>& bullets,
        std::vector<BlackHole>& blackHoles,
        std::vector<CollisionPair>& outCollisions
    );

private:
    float worldWidth, worldHeight;  ///< Domain size for periodic boundaries

    /**
     * @brief Check if two bodies collide using minimum image distance
     * @param a First body
     * @param b Second body
     * @param radiusA Collision radius of body A
     * @param radiusB Collision radius of body B
     * @param outDistance Output: distance between bodies
     * @return True if bodies overlap (distance < radiusA + radiusB)
     */
    bool checkCollision(Body* a, Body* b, float radiusA, float radiusB, float& outDistance);

    /**
     * @brief Get nearest periodic image of a position
     * @param pos Position to adjust
     * @param reference Reference position
     * @return Position adjusted for minimum image convention
     *
     * Accounts for periodic boundaries when computing distances.
     */
    Vec2 getMinimumImagePos(const Vec2& pos, const Vec2& reference);
};

/**
 * @class CollisionHandler
 * @brief Handles collision response and physics for all entity types
 *
 * Processes detected collisions and applies appropriate responses:
 * - Ship-Asteroid: Damage ship, create explosion, respawn if alive
 * - Ship-Ship: Elastic collision with momentum conservation
 * - Asteroid-Asteroid: Merge into larger asteroid (mass conservation)
 * - Bullet-Asteroid: Destroy or merge, split asteroid if large enough
 * - Black Hole: Accrete any object within radius, create particles
 *
 * Creates colored explosion particles for visual feedback matching
 * player colors (green for player 0, cyan for player 1, white for asteroids).
 */
class CollisionHandler {
public:
    /**
     * @brief Construct collision handler for a world
     * @param worldWidth Width of simulation domain
     * @param worldHeight Height of simulation domain
     */
    CollisionHandler(float worldWidth, float worldHeight);

    /**
     * @brief Handle ship colliding with asteroid
     * @param ship Ship that was hit
     * @param asteroid Asteroid that hit the ship
     * @param particles Particle vector for explosion effects
     *
     * Decrements ship lives, creates explosion at collision point,
     * respawns ship at center if lives remain, creates death explosion
     * if no lives remain. Explosion particles match ship color.
     */
    void handleShipAsteroid(Ship* ship, Asteroid* asteroid, std::vector<Particle>& particles);

    /**
     * @brief Handle two ships colliding
     * @param ship1 First ship
     * @param ship2 Second ship
     *
     * Applies elastic collision with momentum and energy conservation.
     * Both ships bounce off each other realistically.
     */
    void handleShipShip(Ship* ship1, Ship* ship2);

    /**
     * @brief Handle two asteroids colliding
     * @param a1 First asteroid
     * @param a2 Second asteroid
     *
     * Merges asteroids into a single larger asteroid, conserving total
     * mass and momentum. The merged asteroid has combined properties.
     */
    void handleAsteroidAsteroid(Asteroid* a1, Asteroid* a2);

    /**
     * @brief Handle bullet hitting asteroid
     * @param bullet Bullet that hit
     * @param asteroid Asteroid that was hit
     * @param particles Particle vector for explosion effects
     * @param asteroids Asteroid vector to spawn fragments
     * @param nextId Next available entity ID for new asteroids
     *
     * Behavior depends on asteroid size:
     * - Large (0): Splits into 2 medium asteroids
     * - Medium (1): Splits into 2 small asteroids
     * - Small (2): Destroyed completely
     * Bullet is consumed. Awards score to bullet owner.
     */
    void handleBulletAsteroid(Bullet* bullet, Asteroid* asteroid, std::vector<Particle>& particles, std::vector<Asteroid>& asteroids, int& nextId);

    /**
     * @brief Handle black hole accreting an object
     * @param body Object being accreted
     * @param blackHole Black hole doing the accreting
     * @param particles Particle vector for accretion effects
     * @param asteroids Asteroid vector to spawn fragments
     * @param nextId Next available entity ID for new asteroids
     * @param distance Distance between body and black hole center
     *
     * Behavior depends on body type:
     * - Asteroids: Split like bullet hit - one half consumed, one escapes
     * - Ships: Instant damage with respawn mechanic
     * - Bullets: Instant destruction
     *
     * Asteroids are "nibbled" by splitting them in half. One fragment is
     * immediately consumed, the other escapes away from the black hole.
     */
    void handleBlackHoleAccretion(Body* body, BlackHole* blackHole, std::vector<Particle>& particles,
                                  std::vector<Asteroid>& asteroids, int& nextId, float distance);

private:
    float worldWidth, worldHeight;  ///< Domain size for respawn calculations

    /**
     * @brief Merge two asteroids into one
     * @param a1 First asteroid (will receive merged properties)
     * @param a2 Second asteroid (will be deactivated)
     *
     * Combines mass and momentum, updates position to center of mass,
     * increases size if appropriate. Conserves total mass and momentum.
     */
    void mergeAsteroids(Asteroid* a1, Asteroid* a2);

    /**
     * @brief Merge bullet into asteroid
     * @param bullet Bullet to merge (will be deactivated)
     * @param asteroid Asteroid receiving mass (properties updated)
     *
     * Adds bullet mass and momentum to asteroid. Used when bullet hits
     * an asteroid too large to destroy.
     */
    void mergeBulletIntoAsteroid(Bullet* bullet, Asteroid* asteroid);

    /**
     * @brief Create particle explosion effect
     * @param pos Explosion center position
     * @param count Number of particles to create
     * @param particles Particle vector to add new particles
     * @param speedMin Minimum particle speed (randomized)
     * @param speedMax Maximum particle speed (randomized)
     * @param lifetimeMultiplier Scale factor for particle lifetime
     * @param playerId Color code: -1=white, 0=green, 1=cyan
     *
     * Creates radial burst of colored particles with random speeds
     * and directions. Used for ship deaths, asteroid destruction,
     * and black hole accretion effects.
     */
    void createExplosion(Vec2 pos, int count, std::vector<Particle>& particles, float speedMin = 50.0f, float speedMax = 150.0f, float lifetimeMultiplier = 1.0f, int playerId = -1);
};
