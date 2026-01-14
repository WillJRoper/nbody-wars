export class AudioManager {
  private audioContext: AudioContext | null = null;
  private masterGain: GainNode | null = null;
  private enabled: boolean = true;

  constructor() {
    try {
      this.audioContext = new AudioContext();
      this.masterGain = this.audioContext.createGain();
      this.masterGain.connect(this.audioContext.destination);
      this.masterGain.gain.value = 0.3;
    } catch (e) {
      console.warn('Web Audio API not supported:', e);
      this.enabled = false;
    }
  }

  private createOscillator(frequency: number, duration: number, type: OscillatorType = 'sine'): void {
    if (!this.enabled || !this.audioContext || !this.masterGain) return;

    const oscillator = this.audioContext.createOscillator();
    const gainNode = this.audioContext.createGain();

    oscillator.connect(gainNode);
    gainNode.connect(this.masterGain);

    oscillator.type = type;
    oscillator.frequency.value = frequency;

    gainNode.gain.setValueAtTime(0.3, this.audioContext.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.01, this.audioContext.currentTime + duration);

    oscillator.start(this.audioContext.currentTime);
    oscillator.stop(this.audioContext.currentTime + duration);
  }

  playShoot(): void {
    this.createOscillator(800, 0.1, 'square');
    setTimeout(() => {
      this.createOscillator(400, 0.05, 'square');
    }, 50);
  }

  playExplosion(): void {
    if (!this.enabled || !this.audioContext || !this.masterGain) return;

    const duration = 0.3;
    const noise = this.audioContext.createBufferSource();
    const buffer = this.audioContext.createBuffer(1, this.audioContext.sampleRate * duration, this.audioContext.sampleRate);
    const data = buffer.getChannelData(0);

    for (let i = 0; i < data.length; i++) {
      data[i] = Math.random() * 2 - 1;
    }

    noise.buffer = buffer;

    const filter = this.audioContext.createBiquadFilter();
    filter.type = 'lowpass';
    filter.frequency.value = 1000;

    const gainNode = this.audioContext.createGain();
    gainNode.gain.setValueAtTime(0.5, this.audioContext.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.01, this.audioContext.currentTime + duration);

    noise.connect(filter);
    filter.connect(gainNode);
    gainNode.connect(this.masterGain);

    noise.start(this.audioContext.currentTime);
    noise.stop(this.audioContext.currentTime + duration);
  }

  playThrust(): void {
    if (!this.enabled || !this.audioContext || !this.masterGain) return;

    const oscillator = this.audioContext.createOscillator();
    const gainNode = this.audioContext.createGain();

    oscillator.connect(gainNode);
    gainNode.connect(this.masterGain);

    oscillator.type = 'sawtooth';
    oscillator.frequency.value = 60 + Math.random() * 20;

    gainNode.gain.setValueAtTime(0.05, this.audioContext.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.01, this.audioContext.currentTime + 0.05);

    oscillator.start(this.audioContext.currentTime);
    oscillator.stop(this.audioContext.currentTime + 0.05);
  }

  playWarning(): void {
    this.createOscillator(200, 0.2, 'triangle');
    setTimeout(() => {
      this.createOscillator(150, 0.2, 'triangle');
    }, 200);
  }

  setVolume(volume: number): void {
    if (this.masterGain) {
      this.masterGain.gain.value = Math.max(0, Math.min(1, volume));
    }
  }

  setEnabled(enabled: boolean): void {
    this.enabled = enabled;
  }
}
