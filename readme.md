## Overview

Low-fidelity topographical 3D earth surface viewer


## References

- [NASA Blue Marble](https://visibleearth.nasa.gov/collection/1484/blue-marble)
- [raylib](https://github.com/raysan5/raylib), particularly `raylib/examples/models/models_heightmap.c`
- [stb](https://github.com/nothings/stb)
- [qoi](https://github.com/phoboslab/qoi)
- [imgui](https://github.com/ocornut/imgui), [cimgui](https://github.com/cimgui/cimgui), [rlImGui](https://github.com/raylib-extras/rlImGui)


## Building

First build:
```
git submodule update --init --recursive
vcvarsall x64
cd deps
build.bat
cd ..
cl build.c
build.exe
```

Subsequent builds:
```
build.exe
```


## Running

```
.\build\bin\marble.exe
```

## How does it work?

- There are two images, topographic and color (BMNG).
- Both are of the same region ("A1" region from NASA's global image grid, currently).
- Topographic is used to make a heightmap.
- Color is used as a texture, overlaid on the heightmap.


## Image Credits

- Blue Marble Next Generation: Reto Stöckli, NASA Earth Observatory.
- Topography: Imagery by Jesse Allen, NASA's Earth Observatory, using data from the General Bathymetric Chart of the Oceans (GEBCO) produced by the British Oceanographic Data Centre.

