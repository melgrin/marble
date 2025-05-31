@cd /d "%~dp0"
@if not exist build mkdir build
@if not exist build\bin mkdir build\bin
@if not exist build\obj mkdir build\obj

:: raylib.lib needs to come before user32.lib, otherwise there's a symbol clash with "CloseWindow".
@set libs=deps/build/raylib.lib deps/build/libtiff.lib deps/build/imgui.lib gdi32.lib msvcrt.lib winmm.lib user32.lib shell32.lib
cl -nologo -W2 -Z7 -Fe:build/bin/marble.exe -Fo:build/obj/ ^
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
:: resize to match topo image size and inflate to improve load times
set w0=21600
set h0=21600
set w1=10800
set h1=10800
set bmng_jpg=..\deps\marble_data\bmng\world.200405.3x%w0%x%h0%.A1.jpg
set bmng_raw=world.200405.3x%w1%x%h1%.A1.raw
if not exist %bmng_raw% (
    if not exist %bmng_jpg% (
        echo "Error: missing image data.  Make sure that git submodules have been initialized.  Looking for %cd%\%bmng_jpg% but it does not exist."
        exit /b 1
    )
    ..\build\bin\imgconv.exe raw %bmng_jpg% --width %w1% --height %h1% --output %bmng_raw%
    @if %errorlevel% neq 0 exit /b %errorlevel%
)
popd

@if %errorlevel% neq 0 exit /b %errorlevel%

set "cwd=%cd%"
call test\build.bat
@if %errorlevel% neq 0 exit /b %errorlevel%
cd /d "%cwd%"

echo:
echo %date% %time%
