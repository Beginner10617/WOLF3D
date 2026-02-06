# WOLF3D‑Like Game (C++ / SDL2)

A simple **Wolfenstein 3D–style raycasting game** written in **C++** using **SDL2**.
This project is a recreation of classic early‑90s FPS mechanics: grid‑based maps, raycasting for pseudo‑3D rendering, and fast keyboard‑driven movement.

> ⚠️ This is **not** a clone of the original Wolfenstein 3D assets or code. All logic is written from scratch.

---
## Demo
<div align="center">
<img src="DEMO.gif" alt="Alt text" width="400" height="250">
</div>

---

## Features

* Classic **raycasting renderer** (2.5D)
* Grid‑based map system
* Real‑time player movement and rotation
* Wall collision detection
* Distance‑based wall shading
* SDL2 window, rendering, and input handling
* Minimal dependencies

---

## Controls

| Key   | Action        |
| ----- | ------------- |
| `W ↑` | Move forward  |
| `S ↓` | Move backward |
| `A`   | Move leftward |
| `D`   | Move rightward|
| `←`   | Rotate left   |
| `→`   | Rotate right  |
| `ESC` | Pause game    |

(Controls can be changed easily in `InputManager.cpp`)

---

## How It Works (Raycasting Overview)

The engine uses **raycasting**, where:

1. A ray is cast for every vertical column of the screen.
2. Each ray steps through the grid map until it hits a wall.
3. The distance to the wall determines the height of the vertical slice.
4. Walls farther away appear shorter, creating a depth illusion.

This technique was famously used in games like **Wolfenstein 3D (1992)**.

---

## Map System

* Maps are stored as a **2D integer grid**
* `0` represents empty space
* Non‑zero values represent walls and doors (6-9 for doors)
* Locked (7-9) and Unlocked doors (6)
* Special characters for collectibles
  
Example:

```
1 1 1 1 1 1
1 0 0 0 0 1
1 0 1 0 0 1
1 0 0 0 0 1
1 1 1 1 1 1
```

---

## Build Instructions

### Dependencies

* **C++17 or later**
* **SDL2**

---

### Compile

```bash
make
```
Or for clean compile 
```bash
make clean
make
```

---

### Run

```bash
./main
```

---

## Known Limitations

* No floor/ceiling texturing
* Single‑level map

---

## Features

* Texture‑mapped walls
* Sprite rendering (enemies, collectibles)
* Mouse look (left-right only)
* Doors and interactable objects
* Simple AI enemies

---

## References & Inspiration

* *Wolfenstein 3D* (id Software, 1992)
* SDL2 Documentation

---

Built as a learning project to understand low‑level rendering, game loops, and classic FPS engines.

If you like this project, feel free to ⭐ the repo or fork it!
