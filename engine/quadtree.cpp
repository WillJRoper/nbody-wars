#include "quadtree.h"
#include "entity.h"
#include <algorithm>

QuadTreeNode::QuadTreeNode(Vec2 center, float halfSize)
    : center(center), halfSize(halfSize), totalMass(0), body(nullptr), isLeaf(true) {
}

int QuadTreeNode::getQuadrant(const Vec2& pos) const {
    int quad = 0;
    if (pos.x >= center.x) quad |= 1;  // East
    if (pos.y >= center.y) quad |= 2;  // South
    return quad;
}

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

QuadTree::QuadTree(float width, float height)
    : worldWidth(width), worldHeight(height) {
    root = std::make_unique<QuadTreeNode>(
        Vec2(width * 0.5f, height * 0.5f),
        std::max(width, height) * 0.5f
    );
}

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
