/**
 * @file potential.h
 * @brief External gravitational potential fields for the simulation
 *
 * Provides various static gravitational potentials that create different
 * orbital dynamics environments. All bodies experience acceleration from
 * the selected potential in addition to N-body mutual gravity.
 *
 * Available potentials:
 * - No Potential: Pure N-body dynamics
 * - Point Mass: Keplerian orbits around central mass
 * - Harmonic: Oscillatory motion with restoring force
 * - Logarithmic: Flat rotation curves (spiral galaxy-like)
 * - NFW: Dark matter halo profile
 */

#pragma once
#include "vec2.h"
#include <memory>

/**
 * @class IExternalPotential
 * @brief Abstract interface for external gravitational potentials
 *
 * All potential implementations must provide:
 * - Acceleration calculation at any position
 * - Human-readable name and description
 */
class IExternalPotential {
public:
    virtual ~IExternalPotential() = default;

    /**
     * @brief Calculate acceleration due to external potential at a position
     * @param pos Position at which to evaluate the potential
     * @return Acceleration vector
     */
    virtual Vec2 accelerationAt(const Vec2& pos) const = 0;

    /**
     * @brief Get the name of this potential
     * @return Short name string
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Get a description of this potential
     * @return Description string with physics details
     */
    virtual const char* getDescription() const = 0;
};

/**
 * @class NoPotential
 * @brief No external potential - pure N-body dynamics
 *
 * Returns zero acceleration everywhere. Bodies only experience
 * mutual gravitational attraction from other bodies.
 */
class NoPotential : public IExternalPotential {
public:
    /**
     * @brief Calculate acceleration (always zero)
     * @param pos Position (unused)
     * @return Zero vector
     */
    Vec2 accelerationAt(const Vec2& pos) const override {
        return Vec2(0, 0);
    }

    const char* getName() const override { return "No Potential"; }
    const char* getDescription() const override {
        return "Free space with no external forces. Only mutual gravity between bodies.";
    }
};

/**
 * @class PointMassPotential
 * @brief Central point mass creating Keplerian orbits
 *
 * Models a massive central body (like a star or black hole).
 * Acceleration: a(r) = -GM * r / (r² + ε²)^(3/2)
 *
 * Softening length ε prevents singularities at r=0.
 * Creates circular, elliptical, parabolic, or hyperbolic orbits
 * depending on velocity and radius.
 */
class PointMassPotential : public IExternalPotential {
public:
    /**
     * @brief Construct point mass potential
     * @param center Position of the central mass
     * @param GM Gravitational parameter (G × mass)
     * @param eps Softening length to prevent singularities
     */
    PointMassPotential(Vec2 center, float GM, float eps)
        : center(center), GM(GM), eps(eps) {}

    /**
     * @brief Calculate acceleration toward central mass
     * @param pos Position at which to calculate acceleration
     * @return Acceleration vector pointing toward center, magnitude ∝ 1/r²
     */
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
    Vec2 center;  ///< Position of central mass
    float GM;     ///< Gravitational parameter (G × mass)
    float eps;    ///< Softening length
};

/**
 * @class HarmonicPotential
 * @brief Isotropic harmonic oscillator potential
 *
 * Creates a linear restoring force toward the center.
 * Acceleration: a(r) = -ω² * r
 *
 * Results in simple harmonic motion - all orbits are ellipses
 * with the same angular frequency ω regardless of amplitude.
 * Unique property: orbital period independent of radius.
 */
class HarmonicPotential : public IExternalPotential {
public:
    /**
     * @brief Construct harmonic potential
     * @param center Center of the potential well
     * @param omega2 Square of angular frequency (ω²)
     */
    HarmonicPotential(Vec2 center, float omega2)
        : center(center), omega2(omega2) {}

    /**
     * @brief Calculate linear restoring acceleration
     * @param pos Position at which to calculate acceleration
     * @return Acceleration proportional to displacement from center
     */
    Vec2 accelerationAt(const Vec2& pos) const override {
        Vec2 dr = pos - center;
        return dr * (-omega2);
    }

    const char* getName() const override { return "Harmonic Oscillator"; }
    const char* getDescription() const override {
        return "Harmonic potential: a(r) = -omega^2 * r. Creates oscillatory orbits.";
    }

private:
    Vec2 center;   ///< Center of oscillator
    float omega2;  ///< Square of angular frequency
};

/**
 * @class LogarithmicPotential
 * @brief Logarithmic potential with flat rotation curve
 *
 * Models a disk galaxy with constant circular velocity.
 * Potential: V(r) = v₀² * ln(r² + r_c²)
 * Acceleration: a(r) = -v₀² * r / (r² + r_c²)
 *
 * Produces flat rotation curves like those observed in spiral galaxies,
 * historically motivating dark matter. Core radius r_c prevents
 * singularity at center.
 */
class LogarithmicPotential : public IExternalPotential {
public:
    /**
     * @brief Construct logarithmic potential
     * @param center Center of the potential
     * @param v0 Circular velocity (asymptotic value at large r)
     * @param rc Core radius (softens central behavior)
     */
    LogarithmicPotential(Vec2 center, float v0, float rc)
        : center(center), v0(v0), rc(rc) {}

    /**
     * @brief Calculate acceleration for flat rotation curve
     * @param pos Position at which to calculate acceleration
     * @return Acceleration giving v_circular ≈ v₀ at large r
     */
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
    Vec2 center;  ///< Center of potential
    float v0;     ///< Circular velocity
    float rc;     ///< Core radius
};

/**
 * @class NFWPotential
 * @brief Navarro-Frenk-White dark matter halo profile
 *
 * Models a cosmological dark matter halo with density profile:
 * ρ(r) = ρ_s / [(r/r_s)(1 + r/r_s)²]
 *
 * This profile emerges naturally from N-body cosmological simulations.
 * Acceleration computed from enclosed mass:
 * M(<r) = 4π ρ_s r_s³ [ln(1+x) - x/(1+x)] where x = r/r_s
 *
 * Produces cuspy density profile at small r and ρ ∝ r⁻³ at large r.
 */
class NFWPotential : public IExternalPotential {
public:
    /**
     * @brief Construct NFW halo potential
     * @param center Center of the halo
     * @param rho_s Characteristic density
     * @param r_s Scale radius (where d(log ρ)/d(log r) = -2)
     * @param G Gravitational constant
     * @param eps Softening length
     */
    NFWPotential(Vec2 center, float rho_s, float r_s, float G, float eps)
        : center(center), rho_s(rho_s), r_s(r_s), G(G), eps(eps) {}

    /**
     * @brief Calculate acceleration from NFW enclosed mass
     * @param pos Position at which to calculate acceleration
     * @return Acceleration from spherically symmetric mass distribution
     *
     * Uses spherical approximation: only radial distance from center matters.
     * Computes enclosed mass M(<r) analytically, then applies
     * a = -GM(<r)/r² with softening.
     */
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

        // a(r) = -G * M(<r) / (r^2 + eps^2)^(3/2)
        float r2_soft = r * r + eps * eps;
        float factor = -G * M_enc / (r2_soft * std::sqrt(r2_soft));

        return dr * factor;
    }

    const char* getName() const override { return "NFW Profile"; }
    const char* getDescription() const override {
        return "Navarro-Frenk-White dark matter halo: ρ(r) ∝ 1/(r(1+r/rs)^2)";
    }

private:
    Vec2 center;   ///< Center of halo
    float rho_s;   ///< Characteristic density
    float r_s;     ///< Scale radius
    float G;       ///< Gravitational constant
    float eps;     ///< Softening length
};

/**
 * @brief Factory function to create potential by level ID
 * @param levelId Integer identifying the potential type (0-4)
 * @param worldCenter Center position for the potential
 * @param worldWidth Width of simulation domain (used for scaling)
 * @return Unique pointer to created potential
 *
 * Level mapping:
 * - 0: No Potential
 * - 1: Point Mass
 * - 2: Harmonic
 * - 3: Logarithmic
 * - 4: NFW Profile
 */
std::unique_ptr<IExternalPotential> createPotential(int levelId, Vec2 worldCenter, float worldWidth);
