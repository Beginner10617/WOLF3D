from PIL import Image
import numpy as np

# Load image
img = Image.open("0flag-2.png").convert("RGB")
arr = np.array(img, dtype=np.float32)

# Flag bounds
x_start, x_end = 17, 48
y_start, y_end = 12, 38
shade_y = 39  # shading strip row

# Extract shading strip (grayscale brightness)
# Since it's pure gray, any channel works
shade_strip = arr[shade_y, x_start:x_end, 0] / 255.0  # normalize to 0–1

# Apply shading
for y in range(y_start, y_end):
    for i, x in enumerate(range(x_start, x_end)):
        factor = shade_strip[i]
        arr[y, x] *= factor  # multiply RGB by brightness

# Clamp and convert back
arr = np.clip(arr, 0, 255).astype(np.uint8)
result = Image.fromarray(arr)

# Save output
result.save("isnotrial_shaded.png")
