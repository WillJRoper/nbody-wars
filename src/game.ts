import { PhysicsEngine } from './physics';
import { Renderer } from './renderer';
import { InputManager } from './input';
import { AudioManager } from './audio';
import { UIManager } from './ui';
import { GameState, GameMode } from './types';
import type { GameConfig } from './types';

export class Game {
  private physics: PhysicsEngine;
  private renderer: Renderer;
  private input: InputManager;
  private audio: AudioManager;
  private ui: UIManager;

  private canvas: HTMLCanvasElement;
  private running: boolean = false;
  private lastTime: number = 0;
  private accumulator: number = 0;
  private readonly fixedDt: number = 1 / 120;

  private prevBulletCount: number = 0;
  private prevAsteroidCount: number = 0;
  private prevBlackHoleCount: number = 0;

  constructor(canvas: HTMLCanvasElement) {
    this.canvas = canvas;
    this.renderer = new Renderer(canvas);
    this.input = new InputManager();
    this.audio = new AudioManager();

    const defaultConfig: GameConfig = {
      mode: GameMode.SOLO,
      level: 0,
      difficulty: {
        bhSpawnRate: 0.0005,
        bhMassMult: 1.0,
        bhAccRadius: 25,
        bhEnabled: true,
        shipMass: 1500,
        bulletMass: 100,
        asteroidMass: 6000,
        asteroidCount: 4
      }
    };

    this.ui = new UIManager(defaultConfig);
    this.physics = new PhysicsEngine();

    this.setupUICallbacks();
    this.setupGameControls();
  }

  private setupUICallbacks(): void {
    this.ui.setCallbacks({
      onStartGame: () => this.startGame(),
      onResumeGame: () => this.resumeGame(),
      onQuitToMenu: () => this.quitToMenu(),
      onRestartGame: () => this.restartGame(),
      onBlackHolesToggle: (enabled: boolean) => this.physics.setBlackHolesEnabled(enabled)
    });
  }

  private setupGameControls(): void {
    window.addEventListener('keydown', (e) => {
      if (this.ui.getState() === GameState.PLAYING) {
        if (e.key === 'p' || e.key === 'P') {
          this.pauseGame();
        } else if (e.key === 'Escape') {
          this.pauseGame();
        }
      }
    });
  }

  async initialize(): Promise<void> {
    this.ui.setLoading(true);

    try {
      await this.physics.initialize(this.canvas.width, this.canvas.height);
      this.ui.setLoading(false);
    } catch (error) {
      console.error('Failed to initialize physics engine:', error);
      this.ui.setLoading(false);
      alert('Failed to load physics engine. Please refresh the page.');
    }
  }

  private startGame(): void {
    const config = this.ui.getConfig();

    this.physics.setMode(config.mode);
    this.physics.setLevel(config.level);
    this.physics.setDifficulty(config.difficulty);
    this.physics.reset();

    this.ui.setState(GameState.PLAYING);
    this.running = true;
    this.lastTime = performance.now();
    this.accumulator = 0;

    this.input.reset();

    this.gameLoop(this.lastTime);
  }

  private pauseGame(): void {
    this.running = false;
    this.ui.setState(GameState.PAUSED);

    const potentialName = this.physics.getPotentialName();
    const potentialDesc = this.physics.getPotentialDescription();
    this.ui.showPauseMenu(potentialName, potentialDesc);
  }

  private resumeGame(): void {
    this.ui.setState(GameState.PLAYING);
    this.running = true;
    this.lastTime = performance.now();
    this.accumulator = 0;
    this.gameLoop(this.lastTime);
  }

  private quitToMenu(): void {
    this.running = false;
    this.ui.setState(GameState.MENU);
  }

  private restartGame(): void {
    this.startGame();
  }

  private gameLoop(currentTime: number): void {
    if (!this.running) return;

    const deltaTime = Math.min((currentTime - this.lastTime) / 1000, 0.1);
    this.lastTime = currentTime;
    this.accumulator += deltaTime;

    // Update input
    this.input.update();

    // Fixed timestep physics updates
    while (this.accumulator >= this.fixedDt) {
      this.updatePhysics();
      this.accumulator -= this.fixedDt;
    }

    // Render
    this.render();

    // Check game over
    if (this.physics.isGameOver()) {
      this.running = false;
      this.ui.setState(GameState.GAME_OVER);
      this.ui.showGameOver(this.physics.getShips());
      return;
    }

    // Continue loop
    requestAnimationFrame((time) => this.gameLoop(time));
  }

  private updatePhysics(): void {
    // Send inputs to physics engine
    const config = this.ui.getConfig();
    const numPlayers = config.mode === GameMode.SOLO ? 1 : 2;

    for (let i = 0; i < numPlayers; i++) {
      const inputState = this.input.getInputState(i);
      this.physics.setInput(i, inputState);

      // Play thrust sound
      const ships = this.physics.getShips();
      if (ships[i] && ships[i].thrusting) {
        // Throttle thrust sound
        if (Math.random() < 0.1) {
          this.audio.playThrust();
        }
      }
    }

    // Step physics
    this.physics.step();

    // Play audio for new events
    this.playAudioForEvents();
  }

  private playAudioForEvents(): void {
    const bulletCount = this.physics.getBullets().length;
    const asteroidCount = this.physics.getAsteroids().length;
    const blackHoleCount = this.physics.getBlackHoles().length;

    // New bullet fired
    if (bulletCount > this.prevBulletCount) {
      this.audio.playShoot();
    }

    // Asteroid destroyed
    if (asteroidCount < this.prevAsteroidCount) {
      this.audio.playExplosion();
    }

    // New black hole spawned
    if (blackHoleCount > this.prevBlackHoleCount) {
      this.audio.playWarning();
    }

    this.prevBulletCount = bulletCount;
    this.prevAsteroidCount = asteroidCount;
    this.prevBlackHoleCount = blackHoleCount;
  }

  private render(): void {
    const ships = this.physics.getShips();
    const asteroids = this.physics.getAsteroids();
    const bullets = this.physics.getBullets();
    const blackHoles = this.physics.getBlackHoles();
    const particles = this.physics.getParticles();

    this.renderer.render(ships, asteroids, bullets, blackHoles, particles);

    // Update HUD
    const wave = this.physics.getWave();
    const potentialName = this.physics.getPotentialName();
    this.ui.updateHUD(ships, wave, potentialName);
  }

  destroy(): void {
    this.running = false;
    this.physics.destroy();
  }
}
