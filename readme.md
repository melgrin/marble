There's two images, topo and color.
Both are cropped to be the Seattle region.
Topo is used to make a heightmap.
Color is used as a texture, overlaid on the heightmap.

It loads from a .raw, which is a format I made up.  run `bin\stbi_to_raw.exe <image.jpg>` to create from .jpg.  I did this because stb image can't load PNGs larger than 1 GB.  The uncompressed size is ~1.3GB.  Loading from .jpg takes ~10 seconds, loading from .raw takes ~1 second.  I made up a format because my very-not-extensive searching didn't turn up with anything reasonable.  BMP is extremely slow.  I think that's because of the "all rows are padded to a multiple of 4 bytes" part of the BMP spec.  qoi seems like a nice format (though still compressed, so maybe wouldn't want it for speed), but can't load more than 400 million pixels.  I'm using images that are 21600 x 21600, so 466 million pixels.  The obvious alternative to all of this is to break up the images into smaller ones.  But that's only a solution to the arbitrary size limits of the aforementioned libraries, and not a solution for load times.  Although probably you wouldn't notice load times if they were scaled down by 100x, but then that's 100x image fragments that are part of the build and runtime of this project.  This brings up the question of why are people afraid of 2 GB?  Since I'm only sending a small region to the GPU right now (~60k), that's fine, and everyone has tens of gigs of RAM now.

10800x10800x3 (produced via `curl .jpg` -> `stbi_load` -> `stbir_resize_uint8_srgba` -> format-specific write)
load info
| format | load time    | file size |
| :----- | -----------: | --------: |
| .raw   | 0.18 seconds |   1367 KB |
| .png   | 4.75 seconds |     76 KB |
| .qoi   | 0.85 seconds |     55 KB |

- [NASA Blue Marble](https://visibleearth.nasa.gov/collection/1484/blue-marble)
- [raylib](https://github.com/raysan5/raylib), particularly `raylib/examples/models/models_heightmap.c`
- [stb](https://github.com/nothings/stb)
- [qoi](https://github.com/phoboslab/qoi)

todo:
- single image grid region (A1, C2, etc)
  - streaming
- handling for all image grids
  - download all?
  - disk space vs load time?
- osm borders, places, buildings
- place search?

