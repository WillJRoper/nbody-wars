/**
 * @file quadtree.h
 * @brief Barnes-Hut quadtree for efficient N-body gravity calculations
 *
 * Implements the Barnes-Hut algorithm to reduce O(N²) pairwise force
 * calculations to O(N log N) using spatial hierarchical grouping.
 * Also provides periodic boundary condition utilities.
 */

#pragma once
#include "vec2.h"
#include <vector>
#include <memory>

// Forward declarations
struct Body;

/**
 * @class QuadTreeNode
 * @brief A node in the Barnes-Hut quadtree
 *
 * Recursively subdivides 2D space into quadrants. Leaf nodes contain
 * individual bodies, while internal nodes store aggregate mass properties
 * (center of mass and total mass) for efficient far-field approximations.
 *
 * The four children represent quadrants in order: NW, NE, SW, SE
 */
class QuadTreeNode {
public:
    Vec2 center;      ///< Geometric center of this node's region
    float halfSize;   ///< Half-width/height of the square region

    // Aggregate mass properties for Barnes-Hut approximation
    Vec2 centerOfMass;  ///< Mass-weighted position of all bodies in subtree
    float totalMass;    ///< Sum of masses of all bodies in subtree

    /// Child nodes for quadrants: [0]=NW, [1]=NE, [2]=SW, [3]=SE
    std::unique_ptr<QuadTreeNode> children[4];

    Body* body;   ///< Pointer to body (only valid for leaf nodes)
    bool isLeaf;  ///< True if this is a leaf node containing a single body

    /**
     * @brief Construct a quadtree node
     * @param center Geometric center of this node's spatial region
     * @param halfSize Half of the width/height of the square region
     */
    QuadTreeNode(Vec2 center, float halfSize);

    /**
     * @brief Insert a body into the quadtree
     * @param b Pointer to the body to insert
     * @param worldWidth Width of the simulation domain
     * @param worldHeight Height of the simulation domain
     *
     * Recursively subdivides if necessary. When a leaf node receives a second
     * body, it subdivides into four children and redistributes both bodies.
     */
    void insert(Body* b, float worldWidth, float worldHeight);

    /**
     * @brief Calculate gravitational acceleration using Barnes-Hut algorithm
     * @param pos Position at which to calculate acceleration
     * @param mass Mass of the body being accelerated (for self-gravity exclusion)
     * @param theta Opening angle criterion (typically ~0.5)
     * @param eps Softening length to prevent singularities
     * @param G Gravitational constant
     * @param worldWidth Width for periodic boundary calculations
     * @param worldHeight Height for periodic boundary calculations
     * @return Gravitational acceleration vector
     *
     * Uses opening angle criterion: s/d < theta, where s is node size and d is distance.
     * If criterion met, treats entire node as a single mass at center of mass.
     * Otherwise, recursively evaluates children.
     */
    Vec2 calculateAcceleration(const Vec2& pos, float mass, float theta,
                               float eps, float G, float worldWidth, float worldHeight) const;

private:
    /**
     * @brief Determine which quadrant contains a position
     * @param pos Position to check
     * @return Quadrant index: 0=NW, 1=NE, 2=SW, 3=SE
     */
    int getQuadrant(const Vec2& pos) const;

    /**
     * @brief Subdivide this node into four children
     *
     * Creates four child nodes representing the four quadrants.
     * Called when a leaf node needs to accept a second body.
     */
    void subdivide();
};

/**
 * @class QuadTree
 * @brief Container for the Barnes-Hut quadtree
 *
 * Manages the root node and provides the interface for building
 * the tree and querying accelerations.
 */
class QuadTree {
public:
    /**
     * @brief Construct a quadtree for the simulation domain
     * @param width Width of the simulation world
     * @param height Height of the simulation world
     */
    QuadTree(float width, float height);

    /**
     * @brief Build the tree from a collection of bodies
     * @param bodies Vector of body pointers to insert into the tree
     *
     * Reconstructs the tree from scratch each time. Should be called
     * after all bodies have moved (after the drift step in leapfrog).
     */
    void build(std::vector<Body*>& bodies);

    /**
     * @brief Calculate gravitational acceleration at a position
     * @param pos Position at which to calculate acceleration
     * @param mass Mass of the body (for self-exclusion)
     * @param theta Opening angle criterion
     * @param eps Softening length
     * @param G Gravitational constant
     * @return Gravitational acceleration vector from all bodies
     */
    Vec2 calculateAcceleration(const Vec2& pos, float mass, float theta,
                               float eps, float G) const;

private:
    float worldWidth;   ///< Width of simulation domain
    float worldHeight;  ///< Height of simulation domain
    std::unique_ptr<QuadTreeNode> root;  ///< Root node of the tree
};

/**
 * @brief Calculate minimum image displacement for periodic boundaries
 * @param dr Displacement vector (destination - source)
 * @param worldWidth Width of the periodic domain
 * @param worldHeight Height of the periodic domain
 * @return Adjusted displacement vector using nearest-image convention
 *
 * Implements torus topology: if displacement exceeds half the box size,
 * uses the shorter path across the periodic boundary. Essential for
 * correctly calculating forces and distances in periodic systems.
 *
 * @note Used for force calculations and collision detection
 */
inline Vec2 minimumImage(Vec2 dr, float worldWidth, float worldHeight) {
    if (dr.x > worldWidth * 0.5f) dr.x -= worldWidth;
    if (dr.x < -worldWidth * 0.5f) dr.x += worldWidth;
    if (dr.y > worldHeight * 0.5f) dr.y -= worldHeight;
    if (dr.y < -worldHeight * 0.5f) dr.y += worldHeight;
    return dr;
}

/**
 * @brief Wrap position to stay within periodic boundaries
 * @param pos Position to wrap
 * @param worldWidth Width of the periodic domain
 * @param worldHeight Height of the periodic domain
 * @return Wrapped position inside [0, worldWidth) × [0, worldHeight)
 *
 * Ensures positions remain in the primary simulation cell by
 * wrapping around periodic boundaries.
 */
inline Vec2 wrapPosition(Vec2 pos, float worldWidth, float worldHeight) {
    while (pos.x < 0) pos.x += worldWidth;
    while (pos.x >= worldWidth) pos.x -= worldWidth;
    while (pos.y < 0) pos.y += worldHeight;
    while (pos.y >= worldHeight) pos.y -= worldHeight;
    return pos;
}
