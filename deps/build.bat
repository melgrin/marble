@cd /D "%~dp0"

@if not exist build mkdir build

:::: libtiff

:: note: there doesn't seem to be a way to exclude the write portions of the library during compilation.  would need to modify source.
:: TODO: disable more of the internal compression schemes in tiffconf.h and then remove the corresponding source files here.
cl -nologo -c -I ./libtiff_config/ /Fo:./build/ ^
 ./libtiff/libtiff/tif_open.c ^
 ./libtiff/libtiff/tif_win32.c ^
 ./libtiff/libtiff/tif_close.c ^
 ./libtiff/libtiff/tif_dir.c ^
 ./libtiff/libtiff/tif_strip.c ^
 ./libtiff/libtiff/tif_read.c ^
 ./libtiff/libtiff/tif_version.c ^
 ./libtiff/libtiff/tif_dirread.c ^
 ./libtiff/libtiff/tif_swab.c ^
 ./libtiff/libtiff/tif_compress.c ^
 ./libtiff/libtiff/tif_tile.c ^
 ./libtiff/libtiff/tif_hash_set.c ^
 ./libtiff/libtiff/tif_aux.c ^
 ./libtiff/libtiff/tif_error.c ^
 ./libtiff/libtiff/tif_warning.c ^
 ./libtiff/libtiff/tif_codec.c ^
 ./libtiff/libtiff/tif_dirinfo.c ^
 ./libtiff/libtiff/tif_next.c ^
 ./libtiff/libtiff/tif_lzw.c ^
 ./libtiff/libtiff/tif_luv.c ^
 ./libtiff/libtiff/tif_thunder.c ^
 ./libtiff/libtiff/tif_fax3.c ^
 ./libtiff/libtiff/tif_fax3sm.c ^
 ./libtiff/libtiff/tif_write.c ^
 ./libtiff/libtiff/tif_packbits.c ^
 ./libtiff/libtiff/tif_dumpmode.c ^
 ./libtiff/libtiff/tif_predict.c ^
 ./libtiff/libtiff/tif_flush.c ^
 ./libtiff/libtiff/tif_dirwrite.c
@if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/libtiff.lib ^
 ./build/tif_open.obj ^
 ./build/tif_win32.obj ^
 ./build/tif_close.obj ^
 ./build/tif_dir.obj ^
 ./build/tif_strip.obj ^
 ./build/tif_read.obj ^
 ./build/tif_version.obj ^
 ./build/tif_dirread.obj ^
 ./build/tif_swab.obj ^
 ./build/tif_compress.obj ^
 ./build/tif_tile.obj ^
 ./build/tif_hash_set.obj ^
 ./build/tif_aux.obj ^
 ./build/tif_error.obj ^
 ./build/tif_warning.obj ^
 ./build/tif_codec.obj ^
 ./build/tif_dirinfo.obj ^
 ./build/tif_next.obj ^
 ./build/tif_lzw.obj ^
 ./build/tif_luv.obj ^
 ./build/tif_thunder.obj ^
 ./build/tif_fax3.obj ^
 ./build/tif_fax3sm.obj ^
 ./build/tif_write.obj ^
 ./build/tif_packbits.obj ^
 ./build/tif_dumpmode.obj ^
 ./build/tif_predict.obj ^
 ./build/tif_flush.obj ^
 ./build/tif_dirwrite.obj
@if %errorlevel% neq 0 exit /b %errorlevel%

:::: raylib

cl -nologo -c -Fo:./build/ -I ./raylib/src/ -DPLATFORM_DESKTOP=1 ./raylib/src/rcore.c
cl -nologo -c -Fo:./build/ -I ./raylib/src/ ./raylib/src/rshapes.c ./raylib/src/rtextures.c ./raylib/src/rtext.c ./raylib/src/rmodels.c ./raylib/src/utils.c ./raylib/src/rglfw.c
@if %errorlevel% neq 0 exit /b %errorlevel%

lib -nologo -out:./build/raylib.lib ./build/rcore.obj ./build/rshapes.obj ./build/rtextures.obj ./build/rtext.obj ./build/rmodels.obj ./build/utils.obj ./build/rglfw.obj
@if %errorlevel% neq 0 exit /b %errorlevel%


