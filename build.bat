:: only raylibdll.lib works.  the static one fails with something about glfw (sure, raylib depends on glfw, but why don't I need it for dll?).  just not understanding something about static libs on windows I suppose.
@set libs=gdi32.lib msvcrt.lib raylibdll.lib winmm.lib
cl -nologo -Z7 %libs% main.c -link -NODEFAULTLIB:libcmt
