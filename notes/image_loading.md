## Image Loading Notes

Decompressing the color image during startup is too slow.  Loading the A1 .jpg takes ~10 seconds.  So I made an uncompressed format, .raw, which only takes ~1 second to load the same image.

During the build, the color image is converted from .jpg to .raw by `../imgconv.c`.  Since .raw takes 8x more space, it's only stored locally.

10800x10800x3 load time comparison

| format | load time    | file size |
| :----- | -----------: | --------: |
| .raw   | 0.18 seconds |   1367 KB |
| .png   | 4.75 seconds |     76 KB |
| .qoi   | 0.85 seconds |     55 KB |

(produced via `curl .jpg` -> `stbi_load` -> `stbir_resize_uint8_srgba` -> format-specific write)

I'm only sending a small region to the GPU right now (~60k), so that's not a concern, but may become one if I increase the size/number of loaded tiles.

## Extra Notes

- The png uncompressed size is ~1.3GB.
- stb image can't load PNGs larger than 1 GB.
- BMP, while uncompressed, it still is extremely slow to load.  The BMP spec says "all rows are padded to a multiple of 4 bytes".  I'm using images that have 3 color channels (RBG), so only 3 bytes.  Expanding this to 4 is probably the cause of the slowness.
- qoi is a good middle ground, but can't load more than 400 million pixels.  I'm using images that are 21600 x 21600, so 466 million pixels.
