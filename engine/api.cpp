#include "engine.h"
#include <emscripten/emscripten.h>
#include <cstring>

// C API for WASM
extern "C" {

// Engine lifecycle
EMSCRIPTEN_KEEPALIVE
void* engine_create(float width, float height, uint32_t seed) {
    return new GameEngine(width, height, seed);
}

EMSCRIPTEN_KEEPALIVE
void engine_destroy(void* handle) {
    delete static_cast<GameEngine*>(handle);
}

// Configuration
EMSCRIPTEN_KEEPALIVE
void engine_set_mode(void* handle, int mode) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setMode(static_cast<GameMode>(mode));
}

EMSCRIPTEN_KEEPALIVE
void engine_set_level(void* handle, int levelId) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setLevel(levelId);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_difficulty(void* handle, float bhSpawnRate, float bhMassMult, float bhAccRadius, int bhEnabled, float shipMass, float bulletMass, float asteroidBaseMass, int asteroidCount) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    DifficultyConfig config;
    config.bhSpawnRate = bhSpawnRate;
    config.bhMassMult = bhMassMult;
    config.bhAccRadius = bhAccRadius;
    config.bhEnabled = bhEnabled != 0;
    config.shipMass = shipMass;
    config.bulletMass = bulletMass;
    config.asteroidBaseMass = asteroidBaseMass;
    config.asteroidCount = asteroidCount;
    engine->setDifficulty(config);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_blackholes_enabled(void* handle, int enabled) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setBlackHolesEnabled(enabled != 0);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_ship_mass(void* handle, float mass) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setShipMass(mass);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_bullet_mass(void* handle, float mass) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setBulletMass(mass);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_asteroid_base_mass(void* handle, float mass) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->setAsteroidBaseMass(mass);
}

EMSCRIPTEN_KEEPALIVE
void engine_set_input(void* handle, int playerId, int left, int right, int thrust, int brake, int shoot) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    InputState input;
    input.left = left != 0;
    input.right = right != 0;
    input.thrust = thrust != 0;
    input.brake = brake != 0;
    input.shoot = shoot != 0;
    engine->setInput(playerId, input);
}

// Simulation
EMSCRIPTEN_KEEPALIVE
void engine_step(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->step();
}

EMSCRIPTEN_KEEPALIVE
void engine_reset(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    engine->reset();
}

// Query state
EMSCRIPTEN_KEEPALIVE
int engine_is_game_over(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->isGameOver() ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
float engine_get_time(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getTime();
}

EMSCRIPTEN_KEEPALIVE
int engine_get_wave(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getWave();
}

// Get render data
EMSCRIPTEN_KEEPALIVE
int engine_get_ship_count(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getShips().size();
}

EMSCRIPTEN_KEEPALIVE
void engine_get_ship_data(void* handle, int index, float* outData) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const auto& ships = engine->getShips();
    if (index < 0 || index >= (int)ships.size()) return;

    const Ship& ship = ships[index];
    outData[0] = ship.pos.x;
    outData[1] = ship.pos.y;
    outData[2] = ship.angle;
    outData[3] = ship.radius;
    outData[4] = ship.active ? 1.0f : 0.0f;
    outData[5] = ship.invulnerable ? 1.0f : 0.0f;
    outData[6] = ship.thrusting ? 1.0f : 0.0f;
    outData[7] = ship.lives;
    outData[8] = ship.score;
    outData[9] = ship.playerId;
}

EMSCRIPTEN_KEEPALIVE
int engine_get_asteroid_count(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getAsteroids().size();
}

EMSCRIPTEN_KEEPALIVE
void engine_get_asteroid_data(void* handle, int index, float* outData) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const auto& asteroids = engine->getAsteroids();
    if (index < 0 || index >= (int)asteroids.size()) return;

    const Asteroid& asteroid = asteroids[index];
    outData[0] = asteroid.pos.x;
    outData[1] = asteroid.pos.y;
    outData[2] = asteroid.radius;
    outData[3] = asteroid.rotation;
    outData[4] = asteroid.size;
    outData[5] = asteroid.active ? 1.0f : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
int engine_get_bullet_count(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getBullets().size();
}

EMSCRIPTEN_KEEPALIVE
void engine_get_bullet_data(void* handle, int index, float* outData) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const auto& bullets = engine->getBullets();
    if (index < 0 || index >= (int)bullets.size()) return;

    const Bullet& bullet = bullets[index];
    outData[0] = bullet.pos.x;
    outData[1] = bullet.pos.y;
    outData[2] = bullet.radius;
    outData[3] = bullet.playerId;
}

EMSCRIPTEN_KEEPALIVE
int engine_get_blackhole_count(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getBlackHoles().size();
}

EMSCRIPTEN_KEEPALIVE
void engine_get_blackhole_data(void* handle, int index, float* outData) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const auto& blackHoles = engine->getBlackHoles();
    if (index < 0 || index >= (int)blackHoles.size()) return;

    const BlackHole& bh = blackHoles[index];
    outData[0] = bh.pos.x;
    outData[1] = bh.pos.y;
    outData[2] = bh.accretionRadius;
    outData[3] = bh.visualRadius;
}

EMSCRIPTEN_KEEPALIVE
int engine_get_particle_count(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    return engine->getParticles().size();
}

EMSCRIPTEN_KEEPALIVE
void engine_get_particle_data(void* handle, int index, float* outData) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const auto& particles = engine->getParticles();
    if (index < 0 || index >= (int)particles.size()) return;

    const Particle& particle = particles[index];
    outData[0] = particle.pos.x;
    outData[1] = particle.pos.y;
    outData[2] = particle.lifetime / particle.maxLifetime;  // Alpha
}

EMSCRIPTEN_KEEPALIVE
const char* engine_get_potential_name(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const IExternalPotential* potential = engine->getPotential();
    return potential ? potential->getName() : "Unknown";
}

EMSCRIPTEN_KEEPALIVE
const char* engine_get_potential_description(void* handle) {
    GameEngine* engine = static_cast<GameEngine*>(handle);
    const IExternalPotential* potential = engine->getPotential();
    return potential ? potential->getDescription() : "";
}

} // extern "C"
