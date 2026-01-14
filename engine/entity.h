/**
 * @file entity.h
 * @brief Game entity definitions for the N-body physics simulation
 *
 * Defines all gameplay objects (ships, asteroids, bullets, black holes, particles)
 * that interact via Newtonian gravity and collisions. Each entity inherits from
 * the base Body struct which provides position, velocity, acceleration, and mass.
 */

#pragma once
#include "vec2.h"
#include <vector>
#include <cstdint>

/**
 * @enum EntityType
 * @brief Classification of game entities for collision detection and rendering
 *
 * Used to identify entity types during collision checks and to determine
 * appropriate rendering and physics behavior.
 */
enum class EntityType {
    SHIP,        ///< Player-controlled spacecraft
    ASTEROID,    ///< Destructible space rock with significant mass
    BULLET,      ///< Projectile fired by ships
    BLACK_HOLE,  ///< Massive gravitational hazard that accretes nearby objects
    PARTICLE     ///< Visual debris from explosions (non-interacting)
};

/**
 * @struct Body
 * @brief Base physics body with Newtonian dynamics
 *
 * All game entities inherit from Body to participate in N-body gravitational
 * simulations. Uses leapfrog integrator (kick-drift-kick) for time evolution.
 * Supports periodic boundary conditions via the wraps flag.
 */
struct Body {
    Vec2 pos;           ///< Position in world coordinates
    Vec2 vel;           ///< Velocity vector
    Vec2 acc;           ///< Acceleration (reset each timestep)
    float mass;         ///< Mass for gravitational interactions
    EntityType type;    ///< Entity classification
    bool wraps;         ///< If true, position wraps at periodic boundaries
    bool active;        ///< If false, entity is marked for deletion
    int id;             ///< Unique identifier

    /**
     * @brief Default constructor - initializes to inactive asteroid
     */
    Body() : mass(0), type(EntityType::ASTEROID), wraps(true), active(true), id(0) {}
};

/**
 * @struct Ship
 * @brief Player-controlled spacecraft with weapons and lives
 *
 * Ships can rotate, thrust, brake, and shoot bullets. They have collision
 * detection with asteroids and other ships. After taking damage, ships
 * receive temporary invulnerability. Ships participate in N-body gravity.
 */
struct Ship : public Body {
    int playerId;              ///< Player identifier (0 or 1)
    float angle;               ///< Orientation in radians (0 = pointing right)
    float radius;              ///< Collision radius
    int lives;                 ///< Remaining lives (game over at 0)
    int score;                 ///< Player score from destroying asteroids
    bool thrusting;            ///< True when thrust animation should display
    bool invulnerable;         ///< True during post-damage invulnerability period
    float invulnerableTime;    ///< Time remaining in invulnerability (seconds)
    float shootCooldown;       ///< Time until next shot is allowed (seconds)

    /**
     * @brief Default constructor
     */
    Ship();

    /**
     * @brief Initialize ship at a position
     * @param id Unique entity ID
     * @param pos Starting position
     * @param playerId Player identifier (0 or 1)
     */
    void init(int id, Vec2 pos, int playerId);

    /**
     * @brief Rotate ship
     * @param deltaAngle Angle change in radians (positive = counter-clockwise)
     */
    void rotate(float deltaAngle);

    /**
     * @brief Apply thrust in forward direction
     * @param power Thrust magnitude
     * @param dt Time step for velocity integration
     */
    void thrust(float power, float dt);

    /**
     * @brief Check if ship can fire
     * @return True if cooldown has expired
     */
    bool canShoot() const { return shootCooldown <= 0; }

    /**
     * @brief Fire a bullet (resets cooldown)
     */
    void shoot() { shootCooldown = 0.2f; }

    /**
     * @brief Update ship state (cooldowns, invulnerability)
     * @param dt Time step in seconds
     */
    void update(float dt);
};

/**
 * @struct Asteroid
 * @brief Destructible space rocks with significant gravitational mass
 *
 * Asteroids come in three sizes: large (0), medium (1), and small (2).
 * When destroyed by bullets, large asteroids split into 2 medium asteroids,
 * and medium asteroids split into 2 small asteroids. Small asteroids are
 * destroyed completely. Asteroids can merge through collisions, combining
 * their masses and momenta. They participate in N-body gravity and can
 * create dramatic gravitational dynamics.
 */
struct Asteroid : public Body {
    float radius;          ///< Collision and visual radius
    int size;              ///< Size class: 0=large, 1=medium, 2=small
    int vertices;          ///< Number of vertices for polygon rendering
    float rotation;        ///< Current rotation angle for visual variety
    float rotationSpeed;   ///< Angular velocity (radians/second)

    /**
     * @brief Default constructor
     */
    Asteroid();

    /**
     * @brief Initialize asteroid with size-dependent properties
     * @param id Unique entity ID
     * @param pos Starting position
     * @param vel Initial velocity
     * @param size Size class (0=large, 1=medium, 2=small)
     * @param baseMass Mass of a large asteroid (smaller sizes use half mass)
     */
    void init(int id, Vec2 pos, Vec2 vel, int size, float baseMass = 12000.0f);

    /**
     * @brief Update rotation animation
     * @param dt Time step in seconds
     */
    void update(float dt);
};

/**
 * @struct Bullet
 * @brief Projectile fired by ships to destroy asteroids
 *
 * Bullets have a limited lifetime and participate in N-body gravity,
 * allowing for curved trajectories in gravitational fields. When bullets
 * hit asteroids, they can either destroy small asteroids or merge into
 * larger asteroids, adding their momentum and mass.
 */
struct Bullet : public Body {
    int playerId;       ///< Owner player ID (for scoring and color)
    float lifetime;     ///< Time remaining before auto-destruction (seconds)
    float maxLifetime;  ///< Initial lifetime for fade calculations
    float radius;       ///< Collision radius

    /**
     * @brief Default constructor
     */
    Bullet();

    /**
     * @brief Initialize bullet with velocity and ownership
     * @param id Unique entity ID
     * @param pos Starting position (usually ship position + offset)
     * @param vel Velocity (usually ship velocity + forward impulse)
     * @param playerId Owner player ID
     */
    void init(int id, Vec2 pos, Vec2 vel, int playerId);

    /**
     * @brief Update lifetime and deactivate when expired
     * @param dt Time step in seconds
     */
    void update(float dt);
};

/**
 * @struct BlackHole
 * @brief Massive gravitational hazard that accretes nearby objects
 *
 * Black holes have extreme mass and an accretion radius. Any object that
 * enters the accretion radius is destroyed (accreted). Black holes drift
 * slowly and are removed when they leave the screen. They create dramatic
 * gravitational effects, bending trajectories of all nearby bodies.
 */
struct BlackHole : public Body {
    float accretionRadius;  ///< Radius within which objects are destroyed
    float visualRadius;     ///< Event horizon rendering size

    /**
     * @brief Default constructor
     */
    BlackHole();

    /**
     * @brief Initialize black hole with mass and danger zone
     * @param id Unique entity ID
     * @param pos Starting position (usually at edge of screen)
     * @param vel Drift velocity (typically slow)
     * @param mass Gravitational mass (typically very large)
     * @param accretionRadius Destruction radius
     */
    void init(int id, Vec2 pos, Vec2 vel, float mass, float accretionRadius);

    /**
     * @brief Check if black hole has drifted off screen
     * @param worldWidth Simulation domain width
     * @param worldHeight Simulation domain height
     * @return True if completely outside visible area
     */
    bool isOffscreen(float worldWidth, float worldHeight) const;
};

/**
 * @struct Particle
 * @brief Visual debris from explosions (non-colliding)
 *
 * Particles are created by ship deaths, asteroid destruction, and accretion
 * events. They move ballistically without gravitational influence, fade out
 * over their lifetime, and are colored based on their source (white for
 * asteroids, green/cyan for ships matching player colors).
 */
struct Particle : public Body {
    float lifetime;      ///< Time remaining before particle fades out
    float maxLifetime;   ///< Initial lifetime for alpha fade calculation
    int playerId;        ///< Color indicator: -1=white, 0=green, 1=cyan

    /**
     * @brief Default constructor
     */
    Particle();

    /**
     * @brief Initialize particle with velocity and color
     * @param pos Starting position (explosion center)
     * @param vel Initial velocity (radial from explosion)
     * @param playerId Color code (-1 for white, 0/1 for ship colors)
     */
    void init(Vec2 pos, Vec2 vel, int playerId = -1);

    /**
     * @brief Update position and lifetime (particles don't obey gravity)
     * @param dt Time step in seconds
     *
     * Particles move ballistically based on initial velocity without
     * gravitational acceleration, fading out as lifetime decreases.
     */
    void update(float dt);
};
