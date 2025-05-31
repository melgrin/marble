@cd /d "%~dp0"

cl -nologo -W2 -Z7 opt.c
@if %errorlevel% neq 0 exit /b %errorlevel%

cl -nologo -W2 -Z7 ^
 -I ../deps/libtiff_config ^
 -I ../deps/libtiff/libtiff ^
 ../deps/build/libtiff.lib ^
 geotiff.c
@if %errorlevel% neq 0 exit /b %errorlevel%

opt.exe
@if %errorlevel% neq 0 exit /b %errorlevel%

geotiff.exe
@if %errorlevel% neq 0 exit /b %errorlevel%

