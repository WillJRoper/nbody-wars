import type { GameConfig, GameMode, ShipData } from './types';
import { GameState } from './types';

export class UIManager {
  private currentState: GameState = GameState.LOADING;
  private config: GameConfig;

  // Menu elements
  private mainMenu: HTMLElement;
  private modeMenu: HTMLElement;
  private levelMenu: HTMLElement;
  private controlsMenu: HTMLElement;
  private difficultyMenu: HTMLElement;
  private aboutMenu: HTMLElement;
  private pauseMenu: HTMLElement;
  private gameOverMenu: HTMLElement;
  private loading: HTMLElement;

  // HUD elements
  private player1Info: HTMLElement;
  private player2Info: HTMLElement;
  private levelInfo: HTMLElement;

  // Callbacks
  private onStartGame?: () => void;
  private onResumeGame?: () => void;
  private onQuitToMenu?: () => void;
  private onRestartGame?: () => void;
  private onBlackHolesToggle?: (enabled: boolean) => void;

  constructor(config: GameConfig) {
    this.config = config;

    // Get menu elements
    this.mainMenu = document.getElementById('mainMenu')!;
    this.modeMenu = document.getElementById('modeMenu')!;
    this.levelMenu = document.getElementById('levelMenu')!;
    this.controlsMenu = document.getElementById('controlsMenu')!;
    this.difficultyMenu = document.getElementById('difficultyMenu')!;
    this.aboutMenu = document.getElementById('aboutMenu')!;
    this.pauseMenu = document.getElementById('pauseMenu')!;
    this.gameOverMenu = document.getElementById('gameOverMenu')!;
    this.loading = document.getElementById('loading')!;

    // Get HUD elements
    this.player1Info = document.getElementById('player1Info')!;
    this.player2Info = document.getElementById('player2Info')!;
    this.levelInfo = document.getElementById('levelInfo')!;

    this.setupEventListeners();
  }

  private setupEventListeners(): void {
    // Main menu
    document.getElementById('startGame')?.addEventListener('click', () => {
      this.showMenu(this.levelMenu);
    });

    document.getElementById('showModeMenu')?.addEventListener('click', () => {
      this.showMenu(this.modeMenu);
    });

    document.getElementById('showLevelMenu')?.addEventListener('click', () => {
      this.showMenu(this.levelMenu);
    });

    document.getElementById('showControlsMenu')?.addEventListener('click', () => {
      this.showMenu(this.controlsMenu);
    });

    document.getElementById('showDifficultyMenu')?.addEventListener('click', () => {
      this.showMenu(this.difficultyMenu);
    });

    document.getElementById('showAboutMenu')?.addEventListener('click', () => {
      this.showMenu(this.aboutMenu);
    });

    // Mode menu
    this.modeMenu.querySelectorAll('[data-mode]').forEach(btn => {
      btn.addEventListener('click', () => {
        const mode = btn.getAttribute('data-mode') as string;
        this.config.mode = { 'solo': 0, 'coop': 1, 'versus': 2 }[mode] as GameMode;
        this.showMenu(this.mainMenu);
      });
    });

    document.getElementById('backFromMode')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
    });

    // Level menu
    this.levelMenu.querySelectorAll('[data-level]').forEach(btn => {
      btn.addEventListener('click', () => {
        const level = parseInt(btn.getAttribute('data-level') || '0');
        this.config.level = level;
        this.hideAllMenus();
        this.onStartGame?.();
      });
    });

    document.getElementById('backFromLevel')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
    });

    // Controls menu
    document.getElementById('backFromControls')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
    });

    // Difficulty menu
    const asteroidCount = document.getElementById('asteroidCount') as HTMLInputElement;
    const asteroidCountValue = document.getElementById('asteroidCountValue')!;
    asteroidCount?.addEventListener('input', () => {
      this.config.difficulty.asteroidCount = parseInt(asteroidCount.value);
      asteroidCountValue.textContent = asteroidCount.value;
    });

    const shipMass = document.getElementById('shipMass') as HTMLInputElement;
    const shipMassValue = document.getElementById('shipMassValue')!;
    shipMass?.addEventListener('input', () => {
      this.config.difficulty.shipMass = parseFloat(shipMass.value);
      shipMassValue.textContent = shipMass.value;
    });

    const bulletMass = document.getElementById('bulletMass') as HTMLInputElement;
    const bulletMassValue = document.getElementById('bulletMassValue')!;
    bulletMass?.addEventListener('input', () => {
      this.config.difficulty.bulletMass = parseFloat(bulletMass.value);
      bulletMassValue.textContent = bulletMass.value;
    });

    const asteroidMass = document.getElementById('asteroidMass') as HTMLInputElement;
    const asteroidMassValue = document.getElementById('asteroidMassValue')!;
    asteroidMass?.addEventListener('input', () => {
      this.config.difficulty.asteroidMass = parseFloat(asteroidMass.value);
      asteroidMassValue.textContent = asteroidMass.value;
    });

    const bhSpawnRate = document.getElementById('bhSpawnRate') as HTMLInputElement;
    const bhSpawnRateValue = document.getElementById('bhSpawnRateValue')!;
    bhSpawnRate?.addEventListener('input', () => {
      this.config.difficulty.bhSpawnRate = parseFloat(bhSpawnRate.value);
      bhSpawnRateValue.textContent = bhSpawnRate.value;
    });

    const bhMassMult = document.getElementById('bhMassMult') as HTMLInputElement;
    const bhMassMultValue = document.getElementById('bhMassMultValue')!;
    bhMassMult?.addEventListener('input', () => {
      this.config.difficulty.bhMassMult = parseFloat(bhMassMult.value);
      bhMassMultValue.textContent = bhMassMult.value;
    });

    const bhAccRadius = document.getElementById('bhAccRadius') as HTMLInputElement;
    const bhAccRadiusValue = document.getElementById('bhAccRadiusValue')!;
    bhAccRadius?.addEventListener('input', () => {
      this.config.difficulty.bhAccRadius = parseFloat(bhAccRadius.value);
      bhAccRadiusValue.textContent = bhAccRadius.value;
    });

    document.getElementById('backFromDifficulty')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
    });

    // About menu
    document.getElementById('backFromAbout')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
    });

    // Pause menu
    document.getElementById('resumeGame')?.addEventListener('click', () => {
      this.hideAllMenus();
      this.onResumeGame?.();
    });

    document.getElementById('quitToMenu')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
      this.onQuitToMenu?.();
    });

    // Black holes toggle
    document.getElementById('blackHolesToggle')?.addEventListener('change', (e) => {
      const enabled = (e.target as HTMLInputElement).checked;
      this.onBlackHolesToggle?.(enabled);
    });

    // Game over menu
    document.getElementById('restartGame')?.addEventListener('click', () => {
      this.hideAllMenus();
      this.onRestartGame?.();
    });

    document.getElementById('backToMenu')?.addEventListener('click', () => {
      this.showMenu(this.mainMenu);
      this.onQuitToMenu?.();
    });
  }

  private showMenu(menu: HTMLElement): void {
    this.hideAllMenus();
    menu.classList.add('active');
  }

  private hideAllMenus(): void {
    [this.mainMenu, this.modeMenu, this.levelMenu, this.controlsMenu,
     this.difficultyMenu, this.aboutMenu, this.pauseMenu, this.gameOverMenu, this.loading]
      .forEach(menu => menu.classList.remove('active'));
  }

  setLoading(loading: boolean): void {
    if (loading) {
      this.hideAllMenus();
      this.loading.classList.add('active');
      this.currentState = GameState.LOADING;
    } else {
      this.loading.classList.remove('active');
      this.showMenu(this.mainMenu);
      this.currentState = GameState.MENU;
    }
  }

  showPauseMenu(potentialName: string, potentialDesc: string): void {
    const info = document.getElementById('pausePhysicsInfo')!;
    info.innerHTML = `
      <strong>Current Level:</strong> ${potentialName}<br>
      ${potentialDesc}<br><br>
      <strong>Physics Settings:</strong><br>
      • Barnes-Hut theta: 0.5<br>
      • Softening length: 5 units<br>
      • Fixed timestep: 1/120 second<br>
      • Gravitational constant: 100<br>
    `;
    this.showMenu(this.pauseMenu);
    this.currentState = GameState.PAUSED;
  }

  showGameOver(ships: ShipData[]): void {
    const stats = document.getElementById('gameOverStats')!;
    let html = '<strong>Final Scores:</strong><br><br>';

    ships.forEach((ship, i) => {
      html += `Player ${i + 1}: ${ship.score} points<br>`;
    });

    stats.innerHTML = html;
    this.showMenu(this.gameOverMenu);
    this.currentState = GameState.GAME_OVER;
  }

  updateHUD(ships: ShipData[], wave: number, potentialName: string): void {
    if (ships.length > 0) {
      this.player1Info.textContent = `P1: Lives: ${ships[0].lives} | Score: ${ships[0].score}`;
    }

    if (ships.length > 1) {
      this.player2Info.textContent = `P2: Lives: ${ships[1].lives} | Score: ${ships[1].score}`;
    } else {
      this.player2Info.textContent = '';
    }

    this.levelInfo.textContent = `Level: ${potentialName} | Wave: ${wave}`;
  }

  getConfig(): GameConfig {
    return this.config;
  }

  getState(): GameState {
    return this.currentState;
  }

  setState(state: GameState): void {
    this.currentState = state;
  }

  setCallbacks(callbacks: {
    onStartGame: () => void;
    onResumeGame: () => void;
    onQuitToMenu: () => void;
    onRestartGame: () => void;
    onBlackHolesToggle: (enabled: boolean) => void;
  }): void {
    this.onStartGame = callbacks.onStartGame;
    this.onResumeGame = callbacks.onResumeGame;
    this.onQuitToMenu = callbacks.onQuitToMenu;
    this.onRestartGame = callbacks.onRestartGame;
    this.onBlackHolesToggle = callbacks.onBlackHolesToggle;
  }
}
