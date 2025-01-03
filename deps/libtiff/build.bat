:: note: there doesn't seem to be a way to exclude the write portions of the library during compilation.  would need to modify source.
:: TODO: disable more of the internal compression schemes in tiffconf.h and then remove the corresponding source files here.

if not exist build mkdir build

cl /nologo /c /I . /Fo:build/ ^
 libtiff/libtiff/tif_open.c ^
 libtiff/libtiff/tif_win32.c ^
 libtiff/libtiff/tif_close.c ^
 libtiff/libtiff/tif_dir.c ^
 libtiff/libtiff/tif_strip.c ^
 libtiff/libtiff/tif_read.c ^
 libtiff/libtiff/tif_version.c ^
 libtiff/libtiff/tif_dirread.c ^
 libtiff/libtiff/tif_swab.c ^
 libtiff/libtiff/tif_compress.c ^
 libtiff/libtiff/tif_tile.c ^
 libtiff/libtiff/tif_hash_set.c ^
 libtiff/libtiff/tif_aux.c ^
 libtiff/libtiff/tif_error.c ^
 libtiff/libtiff/tif_warning.c ^
 libtiff/libtiff/tif_codec.c ^
 libtiff/libtiff/tif_dirinfo.c ^
 libtiff/libtiff/tif_next.c ^
 libtiff/libtiff/tif_lzw.c ^
 libtiff/libtiff/tif_luv.c ^
 libtiff/libtiff/tif_thunder.c ^
 libtiff/libtiff/tif_fax3.c ^
 libtiff/libtiff/tif_fax3sm.c ^
 libtiff/libtiff/tif_write.c ^
 libtiff/libtiff/tif_packbits.c ^
 libtiff/libtiff/tif_dumpmode.c ^
 libtiff/libtiff/tif_predict.c ^
 libtiff/libtiff/tif_flush.c ^
 libtiff/libtiff/tif_dirwrite.c
@if %errorlevel% neq 0 exit /b %errorlevel%

:: cl /nologo /c /I . /I include ..\main.c
:: @if %errorlevel% neq 0 exit /b %errorlevel%
:: 
:: link /nologo /out:mono_main.exe ^
::  tif_open.obj ^
::  tif_win32.obj ^
::  tif_close.obj ^
::  tif_dir.obj ^
::  tif_strip.obj ^
::  tif_read.obj ^
::  tif_version.obj ^
::  tif_dirread.obj ^
::  tif_swab.obj ^
::  tif_compress.obj ^
::  tif_tile.obj ^
::  tif_hash_set.obj ^
::  tif_aux.obj ^
::  tif_error.obj ^
::  tif_warning.obj ^
::  tif_codec.obj ^
::  tif_dirinfo.obj ^
::  tif_next.obj ^
::  tif_lzw.obj ^
::  tif_luv.obj ^
::  tif_thunder.obj ^
::  tif_fax3.obj ^
::  tif_fax3sm.obj ^
::  tif_write.obj ^
::  tif_packbits.obj ^
::  tif_dumpmode.obj ^
::  tif_predict.obj ^
::  tif_flush.obj ^
::  tif_dirwrite.obj ^
::  main.obj
:: @if %errorlevel% neq 0 exit /b %errorlevel%
:: 
:: .\mono_main.exe
:: @if %errorlevel% neq 0 exit /b %errorlevel%
