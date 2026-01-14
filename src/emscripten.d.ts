// Type definitions for Emscripten module

interface EmscriptenModule {
  _malloc(size: number): number;
  _free(ptr: number): void;
  HEAPF32: Float32Array;
  HEAP32: Int32Array;
  UTF8ToString(ptr: number): string;
}
