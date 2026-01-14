import type { InputState } from './types';

interface KeyMapping {
  left: string;
  right: string;
  thrust: string;
  brake: string;
  shoot: string;
}

interface GamepadMapping {
  left: number;
  right: number;
  thrust: number;
  brake: number;
  shoot: number;
}

export class InputManager {
  private keyStates: Map<string, boolean> = new Map();
  private inputStates: [InputState, InputState] = [
    { left: false, right: false, thrust: false, brake: false, shoot: false },
    { left: false, right: false, thrust: false, brake: false, shoot: false }
  ];

  private keyMappings: [KeyMapping, KeyMapping] = [
    { left: 'ArrowLeft', right: 'ArrowRight', thrust: 'ArrowUp', brake: 'ArrowDown', shoot: ' ' },
    { left: 'a', right: 'd', thrust: 'w', brake: 's', shoot: 'e' }
  ];

  private gamepadMappings: [GamepadMapping, GamepadMapping] = [
    { left: 14, right: 15, thrust: 12, brake: 13, shoot: 0 },
    { left: 14, right: 15, thrust: 12, brake: 13, shoot: 0 }
  ];

  constructor() {
    this.setupEventListeners();
    this.loadMappings();
  }

  private setupEventListeners(): void {
    window.addEventListener('keydown', (e) => {
      this.keyStates.set(e.key, true);
      this.updateInputStates();
    });

    window.addEventListener('keyup', (e) => {
      this.keyStates.set(e.key, false);
      this.updateInputStates();
    });

    // Prevent default browser actions for game keys
    window.addEventListener('keydown', (e) => {
      if ([' ', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'w', 'a', 's', 'd', 'e'].includes(e.key)) {
        e.preventDefault();
      }
    });
  }

  private updateInputStates(): void {
    for (let i = 0; i < 2; i++) {
      const mapping = this.keyMappings[i];
      this.inputStates[i] = {
        left: this.keyStates.get(mapping.left) || false,
        right: this.keyStates.get(mapping.right) || false,
        thrust: this.keyStates.get(mapping.thrust) || false,
        brake: this.keyStates.get(mapping.brake) || false,
        shoot: this.keyStates.get(mapping.shoot) || false
      };
    }
  }

  update(): void {
    // Update from keyboard
    this.updateInputStates();

    // Update from gamepads
    const gamepads = navigator.getGamepads();
    for (let i = 0; i < Math.min(gamepads.length, 2); i++) {
      const gamepad = gamepads[i];
      if (!gamepad) continue;

      const mapping = this.gamepadMappings[i];

      // Use buttons or axes
      const leftPressed = gamepad.buttons[mapping.left]?.pressed || gamepad.axes[0] < -0.5;
      const rightPressed = gamepad.buttons[mapping.right]?.pressed || gamepad.axes[0] > 0.5;
      const thrustPressed = gamepad.buttons[mapping.thrust]?.pressed || gamepad.axes[1] < -0.5;
      const shootPressed = gamepad.buttons[mapping.shoot]?.pressed;

      // Merge with keyboard input
      this.inputStates[i].left = this.inputStates[i].left || leftPressed;
      this.inputStates[i].right = this.inputStates[i].right || rightPressed;
      this.inputStates[i].thrust = this.inputStates[i].thrust || thrustPressed;
      this.inputStates[i].shoot = this.inputStates[i].shoot || shootPressed;
    }
  }

  getInputState(playerId: number): InputState {
    return this.inputStates[playerId] || { left: false, right: false, thrust: false, shoot: false };
  }

  isKeyPressed(key: string): boolean {
    return this.keyStates.get(key) || false;
  }

  // Gamepad mapping configuration
  setKeyMapping(playerId: number, action: keyof KeyMapping, key: string): void {
    if (playerId >= 0 && playerId < 2) {
      this.keyMappings[playerId][action] = key;
      this.saveMappings();
    }
  }

  setGamepadMapping(playerId: number, action: keyof GamepadMapping, button: number): void {
    if (playerId >= 0 && playerId < 2) {
      this.gamepadMappings[playerId][action] = button;
      this.saveMappings();
    }
  }

  private saveMappings(): void {
    localStorage.setItem('keyMappings', JSON.stringify(this.keyMappings));
    localStorage.setItem('gamepadMappings', JSON.stringify(this.gamepadMappings));
  }

  private loadMappings(): void {
    try {
      const keyMappings = localStorage.getItem('keyMappings');
      const gamepadMappings = localStorage.getItem('gamepadMappings');

      if (keyMappings) {
        this.keyMappings = JSON.parse(keyMappings);
      }
      if (gamepadMappings) {
        this.gamepadMappings = JSON.parse(gamepadMappings);
      }
    } catch (e) {
      console.warn('Failed to load input mappings:', e);
    }
  }

  reset(): void {
    this.keyStates.clear();
    this.inputStates = [
      { left: false, right: false, thrust: false, shoot: false },
      { left: false, right: false, thrust: false, shoot: false }
    ];
  }
}
