import os
from PIL import Image

def convert_bmp_to_png_transparent():
    for filename in os.listdir("."):
        if not filename.lower().endswith(".bmp"):
            continue

        bmp_path = filename
        png_path = os.path.splitext(filename)[0] + ".png"

        img = Image.open(bmp_path).convert("RGBA")
        pixels = img.load()

        width, height = img.size

        # Get top-left pixel color (R, G, B, A)
        transparent_color = pixels[0, 0][:3]

        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                if (r, g, b) == transparent_color:
                    pixels[x, y] = (r, g, b, 0)  # make transparent

        img.save(png_path, "PNG")
        print(f"Converted: {bmp_path} -> {png_path}")

if __name__ == "__main__":
    convert_bmp_to_png_transparent()
