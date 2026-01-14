# N-Body Wars

An educational Asteroids-like game with real gravitational N-body physics, powered by WebAssembly.

![N-Body Wars](https://img.shields.io/badge/physics-N--body-blue) ![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?logo=webassembly&logoColor=white) ![TypeScript](https://img.shields.io/badge/TypeScript-3178C6?logo=typescript&logoColor=white)
<img width="1728" height="1117" alt="SCR-20260114-kyae" src="https://github.com/user-attachments/assets/9a27a38c-3cce-4181-9e58-94c28d1c27bc" />

## Features

- **Real N-Body Gravity**: Every object (ships, asteroids, bullets, black holes) affects every other object gravitationally
- **Barnes-Hut Optimization**: Efficient O(N log N) gravity calculations using quadtree acceleration, scales to 1000+ bodies
- **5 Gravitational Potentials**: Choose from different gravitational environments:
  - The Void (no external forces)
  - Central Black Hole (Kepler orbits)
  - Harmonic Well (oscillatory motion)
  - Galaxy Disk (flat rotation curves)
  - Dark Matter Halo (NFW profile)
- **Dynamic Black Holes**: Randomly spawning black holes with accretion physics
- **3 Game Modes**: Solo, Co-op, and Versus
- **Deterministic Physics**: Fixed-timestep leapfrog integration for reproducible simulations
- **Vector Graphics**: Classic Asteroids-style line art rendering
- **WebAssembly Engine**: High-performance C++ physics compiled to WASM
- **Gamepad Support**: Full controller support with remapping
- **Configurable Difficulty**: Adjust masses, asteroid count, and black hole parameters in real-time

## Physics Concepts

This game demonstrates several key concepts in computational astrophysics:

### N-Body Gravity
Every body experiences gravitational acceleration from every other body:
```
a = Σ G * m_i * r_i / |r_i|^3
```
With softening to avoid singularities: `|r|^3 → (r^2 + ε^2)^(3/2)`

### Barnes-Hut Algorithm
Instead of computing O(N²) pairwise forces, we use a quadtree to group distant bodies:
- Divide space recursively into quadrants
- For distant groups, treat as single body at center of mass
- Opening criterion: `s/d < θ` where s is cell size, d is distance, θ ≈ 0.5

### External Potentials
Different gravitational environments create different orbital dynamics:
- **Point Mass**: `a(r) = -GM r / (r² + ε²)^(3/2)` - Classic Keplerian orbits
- **Harmonic**: `a(r) = -ω² r` - Oscillator dynamics
- **Logarithmic**: `V(r) = v₀² ln(r² + r_c²)` - Flat rotation curves like spiral galaxies
- **NFW**: `ρ(r) ∝ 1/(r(1+r/r_s)²)` - Dark matter halo profile

### Collision Physics
- **Elastic Collisions**: Asteroids bounce off each other with mass-dependent response (e=1.0)
- **Asteroid Splitting**: Bullets break asteroids into exactly 2 smaller pieces, with mass halving at each level
- **Periodic Boundaries**: Torus topology with minimum-image convention
- **Black Hole Accretion**: Objects within accretion radius are consumed

### Integration
Leapfrog (Kick-Drift-Kick) integrator:
```
v(t+dt/2) = v(t) + a(t) * dt/2
x(t+dt) = x(t) + v(t+dt/2) * dt
v(t+dt) = v(t+dt/2) + a(t+dt) * dt/2
```
Fixed timestep: dt = 1/120 second

## Local Setup

### Prerequisites

- **Node.js** (v18 or later)
- **Emscripten** (latest version)
  ```bash
  # Install Emscripten
  git clone https://github.com/emscripten-core/emsdk.git
  cd emsdk
  ./emsdk install latest
  ./emsdk activate latest
  source ./emsdk_env.sh
  ```

### Build Instructions

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd nbody-wars
   ```

2. **Install dependencies**
   ```bash
   npm install
   ```

3. **Build the project**
   ```bash
   npm run build
   ```
   This will:
   - Compile C++ engine to WebAssembly (`npm run build:wasm`)
   - Build TypeScript frontend (`npm run build:ts`)
   - Output everything to `dist/` folder

4. **Run development server**
   ```bash
   npm run dev
   ```
   Open http://localhost:3000

5. **Preview production build**
   ```bash
   npm run preview
   ```

## How to Play

### Getting Started

1. Click **START GAME** from the main menu
2. Choose your gravitational potential (The Void, Central Black Hole, etc.)
3. The game begins immediately!
4. Optionally adjust difficulty settings before playing
5. Change game mode for Co-op or Versus play

### Controls

**Player 1 (Green)**
- **Left/Right Arrow**: Rotate
- **Up Arrow**: Thrust
- **Down Arrow**: Brake
- **Space**: Shoot

**Player 2 (Cyan, in Co-op/Versus)**
- **A/D**: Rotate
- **W**: Thrust
- **S**: Brake
- **E**: Shoot

**General**
- **P**: Pause
- **ESC**: Menu

### Game Modes

- **Solo**: Single player, survive waves of asteroids
- **Co-op**: Two players work together, shared score
- **Versus**: Two players compete for score

### Difficulty Settings

Customize the physics to your liking:
- **Asteroid Count**: Initial number of asteroids per wave (1-20, default 4)
- **Ship Mass**: Affects gravitational pull and inertia (100-10,000, default 1,500)
- **Bullet Mass**: Bullets participate in N-body gravity (10-500, default 100)
- **Asteroid Mass**: Base mass for large asteroids; splits halve the mass (1,000-20,000, default 6,000)
- **Black Hole Parameters**: Spawn rate, mass multiplier, and accretion radius

### Gameplay Tips

1. **Watch Your Velocity**: In N-body physics, momentum matters! Plan your trajectory.
2. **Use Gravity**: Let external potentials and asteroid gravity work for you.
3. **Beware Black Holes**: Red warning circles show accretion radius. Stay away!
4. **Asteroid Collisions**: Asteroids bounce elastically off each other - watch for chaotic trajectories!
5. **Bullet Physics**: Bullets are affected by gravity too. Curve your shots!
6. **Mass Matters**: Heavier objects pull harder. Large asteroids create local gravity wells.
7. **Brake Control**: Use the brake to slow down and gain fine control in tricky situations.
8. **No Friendly Fire**: Player bullets never damage the other player's ship.

## GitHub Pages Deployment

1. **Build the project**
   ```bash
   npm run build
   ```

2. **Deploy to GitHub Pages**

   Option A: Using `gh-pages` package
   ```bash
   npm install -D gh-pages
   npx gh-pages -d dist
   ```

   Option B: Manual deployment
   ```bash
   # Push dist folder to gh-pages branch
   git add dist -f
   git commit -m "Deploy to GitHub Pages"
   git subtree push --prefix dist origin gh-pages
   ```

3. **Enable GitHub Pages**
   - Go to repository Settings > Pages
   - Source: Deploy from branch `gh-pages`
   - Root directory: `/` (root)
   - Save

4. **Access your game**
   - URL: `https://<username>.github.io/<repository>/`
   - May take a few minutes to deploy

### Important Notes for GitHub Pages

- The game is fully self-contained and works offline after initial load
- WASM file is loaded from the same origin (no CORS issues)
- All assets are bundled in the `dist/` folder

## Project Structure

```
nbody-wars/
├── engine/              # C++ physics engine
│   ├── vec2.h          # 2D vector math
│   ├── quadtree.h/cpp  # Barnes-Hut quadtree
│   ├── potential.h/cpp # External potentials
│   ├── entity.h/cpp    # Game entities
│   ├── collision.h/cpp # Collision detection
│   ├── engine.h/cpp    # Main physics engine
│   ├── api.cpp         # C API for WASM
│   └── Makefile        # Emscripten build
├── src/                # TypeScript frontend
│   ├── types.ts        # Type definitions
│   ├── physics.ts      # WASM wrapper
│   ├── renderer.ts     # Canvas rendering
│   ├── input.ts        # Input handling
│   ├── audio.ts        # Sound effects
│   ├── ui.ts           # UI management
│   ├── game.ts         # Main game loop
│   └── main.ts         # Entry point
├── public/             # Static assets
│   ├── physics.js      # Generated by Emscripten
│   └── physics.wasm    # Generated by Emscripten
├── index.html          # Main HTML
├── package.json        # Node dependencies
├── tsconfig.json       # TypeScript config
└── vite.config.ts      # Vite build config
```

## Technical Details

### Performance

- **Target**: 60 FPS with 1000+ bodies
- **Physics Rate**: 120 Hz fixed timestep
- **Optimization**: Barnes-Hut reduces O(N²) to O(N log N)
- **Memory**: Efficient reuse of buffers, minimal allocations per frame

### Architecture

```
┌─────────────┐
│  TypeScript │ ← User input, rendering, UI
│  Frontend   │
└──────┬──────┘
       │ C API
┌──────▼──────┐
│    WASM     │ ← Physics, collisions, gameplay
│   Engine    │
└─────────────┘
```

### Browser Compatibility

- Modern browsers with WebAssembly support
- Chrome, Firefox, Safari, Edge (latest versions)
- Gamepad API support for controllers

## Educational Use

This game is designed for:
- **Physics Education**: Demonstrates real gravitational N-body dynamics
- **Game Development**: Shows WASM integration with JavaScript
- **Algorithm Learning**: Practical Barnes-Hut quadtree implementation
- **Computational Astrophysics**: Real potentials used in galaxy simulations

## License

MIT License - Feel free to use for educational purposes

## Credits

- Built with TypeScript, WebAssembly (Emscripten), and Vite
- Inspired by classic Asteroids and modern N-body simulators
- Physics concepts from computational astrophysics literature

## Contributing

Issues and pull requests welcome! Areas for improvement:
- Additional potential types
- More game modes
- Visual effects and polish
- Mobile touch controls
- Save/load game states

---

**Enjoy exploring N-body physics through gameplay!** 🚀✨
