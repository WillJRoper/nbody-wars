#pragma once
#include <cmath>

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }

    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float lengthSquared() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSquared()); }
    Vec2 normalized() const { float len = length(); return len > 0 ? *this / len : Vec2(0, 0); }
    float dot(const Vec2& other) const { return x * other.x + y * other.y; }

    // Rotate by angle (radians)
    Vec2 rotated(float angle) const {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Vec2(x * c - y * s, x * s + y * c);
    }
};

inline Vec2 operator*(float s, const Vec2& v) { return v * s; }
