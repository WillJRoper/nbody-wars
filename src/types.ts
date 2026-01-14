/**
 * Game mode determining player count and win conditions
 */
export enum GameMode {
  /** Single player - survive as long as possible */
  SOLO = 0,
  /** Two players cooperate against asteroids */
  COOP = 1,
  /** Two players compete for highest score */
  VERSUS = 2
}

/**
 * Ship rendering and state data
 * Transferred from C++ engine via WASM for each active ship
 */
export interface ShipData {
  x: number;
  y: number;
  angle: number;
  radius: number;
  active: boolean;
  invulnerable: boolean;
  thrusting: boolean;
  lives: number;
  score: number;
  playerId: number;
}

/**
 * Asteroid rendering and state data
 * Transferred from C++ engine for each active asteroid
 */
export interface AsteroidData {
  x: number;          // Position X
  y: number;          // Position Y
  radius: number;     // Collision and render radius
  rotation: number;   // Current rotation angle for visual variety
  size: number;       // Size class (0=large, 1=medium, 2=small)
  active: boolean;    // Whether asteroid is alive
}

/**
 * Bullet rendering data
 * Transferred from C++ engine for each active bullet
 */
export interface BulletData {
  x: number;          // Position X
  y: number;          // Position Y
  radius: number;     // Render radius
  playerId: number;   // Owner player (0 or 1) for color coding
}

/**
 * Black hole rendering data with danger zone visualization
 * Transferred from C++ engine for each active black hole
 */
export interface BlackHoleData {
  x: number;                // Center position X
  y: number;                // Center position Y
  accretionRadius: number;  // Danger zone radius (objects destroyed within this)
  visualRadius: number;     // Event horizon rendering size
}

/**
 * Explosion particle data with color coding
 * Particles fade over time and are color-coded by source
 */
export interface ParticleData {
  x: number;        // Position X
  y: number;        // Position Y
  alpha: number;    // Opacity (0-1) for fade effect
  playerId: number; // Color code: -1=white (asteroids), 0=green (player 1), 1=cyan (player 2)
}

/**
 * Player input state for one frame
 * Captured from keyboard/gamepad and sent to physics engine
 */
export interface InputState {
  left: boolean;    // Rotate counter-clockwise
  right: boolean;   // Rotate clockwise
  thrust: boolean;  // Apply forward thrust
  brake: boolean;   // Apply braking force
  shoot: boolean;   // Fire bullet
}

/**
 * Difficulty and balance configuration
 * All tunable gameplay parameters exposed via UI sliders
 */
export interface DifficultyConfig {
  bhSpawnRate: number;      // Black hole spawn probability per frame
  bhMassMult: number;       // Black hole mass multiplier
  bhAccRadius: number;      // Black hole accretion radius
  bhEnabled: boolean;       // Enable/disable black hole spawning
  shipMass: number;         // Ship mass for N-body gravity
  bulletMass: number;       // Bullet mass for N-body gravity
  asteroidMass: number;     // Large asteroid base mass
  asteroidCount: number;    // Asteroids spawned per wave
}

/**
 * Complete game configuration
 * Combines mode, level, and difficulty settings
 */
export interface GameConfig {
  mode: GameMode;               // Game mode (solo/coop/versus)
  level: number;                // Gravitational potential level (0-4)
  difficulty: DifficultyConfig; // Gameplay balance parameters
}

/**
 * High-level game state for UI management
 */
export enum GameState {
  LOADING,   // Initial asset/WASM loading
  MENU,      // Main menu / configuration screen
  PLAYING,   // Active gameplay
  PAUSED,    // Game paused
  GAME_OVER  // All players dead or game ended
}
