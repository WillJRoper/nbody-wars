/**
 * @file quadtree.cpp
 * @brief Implementation of Barnes-Hut quadtree for N-body gravity
 *
 * Implements the Barnes-Hut algorithm to accelerate N-body gravitational
 * calculations from O(N²) to O(N log N). The quadtree recursively subdivides
 * 2D space, storing aggregate mass properties at each node. Distant node
 * clusters are approximated as single masses, controlled by the opening
 * angle criterion (theta).
 *
 * Key algorithm details:
 * - Leaf nodes contain at most one body
 * - Internal nodes store center of mass and total mass of subtree
 * - Opening criterion: s/d < theta (s=node size, d=distance, theta~0.5)
 * - Supports periodic boundary conditions via minimum image convention
 */

#include "quadtree.h"
#include "entity.h"
#include <algorithm>

/**
 * @brief Construct a quadtree node
 * @param center Geometric center of node's spatial region
 * @param halfSize Half-width/height of square region
 *
 * Initializes empty node as a leaf with no mass. Children are created
 * lazily when subdivision is needed.
 */
QuadTreeNode::QuadTreeNode(Vec2 center, float halfSize)
    : center(center), halfSize(halfSize), totalMass(0), body(nullptr), isLeaf(true) {
}

/**
 * @brief Determine which quadrant contains a position
 * @param pos Position to classify
 * @return Quadrant index: 0=NW, 1=NE, 2=SW, 3=SE
 *
 * Uses bitwise encoding: bit 0 = East (x >= center.x), bit 1 = South (y >= center.y)
 */
int QuadTreeNode::getQuadrant(const Vec2& pos) const {
    int quad = 0;
    if (pos.x >= center.x) quad |= 1;  // East
    if (pos.y >= center.y) quad |= 2;  // South
    return quad;
}

/**
 * @brief Subdivide this node into four children
 *
 * Creates four child nodes representing the quadrants NW, NE, SW, SE.
 * Each child has half the linear size of the parent. Called when a leaf
 * node needs to store a second body.
 */
void QuadTreeNode::subdivide() {
    float newHalfSize = halfSize * 0.5f;
    children[0] = std::make_unique<QuadTreeNode>(
        Vec2(center.x - newHalfSize, center.y - newHalfSize), newHalfSize); // NW
    children[1] = std::make_unique<QuadTreeNode>(
        Vec2(center.x + newHalfSize, center.y - newHalfSize), newHalfSize); // NE
    children[2] = std::make_unique<QuadTreeNode>(
        Vec2(center.x - newHalfSize, center.y + newHalfSize), newHalfSize); // SW
    children[3] = std::make_unique<QuadTreeNode>(
        Vec2(center.x + newHalfSize, center.y + newHalfSize), newHalfSize); // SE
    isLeaf = false;
}

/**
 * @brief Insert a body into the quadtree
 * @param b Pointer to body to insert
 * @param worldWidth Width of simulation domain (for periodic boundaries)
 * @param worldHeight Height of simulation domain (for periodic boundaries)
 *
 * Recursively inserts body and updates center of mass along the path.
 * If inserting into a leaf that already contains a body, subdivides the
 * leaf and redistributes both bodies. Center of mass is computed using
 * mass-weighted averaging: COM = (m1*r1 + m2*r2) / (m1 + m2)
 */
void QuadTreeNode::insert(Body* b, float worldWidth, float worldHeight) {
    if (isLeaf) {
        if (body == nullptr) {
            // Empty leaf - just store the body
            body = b;
            centerOfMass = b->pos;
            totalMass = b->mass;
        } else {
            // Leaf already has a body - subdivide
            Body* existingBody = body;
            body = nullptr;
            subdivide();

            // Reinsert existing body
            int quad = getQuadrant(existingBody->pos);
            children[quad]->insert(existingBody, worldWidth, worldHeight);

            // Insert new body
            quad = getQuadrant(b->pos);
            children[quad]->insert(b, worldWidth, worldHeight);

            // Update center of mass
            float m1 = existingBody->mass;
            float m2 = b->mass;
            totalMass = m1 + m2;
            centerOfMass = (existingBody->pos * m1 + b->pos * m2) / totalMass;
        }
    } else {
        // Internal node - insert into appropriate child
        int quad = getQuadrant(b->pos);
        children[quad]->insert(b, worldWidth, worldHeight);

        // Update center of mass
        float oldMass = totalMass;
        Vec2 oldCOM = centerOfMass;
        totalMass += b->mass;
        if (totalMass > 0) {
            centerOfMass = (oldCOM * oldMass + b->pos * b->mass) / totalMass;
        }
    }
}

/**
 * @brief Calculate gravitational acceleration using Barnes-Hut algorithm
 * @param pos Position at which to calculate acceleration
 * @param mass Mass of the body (for self-interaction exclusion)
 * @param theta Opening angle criterion (typically 0.5)
 * @param eps Softening length to prevent singularities
 * @param G Gravitational constant
 * @param worldWidth Width for periodic boundary calculations
 * @param worldHeight Height for periodic boundary calculations
 * @return Gravitational acceleration vector
 *
 * Implementation of Barnes-Hut approximation:
 * - For leaf nodes: compute direct force (excluding self-interaction)
 * - For internal nodes: check opening criterion s/d < theta
 *   - If satisfied: treat node as single mass at center of mass
 *   - Otherwise: recurse into children for higher accuracy
 * Uses softened gravity: a = G*M*r / (r² + ε²)^(3/2) to prevent singularities
 * Periodic boundaries handled via minimum image convention
 */
Vec2 QuadTreeNode::calculateAcceleration(const Vec2& pos, float mass, float theta,
                                         float eps, float G,
                                         float worldWidth, float worldHeight) const {
    if (totalMass == 0) return Vec2(0, 0);

    // Calculate distance using minimum image convention
    Vec2 dr = minimumImage(centerOfMass - pos, worldWidth, worldHeight);
    float r2 = dr.lengthSquared();

    if (isLeaf) {
        // Leaf node - calculate direct force
        if (body && body->pos.x == pos.x && body->pos.y == pos.y && body->mass == mass) {
            // Same body - no self-interaction
            return Vec2(0, 0);
        }

        float r3 = std::pow(r2 + eps * eps, 1.5f);
        return dr * (G * totalMass / r3);
    } else {
        // Internal node - check opening criterion
        float r = std::sqrt(r2);
        float s = halfSize * 2.0f;  // Node size

        if (s / r < theta) {
            // Node is far enough - use approximation
            float r3 = std::pow(r2 + eps * eps, 1.5f);
            return dr * (G * totalMass / r3);
        } else {
            // Node is too close - recurse into children
            Vec2 acc(0, 0);
            for (int i = 0; i < 4; i++) {
                if (children[i]) {
                    acc += children[i]->calculateAcceleration(pos, mass, theta, eps, G,
                                                             worldWidth, worldHeight);
                }
            }
            return acc;
        }
    }
}

// ============================================================================
// QuadTree wrapper class implementation
// ============================================================================

/**
 * @brief Construct a quadtree for the simulation domain
 * @param width Width of simulation world
 * @param height Height of simulation world
 *
 * Creates root node centered at (width/2, height/2) with size large enough
 * to contain the entire domain. Uses max(width, height) to handle non-square
 * domains.
 */
QuadTree::QuadTree(float width, float height)
    : worldWidth(width), worldHeight(height) {
    root = std::make_unique<QuadTreeNode>(
        Vec2(width * 0.5f, height * 0.5f),
        std::max(width, height) * 0.5f
    );
}

/**
 * @brief Build the tree from a collection of bodies
 * @param bodies Vector of body pointers to insert
 *
 * Reconstructs the tree from scratch. Should be called after all bodies
 * have moved (typically after the drift step in leapfrog integration).
 * Creates a new root node and inserts all bodies, building the spatial
 * hierarchy bottom-up.
 */
void QuadTree::build(std::vector<Body*>& bodies) {
    root = std::make_unique<QuadTreeNode>(
        Vec2(worldWidth * 0.5f, worldHeight * 0.5f),
        std::max(worldWidth, worldHeight) * 0.5f
    );

    for (Body* body : bodies) {
        root->insert(body, worldWidth, worldHeight);
    }
}

Vec2 QuadTree::calculateAcceleration(const Vec2& pos, float mass,
                                     float theta, float eps, float G) const {
    return root->calculateAcceleration(pos, mass, theta, eps, G, worldWidth, worldHeight);
}
