@if not exist build mkdir build
@if not exist build\bin mkdir build\bin
@if not exist build\obj mkdir build\obj

:: only raylibdll.lib works.  the static one fails with something about glfw (sure, raylib depends on glfw, but why don't I need it for dll?).  just not understanding something about static libs on windows I suppose.
:: TODO think I just need gdi32.lib in this libs list to do static linking.  and maybe opengl32.lib
:: @set libs=gdi32.lib msvcrt.lib raylibdll.lib winmm.lib geotiff_i.lib tiff.lib
@set libs=gdi32.lib msvcrt.lib raylibdll.lib winmm.lib tiff.lib
cl -nologo -W2 -Z7 /Fe:build/bin/ /Fo:build/obj/ %* %libs% main.c -link -NODEFAULTLIB:libcmt

@if %errorlevel% neq 0 exit /b %errorlevel%

cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ stbi_to_raw.c
cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ raw_to_png.c
cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ jpg_to_png.c
cl -nologo -Z7 /Fe:build/bin/ /Fo:build/obj/ jpg_to_qoi_resize.c

@if %errorlevel% neq 0 exit /b %errorlevel%

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
