# Makefile for building the physics engine with Emscripten

CXX = em++
CXXFLAGS = -std=c++17 -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 \
           -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall","UTF8ToString","HEAP8","HEAPF32"]' \
           -s EXPORTED_FUNCTIONS='["_malloc","_free"]' \
           -s MODULARIZE=1 -s EXPORT_NAME='createPhysicsModule' \
           -s ENVIRONMENT='web' -s SINGLE_FILE=0

SOURCES = vec2.h quadtree.cpp potential.cpp entity.cpp collision.cpp engine.cpp api.cpp
OUTPUT = ../public/physics.js

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CXX) $(CXXFLAGS) quadtree.cpp potential.cpp entity.cpp collision.cpp engine.cpp api.cpp -o $(OUTPUT)

clean:
	rm -f $(OUTPUT) ../public/physics.wasm

.PHONY: all clean
