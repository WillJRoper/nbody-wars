#pragma once
#include "vec2.h"
#include <memory>

// Interface for external potentials
class IExternalPotential {
public:
    virtual ~IExternalPotential() = default;
    virtual Vec2 accelerationAt(const Vec2& pos) const = 0;
    virtual const char* getName() const = 0;
    virtual const char* getDescription() const = 0;
};

// 1. No potential
class NoPotential : public IExternalPotential {
public:
    Vec2 accelerationAt(const Vec2& pos) const override {
        return Vec2(0, 0);
    }

    const char* getName() const override { return "No Potential"; }
    const char* getDescription() const override {
        return "Free space with no external forces. Only mutual gravity between bodies.";
    }
};

// 2. Central point mass potential
class PointMassPotential : public IExternalPotential {
public:
    PointMassPotential(Vec2 center, float GM, float eps)
        : center(center), GM(GM), eps(eps) {}

    Vec2 accelerationAt(const Vec2& pos) const override {
        Vec2 dr = center - pos;
        float r2 = dr.lengthSquared();
        float r3 = std::pow(r2 + eps * eps, 1.5f);
        return dr * (GM / r3);
    }

    const char* getName() const override { return "Point Mass"; }
    const char* getDescription() const override {
        return "Central gravitational potential: a(r) = -GM * r / (r^2 + eps^2)^1.5";
    }

private:
    Vec2 center;
    float GM;    // Gravitational parameter
    float eps;   // Softening length
};

// 3. Harmonic oscillator potential
class HarmonicPotential : public IExternalPotential {
public:
    HarmonicPotential(Vec2 center, float omega2)
        : center(center), omega2(omega2) {}

    Vec2 accelerationAt(const Vec2& pos) const override {
        Vec2 dr = pos - center;
        return dr * (-omega2);
    }

    const char* getName() const override { return "Harmonic Oscillator"; }
    const char* getDescription() const override {
        return "Harmonic potential: a(r) = -omega^2 * r. Creates oscillatory orbits.";
    }

private:
    Vec2 center;
    float omega2;  // omega^2
};

// 4. Logarithmic potential
class LogarithmicPotential : public IExternalPotential {
public:
    LogarithmicPotential(Vec2 center, float v0, float rc)
        : center(center), v0(v0), rc(rc) {}

    Vec2 accelerationAt(const Vec2& pos) const override {
        Vec2 dr = pos - center;
        float r2 = dr.lengthSquared();
        float r = std::sqrt(r2);
        if (r < 1e-6f) return Vec2(0, 0);

        // a(r) = -v0^2 * r / (r^2 + rc^2)
        float factor = -v0 * v0 / (r2 + rc * rc);
        return dr * factor;
    }

    const char* getName() const override { return "Logarithmic"; }
    const char* getDescription() const override {
        return "Logarithmic potential: V(r) = v0^2 * ln(r^2 + rc^2). Flat rotation curve.";
    }

private:
    Vec2 center;
    float v0;    // Circular velocity
    float rc;    // Core radius
};

// 5. NFW (Navarro-Frenk-White) profile potential
class NFWPotential : public IExternalPotential {
public:
    NFWPotential(Vec2 center, float rho_s, float r_s, float G, float eps)
        : center(center), rho_s(rho_s), r_s(r_s), G(G), eps(eps) {}

    Vec2 accelerationAt(const Vec2& pos) const override {
        Vec2 dr = pos - center;
        float r = dr.length();
        if (r < 1e-6f) return Vec2(0, 0);

        // NFW enclosed mass: M(<r) = 4π ρ_s r_s^3 [ln(1+x) - x/(1+x)]
        // where x = r/r_s
        float x = r / r_s;
        float ln_term = std::log(1.0f + x);
        float frac_term = x / (1.0f + x);
        float M_enc = 4.0f * 3.14159265f * rho_s * r_s * r_s * r_s * (ln_term - frac_term);

        // a(r) = -G * M(<r) / (r^2 + eps^2)
        float r2_soft = r * r + eps * eps;
        float factor = -G * M_enc / (r2_soft * std::sqrt(r2_soft));

        return dr * factor;
    }

    const char* getName() const override { return "NFW Profile"; }
    const char* getDescription() const override {
        return "Navarro-Frenk-White dark matter halo: ρ(r) ∝ 1/(r(1+r/rs)^2)";
    }

private:
    Vec2 center;
    float rho_s;  // Characteristic density
    float r_s;    // Scale radius
    float G;      // Gravitational constant
    float eps;    // Softening
};

// Factory function to create potentials
std::unique_ptr<IExternalPotential> createPotential(int levelId, Vec2 worldCenter, float worldWidth);
