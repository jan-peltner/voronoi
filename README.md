# Voronoi Diagram Generator

A simple interactive application that displays animated Voronoi diagrams using GPU shaders. The program renders colorful regions based on moving seed points.

## Features

- GPU-accelerated Voronoi diagram rendering
- Animated seeds with simple physics (boundary collisions)
- Dynamic edge detection with adjustable thickness
- Interactive seed management (spawn, delete)
- Visualization toggles (show/hide seeds, edges)
- Pause/resume functionality
- Uses the Catppuccin Mocha color palette

## Controls
 - <kbd>S</kbd> Spawn a seed
 - <kbd>D</kbd> Delete last seed
 - <kbd>H</kbd> Hide/Show seeds
 - <kbd>E</kbd> Toggle edge detection
 - <kbd>↑</kbd> Increase edge thickness
 - <kbd>↓</kbd> Decrease edge thickness
 - <kbd>Space</kbd> Pause/Resume
 - <kbd>Esc</kbd> Quit

## Quickstart

```bash
./run.sh
```

Compiles the program, copies the necessary shader files to the bin directory, and then runs the executable.

## Requirements

- raylib library
- OpenGL support for shader functionality

## Implementation

The program includes two implementations:
- `main.c`: GPU-accelerated version using fragment shaders
- `cpu.c`: CPU-based implementation (not compiled by default)
