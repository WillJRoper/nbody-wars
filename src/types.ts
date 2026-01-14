export enum GameMode {
  SOLO = 0,
  COOP = 1,
  VERSUS = 2
}

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

export interface AsteroidData {
  x: number;
  y: number;
  radius: number;
  rotation: number;
  size: number;
  active: boolean;
}

export interface BulletData {
  x: number;
  y: number;
  radius: number;
  playerId: number;
}

export interface BlackHoleData {
  x: number;
  y: number;
  accretionRadius: number;
  visualRadius: number;
}

export interface ParticleData {
  x: number;
  y: number;
  alpha: number;
  playerId: number;  // -1 for white (default), 0/1 for ship colors
}

export interface InputState {
  left: boolean;
  right: boolean;
  thrust: boolean;
  brake: boolean;
  shoot: boolean;
}

export interface DifficultyConfig {
  bhSpawnRate: number;
  bhMassMult: number;
  bhAccRadius: number;
  bhEnabled: boolean;
  shipMass: number;
  bulletMass: number;
  asteroidMass: number;
  asteroidCount: number;
}

export interface GameConfig {
  mode: GameMode;
  level: number;
  difficulty: DifficultyConfig;
}

export enum GameState {
  LOADING,
  MENU,
  PLAYING,
  PAUSED,
  GAME_OVER
}
