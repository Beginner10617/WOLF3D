import random

MAP_FILE = "map.txt"
OUT_FILE = "enemies.txt"

# Default density for cells not inside any window
DEFAULT_DENSITY = 0.05

# Window-specific densities
# (x1, y1, x2, y2) -> density
DENSITY_WINDOWS = {
    (0, 0, 4, 4): 0,
    (1, 1, 5, 2): 0.30,   
    (2, 2, 4, 3): 0.60,   
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
