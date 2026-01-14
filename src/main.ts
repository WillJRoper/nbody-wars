import { Game } from './game';

async function main() {
  const canvas = document.getElementById('gameCanvas') as HTMLCanvasElement;

  if (!canvas) {
    console.error('Canvas element not found');
    return;
  }

  const game = new Game(canvas);

  try {
    await game.initialize();
    console.log('N-Body Wars initialized successfully');
  } catch (error) {
    console.error('Failed to initialize game:', error);
  }

  // Cleanup on page unload
  window.addEventListener('beforeunload', () => {
    game.destroy();
  });
}

main();
