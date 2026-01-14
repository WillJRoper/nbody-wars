/**
 * @file potential.cpp
 * @brief Implementation of potential factory function
 *
 * Creates instances of external gravitational potentials with physics-based
 * parameter tuning. Each level provides a different orbital dynamics
 * environment for gameplay variety.
 */

#include "potential.h"

/**
 * @brief Factory function to create potential by level ID
 * @param levelId Level identifier (0-4)
 * @param worldCenter Center position for potential
 * @param worldWidth Width of simulation domain (for scaling parameters)
 * @return Unique pointer to created potential
 *
 * Parameter tuning notes:
 * - Level 1 (Point Mass): GM=50000 gives stable circular orbits at typical distances
 * - Level 2 (Harmonic): omega²=0.0001 creates gentle oscillatory motion
 * - Level 3 (Logarithmic): v0=10 with rc=0.1*width provides mild rotation
 * - Level 4 (NFW): rho_s and r_s tuned for realistic dark matter halo effects
 */
std::unique_ptr<IExternalPotential> createPotential(int levelId, Vec2 worldCenter, float worldWidth) {
    switch (levelId) {
        case 0:
            return std::make_unique<NoPotential>();

        case 1: {
            // Central point mass
            float GM = 50000.0f;
            float eps = 20.0f;
            return std::make_unique<PointMassPotential>(worldCenter, GM, eps);
        }

        case 2: {
            // Harmonic oscillator
            float omega2 = 0.0001f;  // omega^2
            return std::make_unique<HarmonicPotential>(worldCenter, omega2);
        }

        case 3: {
            // Logarithmic potential
            float v0 = 10.0f;           // Circular velocity
            float rc = worldWidth * 0.1f;  // Core radius
            return std::make_unique<LogarithmicPotential>(worldCenter, v0, rc);
        }

        case 4: {
            // NFW profile
            float rho_s = 0.004f;         // Characteristic density
            float r_s = worldWidth * 0.2f;   // Scale radius
            float G = 50.0f;              // Gravitational constant
            float eps = 10.0f;            // Softening
            return std::make_unique<NFWPotential>(worldCenter, rho_s, r_s, G, eps);
        }

        default:
            return std::make_unique<NoPotential>();
    }
}
