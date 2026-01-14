#pragma once
#include "vec2.h"
#include <vector>
#include <memory>

// Forward declarations
struct Body;

// Barnes-Hut quadtree node
class QuadTreeNode {
public:
    Vec2 center;
    float halfSize;

    // Center of mass and total mass
    Vec2 centerOfMass;
    float totalMass;

    // Children (NW, NE, SW, SE)
    std::unique_ptr<QuadTreeNode> children[4];

    // Body (only for leaf nodes)
    Body* body;
    bool isLeaf;

    QuadTreeNode(Vec2 center, float halfSize);

    // Insert a body into the tree
    void insert(Body* b, float worldWidth, float worldHeight);

    // Calculate acceleration on a body using Barnes-Hut
    Vec2 calculateAcceleration(const Vec2& pos, float mass, float theta,
                               float eps, float G, float worldWidth, float worldHeight) const;

private:
    int getQuadrant(const Vec2& pos) const;
    void subdivide();
};

class QuadTree {
public:
    QuadTree(float width, float height);

    void build(std::vector<Body*>& bodies);
    Vec2 calculateAcceleration(const Vec2& pos, float mass, float theta,
                               float eps, float G) const;

private:
    float worldWidth, worldHeight;
    std::unique_ptr<QuadTreeNode> root;
};

// Calculate minimum image distance for periodic boundaries (torus)
inline Vec2 minimumImage(Vec2 dr, float worldWidth, float worldHeight) {
    if (dr.x > worldWidth * 0.5f) dr.x -= worldWidth;
    if (dr.x < -worldWidth * 0.5f) dr.x += worldWidth;
    if (dr.y > worldHeight * 0.5f) dr.y -= worldHeight;
    if (dr.y < -worldHeight * 0.5f) dr.y += worldHeight;
    return dr;
}

// Apply periodic wrap to position
inline Vec2 wrapPosition(Vec2 pos, float worldWidth, float worldHeight) {
    while (pos.x < 0) pos.x += worldWidth;
    while (pos.x >= worldWidth) pos.x -= worldWidth;
    while (pos.y < 0) pos.y += worldHeight;
    while (pos.y >= worldHeight) pos.y -= worldHeight;
    return pos;
}
