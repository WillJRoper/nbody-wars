import type {
  ShipData,
  AsteroidData,
  BulletData,
  BlackHoleData,
  ParticleData
} from './types';

export class Renderer {
  private canvas: HTMLCanvasElement;
  private ctx: CanvasRenderingContext2D;
  private width: number;
  private height: number;

  constructor(canvas: HTMLCanvasElement) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d')!;
    this.width = canvas.width;
    this.height = canvas.height;
  }

  clear(): void {
    this.ctx.fillStyle = '#000';
    this.ctx.fillRect(0, 0, this.width, this.height);
  }

  drawShip(ship: ShipData): void {
    if (!ship.active) return;

    this.ctx.save();
    this.ctx.translate(ship.x, ship.y);
    this.ctx.rotate(ship.angle);

    // Flashing when invulnerable
    if (ship.invulnerable && Math.floor(Date.now() / 100) % 2 === 0) {
      this.ctx.restore();
      return;
    }

    // Ship body (triangle)
    this.ctx.strokeStyle = ship.playerId === 0 ? '#0f0' : '#0ff';
    this.ctx.lineWidth = 2;
    this.ctx.beginPath();
    this.ctx.moveTo(ship.radius, 0);
    this.ctx.lineTo(-ship.radius * 0.7, ship.radius * 0.7);
    this.ctx.lineTo(-ship.radius * 0.4, 0);
    this.ctx.lineTo(-ship.radius * 0.7, -ship.radius * 0.7);
    this.ctx.closePath();
    this.ctx.stroke();

    // Thrust flame
    if (ship.thrusting) {
      this.ctx.strokeStyle = '#f80';
      this.ctx.beginPath();
      this.ctx.moveTo(-ship.radius * 0.7, ship.radius * 0.5);
      this.ctx.lineTo(-ship.radius * 1.2, 0);
      this.ctx.lineTo(-ship.radius * 0.7, -ship.radius * 0.5);
      this.ctx.stroke();
    }

    this.ctx.restore();
  }

  drawAsteroid(asteroid: AsteroidData): void {
    if (!asteroid.active) return;

    this.ctx.save();
    this.ctx.translate(asteroid.x, asteroid.y);
    this.ctx.rotate(asteroid.rotation);

    this.ctx.strokeStyle = '#fff';
    this.ctx.lineWidth = 2;

    // Draw irregular polygon
    const vertices = 8;
    this.ctx.beginPath();
    for (let i = 0; i <= vertices; i++) {
      const angle = (i / vertices) * Math.PI * 2;
      const variation = 0.7 + (Math.sin(i * 2.5) * 0.3);
      const r = asteroid.radius * variation;
      const x = Math.cos(angle) * r;
      const y = Math.sin(angle) * r;

      if (i === 0) {
        this.ctx.moveTo(x, y);
      } else {
        this.ctx.lineTo(x, y);
      }
    }
    this.ctx.stroke();

    this.ctx.restore();
  }

  drawBullet(bullet: BulletData): void {
    this.ctx.fillStyle = bullet.playerId === 0 ? '#0f0' : '#0ff';
    this.ctx.beginPath();
    this.ctx.arc(bullet.x, bullet.y, bullet.radius, 0, Math.PI * 2);
    this.ctx.fill();
  }

  drawBlackHole(blackHole: BlackHoleData): void {
    const { x, y, accretionRadius, visualRadius } = blackHole;

    // Event horizon (filled circle)
    this.ctx.fillStyle = '#000';
    this.ctx.beginPath();
    this.ctx.arc(x, y, visualRadius, 0, Math.PI * 2);
    this.ctx.fill();

    // Accretion disk (gradient)
    const gradient = this.ctx.createRadialGradient(x, y, visualRadius, x, y, accretionRadius);
    gradient.addColorStop(0, 'rgba(255, 100, 0, 0.6)');
    gradient.addColorStop(0.5, 'rgba(255, 100, 0, 0.3)');
    gradient.addColorStop(1, 'rgba(255, 100, 0, 0)');

    this.ctx.fillStyle = gradient;
    this.ctx.beginPath();
    this.ctx.arc(x, y, accretionRadius, 0, Math.PI * 2);
    this.ctx.fill();

    // Rotating accretion effect
    const time = Date.now() * 0.001;
    this.ctx.strokeStyle = 'rgba(255, 150, 0, 0.5)';
    this.ctx.lineWidth = 2;
    for (let i = 0; i < 3; i++) {
      const angle = time + (i * Math.PI * 2 / 3);
      const r1 = visualRadius * 1.5;
      const r2 = accretionRadius * 0.8;
      this.ctx.beginPath();
      this.ctx.arc(x, y, r1 + i * 5, angle, angle + Math.PI * 0.3);
      this.ctx.stroke();
    }

    // Warning circle
    this.ctx.strokeStyle = 'rgba(255, 0, 0, 0.5)';
    this.ctx.lineWidth = 1;
    this.ctx.setLineDash([5, 5]);
    this.ctx.beginPath();
    this.ctx.arc(x, y, accretionRadius, 0, Math.PI * 2);
    this.ctx.stroke();
    this.ctx.setLineDash([]);
  }

  drawParticle(particle: ParticleData): void {
    // Color based on player ID
    let color: string;
    if (particle.playerId === 0) {
      // Player 1 (green)
      color = `rgba(0, 255, 0, ${particle.alpha})`;
    } else if (particle.playerId === 1) {
      // Player 2 (cyan)
      color = `rgba(0, 255, 255, ${particle.alpha})`;
    } else {
      // Default white (asteroids, etc)
      color = `rgba(255, 255, 255, ${particle.alpha})`;
    }

    this.ctx.fillStyle = color;
    this.ctx.fillRect(particle.x - 1, particle.y - 1, 2, 2);
  }

  drawCenterOfMass(x: number, y: number): void {
    this.ctx.strokeStyle = 'rgba(255, 255, 0, 0.3)';
    this.ctx.lineWidth = 1;
    this.ctx.beginPath();
    this.ctx.arc(x, y, 10, 0, Math.PI * 2);
    this.ctx.stroke();

    this.ctx.strokeStyle = 'rgba(255, 255, 0, 0.5)';
    this.ctx.beginPath();
    this.ctx.moveTo(x - 5, y);
    this.ctx.lineTo(x + 5, y);
    this.ctx.moveTo(x, y - 5);
    this.ctx.lineTo(x, y + 5);
    this.ctx.stroke();
  }

  render(
    ships: ShipData[],
    asteroids: AsteroidData[],
    bullets: BulletData[],
    blackHoles: BlackHoleData[],
    particles: ParticleData[]
  ): void {
    this.clear();

    // Draw center marker for potentials
    this.drawCenterOfMass(this.width / 2, this.height / 2);

    // Draw particles (background)
    particles.forEach(p => this.drawParticle(p));

    // Draw black holes
    blackHoles.forEach(bh => this.drawBlackHole(bh));

    // Draw asteroids
    asteroids.forEach(a => this.drawAsteroid(a));

    // Draw bullets
    bullets.forEach(b => this.drawBullet(b));

    // Draw ships
    ships.forEach(s => this.drawShip(s));
  }
}
