from PIL import Image
import os

# Configuration
IMAGE_PATH = "avatar.png"
ORIGINAL_SIZE = 32
FINAL_WIDTH = 25
HEIGHT = 32
GAP = 1
OUTPUT_DIR = "sprites"

os.makedirs(OUTPUT_DIR, exist_ok=True)

img = Image.open(IMAGE_PATH)
img_width, img_height = img.size

sprite_index = 0

y = GAP
while y + ORIGINAL_SIZE <= img_height:
    x = GAP
    while x + ORIGINAL_SIZE <= img_width:
        # Crop only the left 25px of the 32px tile
        box = (x, y, x + FINAL_WIDTH, y + HEIGHT)
        sprite = img.crop(box)
        sprite.save(os.path.join(OUTPUT_DIR, f"sprite_{sprite_index}.png"))
        sprite_index += 1

        x += ORIGINAL_SIZE + GAP
    y += ORIGINAL_SIZE + GAP

print(f"Extracted {sprite_index} sprites (25x32).")
