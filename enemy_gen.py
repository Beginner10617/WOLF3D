import random

MAP_FILE = "map.txt"
OUT_FILE = "enemies.txt"

# Default density for cells not inside any window
DEFAULT_DENSITY = 0.0

# Window-specific densities
# (x1, y1, x2, y2) -> density
DENSITY_WINDOWS = {
(5,1,7,7)	:	0.09524,
(9,3,21,5)	:	0.05128,
(23,1,29,7)	:	0.04082,
(31,3,36,5)	:	0.05556,
(38,1,47,7)	:	0.02857,
(49,3,54,5)	:	0.11111,
(56,1,62,7)	:	0.06122,
(56,14,62,20)	:	0.08163,
(58,22,60,27)	:	0.16667,
(56,29,62,35)	:	0.08163,
(58,37,60,42)	:	0.16667,
(56,44,62,50)	:	0.10204,
(56,56,62,62)	:	0.06122,
(43,56,49,62)	:	0.08163,
(36,58,41,60)	:	0.11111,
(28,56,34,62)	:	0.08163,
(21,58,26,60)	:	0.11111,
(13,56,19,62)	:	0.10204,
(1,56,7,62)	:	0.08163,
(3,44,46,46)	:	0.09848,
(33,28,41,36)	:	0.06173,
(17,28,25,36)	:	0.06173,
(6,28,9,36)	:	0.08333
}

RANDOM_SEED = None
# RANDOM_SEED = 42

def is_wall(token):
    """Walls are non-zero integers."""
    try:
        return int(token) != 0
    except ValueError:
        return False


def load_map(path):
    with open(path) as f:
        return [line.split() for line in f if line.strip()]


def density_for_cell(x, y):
    """
    Returns density for a given cell based on windows.
    First matching window wins.
    """
    for (x1, y1, x2, y2), density in DENSITY_WINDOWS.items():
        if x1 <= x <= x2 and y1 <= y <= y2:
            return density
    return DEFAULT_DENSITY


def generate_enemies(grid):
    enemies = []
    h = len(grid)
    w = len(grid[0])

    for y in range(h):
        for x in range(w):
            if is_wall(grid[y][x]):
                continue

            density = density_for_cell(x, y)
            if random.random() < density:
                enemies.append((x + 0.5, y + 0.5))

    return enemies


def main():
    if RANDOM_SEED is not None:
        random.seed(RANDOM_SEED)

    grid = load_map(MAP_FILE)
    enemies = generate_enemies(grid)

    with open(OUT_FILE, "w") as f:
        for x, y in enemies:
            f.write(f"{x} {y}\n")

    print(f"Placed {len(enemies)} enemies → {OUT_FILE}")


if __name__ == "__main__":
    main()
