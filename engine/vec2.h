/**
 * @file vec2.h
 * @brief 2D vector mathematics for N-body physics simulation
 *
 * Provides a lightweight Vec2 struct with standard vector operations
 * including arithmetic, dot product, normalization, and rotation.
 */

#pragma once
#include <cmath>

/**
 * @struct Vec2
 * @brief A 2D vector with floating-point components
 *
 * Represents positions, velocities, accelerations, and forces
 * in the 2D simulation space. Provides operator overloading for
 * convenient vector arithmetic.
 */
struct Vec2 {
    float x; ///< X component
    float y; ///< Y component

    /**
     * @brief Default constructor - initializes to zero vector
     */
    Vec2() : x(0), y(0) {}

    /**
     * @brief Construct from components
     * @param x X component
     * @param y Y component
     */
    Vec2(float x, float y) : x(x), y(y) {}

    /**
     * @brief Vector addition
     * @param other Vector to add
     * @return Sum of vectors
     */
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }

    /**
     * @brief Vector subtraction
     * @param other Vector to subtract
     * @return Difference of vectors
     */
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }

    /**
     * @brief Scalar multiplication
     * @param s Scalar value
     * @return Scaled vector
     */
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }

    /**
     * @brief Scalar division
     * @param s Scalar divisor
     * @return Scaled vector
     */
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }

    /**
     * @brief In-place vector addition
     * @param other Vector to add
     * @return Reference to this vector
     */
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }

    /**
     * @brief In-place vector subtraction
     * @param other Vector to subtract
     * @return Reference to this vector
     */
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }

    /**
     * @brief In-place scalar multiplication
     * @param s Scalar value
     * @return Reference to this vector
     */
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    /**
     * @brief Calculate squared magnitude (avoids sqrt)
     * @return |v|² = x² + y²
     * @note Faster than length() for distance comparisons
     */
    float lengthSquared() const { return x * x + y * y; }

    /**
     * @brief Calculate vector magnitude
     * @return |v| = √(x² + y²)
     */
    float length() const { return std::sqrt(lengthSquared()); }

    /**
     * @brief Return unit vector in same direction
     * @return Normalized vector (length = 1) or zero if length = 0
     */
    Vec2 normalized() const { float len = length(); return len > 0 ? *this / len : Vec2(0, 0); }

    /**
     * @brief Calculate dot product with another vector
     * @param other The other vector
     * @return v · other = x*other.x + y*other.y
     * @note Useful for projections and angle calculations
     */
    float dot(const Vec2& other) const { return x * other.x + y * other.y; }

    /**
     * @brief Rotate vector by angle
     * @param angle Rotation angle in radians (positive = counter-clockwise)
     * @return Rotated vector
     * @note Uses standard 2D rotation matrix: [cos -sin; sin cos]
     */
    Vec2 rotated(float angle) const {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Vec2(x * c - y * s, x * s + y * c);
    }
};

/**
 * @brief Scalar multiplication (commutative)
 * @param s Scalar value
 * @param v Vector
 * @return s * v
 * @note Allows writing scalar * vector in addition to vector * scalar
 */
inline Vec2 operator*(float s, const Vec2& v) { return v * s; }
