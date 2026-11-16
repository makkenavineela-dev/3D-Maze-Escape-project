"""Generates the game's textures into the textures/ folder.

    python tools/make_textures.py

wall.png    stone bricks, tiles seamlessly (512 x 512, 2 m x 2 m in the game)
ground.png  earth with pebbles, tiles seamlessly
font.png    white letters (ASCII 32-127) on a transparent grid, 16 per row
font.txt    font grid sizes read by Overlay.cpp: cell_width cell_height pad_x pad_y advance line_height

Everything is generated from random noise, so there are no image licences to worry about.
The font is DejaVu Sans Mono Bold (free licence), which ships with matplotlib.
"""
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

SIZE = 512
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "textures")
rng = np.random.default_rng(7)


def tiled_noise(cells, blur):
    """Smooth random noise in 0..1 that wraps around at the edges (no visible seams when tiled)."""
    small = Image.fromarray((rng.random((cells, cells)) * 255).astype(np.uint8))
    big = small.resize((SIZE, SIZE), Image.Resampling.BICUBIC)
    tiled = Image.new("L", (SIZE * 3, SIZE * 3))
    for ty in range(3):
        for tx in range(3):
            tiled.paste(big, (tx * SIZE, ty * SIZE))
    tiled = tiled.filter(ImageFilter.GaussianBlur(blur)).crop((SIZE, SIZE, 2 * SIZE, 2 * SIZE))
    a = np.asarray(tiled, dtype=np.float32) / 255.0
    return (a - a.min()) / (a.max() - a.min() + 1e-6)


def fractal_noise(octaves):
    """Several noise layers from coarse to fine added together."""
    total, weight = np.zeros((SIZE, SIZE), np.float32), 0.0
    for cells, blur, w in octaves:
        total += tiled_noise(cells, blur) * w
        weight += w
    return total / weight


def save(array, name):
    Image.fromarray(np.clip(array * 255, 0, 255).astype(np.uint8)).save(os.path.join(OUT, name))
    print("wrote", name)


def make_wall():
    brick_w, brick_h, mortar = 128, 64, 6
    y, x = np.mgrid[0:SIZE, 0:SIZE]
    row = y // brick_h
    xs = (x + (row % 2) * (brick_w // 2)) % SIZE          # every other row shifted by half a brick
    col = xs // brick_w
    in_x, in_y = xs % brick_w, y % brick_h
    edge = np.minimum(np.minimum(in_x - mortar, brick_w - 1 - in_x), np.minimum(in_y - mortar, brick_h - 1 - in_y))
    is_mortar = (in_x < mortar) | (in_y < mortar)

    # each brick gets its own shade and slight colour shift
    shades = rng.uniform(0.82, 1.12, size=(SIZE // brick_h, SIZE // brick_w))
    warmth = rng.uniform(-0.04, 0.04, size=shades.shape)
    shade, warm = shades[row, col], warmth[row, col]

    grain = fractal_noise([(16, 6, 0.5), (64, 2, 0.3), (256, 0.6, 0.2)])
    stone = np.stack([0.60 + warm, np.full_like(warm, 0.56), 0.50 - warm], axis=-1)
    stone *= (shade * (0.80 + 0.35 * grain))[..., None]
    bevel = np.clip(edge / 5.0, 0.0, 1.0) * 0.25 + 0.75     # brick edges slightly darker
    stone *= bevel[..., None]
    mortar_colour = np.stack([0.33, 0.31, 0.28], axis=-1) * (0.85 + 0.3 * grain)[..., None]
    save(np.where(is_mortar[..., None], mortar_colour, stone), "wall.png")


def make_ground():
    variation = fractal_noise([(8, 10, 0.5), (32, 4, 0.3), (128, 1.2, 0.2)])
    grain = tiled_noise(512, 0.4)
    base = np.stack([0.42, 0.36, 0.29], axis=-1) * (0.70 + 0.45 * variation + 0.12 * (grain - 0.5))[..., None]

    # scattered pebbles, drawn on a wrap-around canvas so they tile too
    pebbles = Image.new("L", (SIZE, SIZE), 0)
    draw = ImageDraw.Draw(pebbles)
    for _ in range(260):
        cx, cy, r = rng.uniform(0, SIZE), rng.uniform(0, SIZE), rng.uniform(2, 7)
        value = int(rng.uniform(90, 255))
        for ox in (-SIZE, 0, SIZE):
            for oy in (-SIZE, 0, SIZE):
                draw.ellipse((cx + ox - r, cy + oy - r * 0.8, cx + ox + r, cy + oy + r * 0.8), fill=value)
    stones = np.asarray(pebbles.filter(ImageFilter.GaussianBlur(0.8)), np.float32) / 255.0
    pebble_colour = np.stack([0.58, 0.55, 0.50], axis=-1) * (0.6 + 0.4 * stones)[..., None]
    mix = np.clip(stones * 1.6, 0.0, 1.0)[..., None]
    save(base * (1 - mix) + pebble_colour * mix, "ground.png")


def make_font():
    import matplotlib
    font_path = os.path.join(os.path.dirname(matplotlib.__file__), "mpl-data", "fonts", "ttf", "DejaVuSansMono-Bold.ttf")
    font = ImageFont.truetype(font_path, 64)
    ascent, descent = font.getmetrics()
    advance = font.getlength("M")
    pad_x, pad_y = 12, 12
    cell_w, cell_h = int(advance + 0.999) + 2 * pad_x, ascent + descent + 2 * pad_y
    atlas = Image.new("RGBA", (16 * cell_w, 6 * cell_h), (255, 255, 255, 0))
    draw = ImageDraw.Draw(atlas)
    for code in range(32, 128):
        i = code - 32
        x, y = (i % 16) * cell_w + pad_x, (i // 16) * cell_h + pad_y
        draw.text((x, y), chr(code) if code < 127 else "#", font=font, fill=(255, 255, 255, 255))
    atlas.save(os.path.join(OUT, "font.png"))
    with open(os.path.join(OUT, "font.txt"), "w") as f:
        f.write(f"{cell_w} {cell_h} {pad_x} {pad_y} {advance:.3f} {ascent + descent}\n")
    print("wrote font.png", atlas.size, "and font.txt")


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    make_wall()
    make_ground()
    make_font()
    sys.exit(0)
