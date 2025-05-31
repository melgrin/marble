@echo off
setlocal enabledelayedexpansion

cd /D "%~dp0"

if not exist build mkdir build

set "compile_flags=-Z7"

:::: libtiff

:: note: there doesn't seem to be a way to exclude the write portions of the library during compilation.  would need to modify source.
:: TODO: disable more of the internal compression schemes in tiffconf.h and then remove the corresponding source files here.
set files=^
    tif_open ^
    tif_win32 ^
    tif_close ^
    tif_dir ^
    tif_strip ^
    tif_read ^
    tif_version ^
    tif_dirread ^
    tif_swab ^
    tif_compress ^
    tif_tile ^
    tif_hash_set ^
    tif_aux ^
    tif_error ^
    tif_warning ^
    tif_codec ^
    tif_dirinfo ^
    tif_next ^
    tif_lzw ^
    tif_luv ^
    tif_thunder ^
    tif_fax3 ^
    tif_fax3sm ^
    tif_write ^
    tif_packbits ^
    tif_dumpmode ^
    tif_predict ^
    tif_flush ^
    tif_dirwrite ^
    tif_getimage ^
    tif_color

set "srcs="
set "objs="
for %%f in (%files%) do (
    set "srcs=!srcs! ./libtiff/libtiff/%%f.c"
    set "objs=!objs! ./build/%%f.obj"
)

cl -nologo -c -I ./libtiff_config/ -Fo:./build/ %compile_flags% %srcs%
if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/libtiff.lib %objs%
if %errorlevel% neq 0 exit /b %errorlevel%


:::: raylib

set raylib_config_override=-I . -FI ./raylib_config/config.h
cl -nologo -c %raylib_config_override% -Fo:./build/ -I ./raylib/src/ %compile_flags% ^
    -I ./raylib/src/external/glfw/include -DPLATFORM_DESKTOP=1 ^
    ./raylib/src/rcore.c
cl -nologo -c %raylib_config_override% -Fo:./build/ -I ./raylib/src/ %compile_flags% ^
    ./raylib/src/rshapes.c ^
    ./raylib/src/rtextures.c ^
    ./raylib/src/rtext.c ^
    ./raylib/src/rmodels.c ^
    ./raylib/src/utils.c ^
    ./raylib/src/rglfw.c
if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/raylib.lib ^
    ./build/rcore.obj ^
    ./build/rshapes.obj ^
    ./build/rtextures.obj ^
    ./build/rtext.obj ^
    ./build/rmodels.obj ^
    ./build/utils.obj ^
    ./build/rglfw.obj
if %errorlevel% neq 0 exit /b %errorlevel%


:::: imgui + cimgui + rlImGui

:: -D IMGUI_DISABLE_OBSOLETE_FUNCTIONS - required otherwise DebugCheckVersionAndDataLayout fails: "Assertion failed: sz_io == sizeof(ImGuiIO) && "Mismatched struct layout!", file imgui\imgui.cpp, line 10390"
:: -D CIMGUI_NO_EXPORT - static linking, so no need for this.  otherwise, creates .lib and .exp alongside .exe.
:: -D CIMGUI_DEFINE_ENUMS_AND_STRUCTS - exposes C API of cimgui.  only use this when compiling C, not C++.
:: -D NO_FONT_AWESOME - rlImGui - don't think I want extra fonts right now, no matter how awesome they are.

cl -nologo -c -MT -EHsc ^
    -D IMGUI_DISABLE_OBSOLETE_FUNCTIONS ^
    -D CIMGUI_NO_EXPORT ^
    -D NO_FONT_AWESOME ^
    -I ./cimgui/imgui ^
    -I ./cimgui ^
    -I ./rlImGui ^
    -I ./raylib/src ^
    -Fo:./build/ ^
    %compile_flags% ^
    ./cimgui/imgui/imgui.cpp ^
    ./cimgui/imgui/imgui_demo.cpp ^
    ./cimgui/imgui/imgui_draw.cpp ^
    ./cimgui/imgui/imgui_tables.cpp ^
    ./cimgui/imgui/imgui_widgets.cpp ^
    ./cimgui/cimgui.cpp ^
    ./rlImGui/rlImGui.cpp
if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/imgui.lib ^
    ./build/imgui.obj ^
    ./build/imgui_demo.obj ^
    ./build/imgui_draw.obj ^
    ./build/imgui_tables.obj ^
    ./build/imgui_widgets.obj ^
    ./build/cimgui.obj ^
    ./build/rlImGui.obj
if %errorlevel% neq 0 exit /b %errorlevel%

