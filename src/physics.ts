import type {
  ShipData,
  AsteroidData,
  BulletData,
  BlackHoleData,
  ParticleData,
  InputState,
  DifficultyConfig,
  GameMode
} from './types';

interface PhysicsModule extends EmscriptenModule {
  _engine_create: (width: number, height: number, seed: number) => number;
  _engine_destroy: (handle: number) => void;
  _engine_set_mode: (handle: number, mode: number) => void;
  _engine_set_level: (handle: number, levelId: number) => void;
  _engine_set_difficulty: (handle: number, bhSpawnRate: number, bhMassMult: number, bhAccRadius: number) => void;
  _engine_set_input: (handle: number, playerId: number, left: number, right: number, thrust: number, shoot: number) => void;
  _engine_step: (handle: number) => void;
  _engine_reset: (handle: number) => void;
  _engine_is_game_over: (handle: number) => number;
  _engine_get_time: (handle: number) => number;
  _engine_get_wave: (handle: number) => number;
  _engine_get_ship_count: (handle: number) => number;
  _engine_get_ship_data: (handle: number, index: number, outData: number) => void;
  _engine_get_asteroid_count: (handle: number) => number;
  _engine_get_asteroid_data: (handle: number, index: number, outData: number) => void;
  _engine_get_bullet_count: (handle: number) => number;
  _engine_get_bullet_data: (handle: number, index: number, outData: number) => void;
  _engine_get_blackhole_count: (handle: number) => number;
  _engine_get_blackhole_data: (handle: number, index: number, outData: number) => void;
  _engine_get_particle_count: (handle: number) => number;
  _engine_get_particle_data: (handle: number, index: number, outData: number) => void;
  _engine_get_potential_name: (handle: number) => number;
  _engine_get_potential_description: (handle: number) => number;
}

export class PhysicsEngine {
  private module: any = null;
  private handle: number = 0;
  private tempBuffer: Float32Array;
  private tempPtr: number = 0;

  constructor() {
    this.tempBuffer = new Float32Array(16);
  }

  async initialize(width: number, height: number, seed: number = Date.now()): Promise<void> {
    // Load the WASM module dynamically at runtime
    const script = document.createElement('script');
    script.src = '/physics.js';
    document.head.appendChild(script);

    // Wait for the script to load
    await new Promise((resolve, reject) => {
      script.onload = resolve;
      script.onerror = reject;
    });

    // @ts-ignore - createPhysicsModule is loaded from the script
    const createPhysicsModule = (window as any).createPhysicsModule;
    if (!createPhysicsModule) {
      throw new Error('Physics module not loaded');
    }
    this.module = await createPhysicsModule();

    // Allocate temp buffer
    this.tempPtr = this.module._malloc(this.tempBuffer.length * 4);
    this.handle = this.module._engine_create(width, height, seed);

    if (this.handle === 0) {
      throw new Error('Failed to create physics engine (handle is 0)');
    }
  }

  destroy(): void {
    if (this.module && this.handle) {
      this.module._engine_destroy(this.handle);
      if (this.tempPtr) {
        this.module._free(this.tempPtr);
      }
    }
  }

  setMode(mode: GameMode): void {
    if (this.module && this.handle) {
      this.module._engine_set_mode(this.handle, mode);
    }
  }

  setLevel(levelId: number): void {
    if (this.module && this.handle) {
      this.module._engine_set_level(this.handle, levelId);
    }
  }

  setDifficulty(config: DifficultyConfig): void {
    if (this.module && this.handle) {
      this.module._engine_set_difficulty(
        this.handle,
        config.bhSpawnRate,
        config.bhMassMult,
        config.bhAccRadius,
        config.bhEnabled ? 1 : 0,
        config.shipMass,
        config.bulletMass,
        config.asteroidMass,
        config.asteroidCount
      );
    }
  }

  setBlackHolesEnabled(enabled: boolean): void {
    if (this.module && this.handle) {
      this.module._engine_set_blackholes_enabled(this.handle, enabled ? 1 : 0);
    }
  }

  setShipMass(mass: number): void {
    if (this.module && this.handle) {
      this.module._engine_set_ship_mass(this.handle, mass);
    }
  }

  setBulletMass(mass: number): void {
    if (this.module && this.handle) {
      this.module._engine_set_bullet_mass(this.handle, mass);
    }
  }

  setAsteroidBaseMass(mass: number): void {
    if (this.module && this.handle) {
      this.module._engine_set_asteroid_base_mass(this.handle, mass);
    }
  }

  setInput(playerId: number, input: InputState): void {
    if (this.module && this.handle) {
      this.module._engine_set_input(
        this.handle,
        playerId,
        input.left ? 1 : 0,
        input.right ? 1 : 0,
        input.thrust ? 1 : 0,
        input.brake ? 1 : 0,
        input.shoot ? 1 : 0
      );
    }
  }

  step(): void {
    if (this.module && this.handle) {
      this.module._engine_step(this.handle);
    }
  }

  reset(): void {
    if (this.module && this.handle) {
      this.module._engine_reset(this.handle);
    }
  }

  isGameOver(): boolean {
    if (this.module && this.handle) {
      return this.module._engine_is_game_over(this.handle) !== 0;
    }
    return false;
  }

  getTime(): number {
    if (this.module && this.handle) {
      return this.module._engine_get_time(this.handle);
    }
    return 0;
  }

  getWave(): number {
    if (this.module && this.handle) {
      return this.module._engine_get_wave(this.handle);
    }
    return 1;
  }

  getShips(): ShipData[] {
    if (!this.module || !this.handle) return [];

    const count = this.module._engine_get_ship_count(this.handle);
    const ships: ShipData[] = [];

    for (let i = 0; i < count; i++) {
      this.module._engine_get_ship_data(this.handle, i, this.tempPtr);

      // Access WASM memory as Float32Array
      const heap = new Float32Array(this.module.HEAP8.buffer, this.tempPtr, 10);
      for (let j = 0; j < 10; j++) {
        this.tempBuffer[j] = heap[j];
      }

      ships.push({
        x: this.tempBuffer[0],
        y: this.tempBuffer[1],
        angle: this.tempBuffer[2],
        radius: this.tempBuffer[3],
        active: this.tempBuffer[4] !== 0,
        invulnerable: this.tempBuffer[5] !== 0,
        thrusting: this.tempBuffer[6] !== 0,
        lives: this.tempBuffer[7],
        score: this.tempBuffer[8],
        playerId: this.tempBuffer[9]
      });
    }

    return ships;
  }

  getAsteroids(): AsteroidData[] {
    if (!this.module || !this.handle) return [];

    const count = this.module._engine_get_asteroid_count(this.handle);
    const asteroids: AsteroidData[] = [];

    for (let i = 0; i < count; i++) {
      this.module._engine_get_asteroid_data(this.handle, i, this.tempPtr);

      const heap = new Float32Array(this.module.HEAP8.buffer, this.tempPtr, 6);
      for (let j = 0; j < 6; j++) {
        this.tempBuffer[j] = heap[j];
      }

      asteroids.push({
        x: this.tempBuffer[0],
        y: this.tempBuffer[1],
        radius: this.tempBuffer[2],
        rotation: this.tempBuffer[3],
        size: this.tempBuffer[4],
        active: this.tempBuffer[5] !== 0
      });
    }

    return asteroids;
  }

  getBullets(): BulletData[] {
    if (!this.module || !this.handle) return [];

    const count = this.module._engine_get_bullet_count(this.handle);
    const bullets: BulletData[] = [];

    for (let i = 0; i < count; i++) {
      this.module._engine_get_bullet_data(this.handle, i, this.tempPtr);

      const heap = new Float32Array(this.module.HEAP8.buffer, this.tempPtr, 4);
      for (let j = 0; j < 4; j++) {
        this.tempBuffer[j] = heap[j];
      }

      bullets.push({
        x: this.tempBuffer[0],
        y: this.tempBuffer[1],
        radius: this.tempBuffer[2],
        playerId: this.tempBuffer[3]
      });
    }

    return bullets;
  }

  getBlackHoles(): BlackHoleData[] {
    if (!this.module || !this.handle) return [];

    const count = this.module._engine_get_blackhole_count(this.handle);
    const blackHoles: BlackHoleData[] = [];

    for (let i = 0; i < count; i++) {
      this.module._engine_get_blackhole_data(this.handle, i, this.tempPtr);

      const heap = new Float32Array(this.module.HEAP8.buffer, this.tempPtr, 4);
      for (let j = 0; j < 4; j++) {
        this.tempBuffer[j] = heap[j];
      }

      blackHoles.push({
        x: this.tempBuffer[0],
        y: this.tempBuffer[1],
        accretionRadius: this.tempBuffer[2],
        visualRadius: this.tempBuffer[3]
      });
    }

    return blackHoles;
  }

  getParticles(): ParticleData[] {
    if (!this.module || !this.handle) return [];

    const count = this.module._engine_get_particle_count(this.handle);
    const particles: ParticleData[] = [];

    for (let i = 0; i < count; i++) {
      this.module._engine_get_particle_data(this.handle, i, this.tempPtr);

      const heap = new Float32Array(this.module.HEAP8.buffer, this.tempPtr, 3);
      for (let j = 0; j < 3; j++) {
        this.tempBuffer[j] = heap[j];
      }

      particles.push({
        x: this.tempBuffer[0],
        y: this.tempBuffer[1],
        alpha: this.tempBuffer[2]
      });
    }

    return particles;
  }

  getPotentialName(): string {
    if (!this.module || !this.handle) return '';
    const ptr = this.module._engine_get_potential_name(this.handle);
    return this.module.UTF8ToString(ptr);
  }

  getPotentialDescription(): string {
    if (!this.module || !this.handle) return '';
    const ptr = this.module._engine_get_potential_description(this.handle);
    return this.module.UTF8ToString(ptr);
  }
}
