@if not exist build mkdir build
@if not exist build\bin mkdir build\bin
@if not exist build\obj mkdir build\obj

:: only raylibdll.lib works.  the static one fails with something about glfw (sure, raylib depends on glfw, but why don't I need it for dll?).  just not understanding something about static libs on windows I suppose.
:: TODO think I just need gdi32.lib in this libs list to do static linking.  and maybe opengl32.lib
@set libs=gdi32.lib msvcrt.lib raylibdll.lib winmm.lib geotiff_i.lib tiff.lib
cl -nologo -W2 -Z7 /Fe:build/bin/ /Fo:build/obj/ %* %libs% main.c -link -NODEFAULTLIB:libcmt

@if %errorlevel% neq 0 exit /b %errorlevel%

cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ imgconv.c

@if %errorlevel% neq 0 exit /b %errorlevel%

@echo off
if not exist local mkdir local
pushd local
if not exist world.200405.3x10800x10800.A1.raw (
    if not exist world.200405.3x21600x21600.A1.jpg (
        curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A1.jpg
    )
    ..\build\bin\imgconv.exe raw world.200405.3x21600x21600.A1.jpg --width 10800 --height 10800
    move world.200405.3x21600x21600.A1.raw world.200405.3x10800x10800.A1.raw
)
popd

@if %errorlevel% neq 0 exit /b %errorlevel%
