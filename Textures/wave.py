import os
import math
import numpy as np
from PIL import Image

# ---------------- CONFIG ----------------
INPUT_IMAGE = "avatar.png"
OUTPUT_DIR = "Avatar"
FRAMES = 5

AMPLITUDE = 3              # vertical pixel shift
FREQUENCY = 0.15           # wave density along X
BRIGHTNESS_STRENGTH = 0.15 # lighting illusion
# ----------------------------------------

os.makedirs(OUTPUT_DIR, exist_ok=True)

img = Image.open(INPUT_IMAGE).convert("RGBA")
width, height = img.size
pixels = np.array(img)

for frame in range(FRAMES):
    new_pixels = np.zeros_like(pixels)
    phase = (frame / FRAMES) * 2 * math.pi

    for x in range(width):
        wave = math.sin(x * FREQUENCY + phase)
        y_offset = int(wave * AMPLITUDE)
        brightness = 1.0 + wave * BRIGHTNESS_STRENGTH

        for y in range(height):
            src_y = y - y_offset

            if 0 <= src_y < height:
                r, g, b, a = pixels[src_y, x]

                r = int(max(0, min(255, r * brightness)))
                g = int(max(0, min(255, g * brightness)))
                b = int(max(0, min(255, b * brightness)))

                new_pixels[y, x] = (r, g, b, a)
            else:
                new_pixels[y, x] = (0, 0, 0, 0)

    Image.fromarray(new_pixels, "RGBA").save(
        f"{OUTPUT_DIR}/frame_{frame}.png"
    )

print("✔ Horizontal waving flag frames generated in ./Avatar/")
