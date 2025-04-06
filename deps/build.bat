@echo off
setlocal enabledelayedexpansion

cd /D "%~dp0"

if not exist build mkdir build

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

cl -nologo -c -I ./libtiff_config/ -Fo:./build/ %srcs%
if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/libtiff.lib %objs%
if %errorlevel% neq 0 exit /b %errorlevel%

:::: raylib

set raylib_config_override=-I . -FI ./raylib_config/config.h
cl -nologo -c %raylib_config_override% -Fo:./build/ -I ./raylib/src/ ^
    -I ./raylib/src/external/glfw/include -DPLATFORM_DESKTOP=1 ^
    ./raylib/src/rcore.c
cl -nologo -c %raylib_config_override% -Fo:./build/ -I ./raylib/src/ ^
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

