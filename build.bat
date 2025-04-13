@if not exist build mkdir build
@if not exist build\bin mkdir build\bin
@if not exist build\obj mkdir build\obj

:: raylib.lib needs to come before user32.lib, otherwise there's a symbol clash with "CloseWindow".
@set libs=deps/build/raylib.lib deps/build/libtiff.lib deps/build/imgui.lib gdi32.lib msvcrt.lib winmm.lib user32.lib shell32.lib
cl -nologo -W2 -Z7 -Fe:build/bin/ -Fo:build/obj/ ^
 -I deps/stb ^
 -I deps/qoi ^
 -I deps/raylib/src ^
 -I deps/libtiff_config ^
 -I deps/libtiff/libtiff ^
 -I deps/cimgui ^
 -I deps/rlImGui ^
 %* ^
 %libs% ^
 main.c ^
 -link -NODEFAULTLIB:libcmt

@if %errorlevel% neq 0 exit /b %errorlevel%

cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ ^
 -I deps/stb ^
 -I deps/qoi ^
 -I deps/libtiff_config ^
 -I deps/libtiff/libtiff ^
 deps/build/libtiff.lib ^
 imgconv.c

@if %errorlevel% neq 0 exit /b %errorlevel%

@echo off
if not exist local mkdir local
pushd local
if not exist world.200405.3x10800x10800.A1.raw (
    if not exist world.200405.3x21600x21600.A1.jpg (
        curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A1.jpg
    )
    ..\build\bin\imgconv.exe raw world.200405.3x21600x21600.A1.jpg --width 10800 --height 10800
    @if %errorlevel% neq 0 exit /b %errorlevel%
    move world.200405.3x21600x21600.A1.raw world.200405.3x10800x10800.A1.raw
)
popd

@if %errorlevel% neq 0 exit /b %errorlevel%

echo:
echo %date% %time%
