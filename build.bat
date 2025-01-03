@if not exist build mkdir build
@if not exist build\bin mkdir build\bin
@if not exist build\obj mkdir build\obj

@set libs=gdi32.lib msvcrt.lib raylibdll.lib winmm.lib
cl -nologo -W2 -Z7 -Fe:build/bin/ -Fo:build/obj/ ^
 -I deps/stb ^
 -I deps/qoi ^
 -I deps/libtiff ^
 -I deps/libtiff/libtiff/libtiff ^
 %* ^
 %libs% ^
 main.c ^
 deps/libtiff/build/*.obj ^
 -link -NODEFAULTLIB:libcmt

@if %errorlevel% neq 0 exit /b %errorlevel%

cl -nologo -Z7 -Fe:build/bin/ -Fo:build/obj/ -I deps/stb stbi_to_raw.c
cl -nologo -Z7 -Fe:build/bin/ -Fo:build/obj/ -I deps/stb raw_to_png.c
cl -nologo -Z7 -Fe:build/bin/ -Fo:build/obj/ -I deps/stb jpg_to_png.c
cl -nologo -Z7 -Fe:build/bin/ -Fo:build/obj/ -I deps/stb -I deps/qoi jpg_to_qoi_resize.c

@if %errorlevel% neq 0 exit /b %errorlevel%

@if not exist local mkdir local

@if not exist local\world.200405.3x21600x21600.A1.raw (
    pushd local
    ..\build\bin\stbi_to_raw.exe world.200405.3x21600x21600.A1.jpg
    popd
)

@if not exist local\world.200405.3x21600x21600.A1.resized.qoi (
    pushd local
    ..\build\bin\jpg_to_qoi_resize.exe world.200405.3x21600x21600.A1.jpg
    popd
)

@if %errorlevel% neq 0 exit /b %errorlevel%
