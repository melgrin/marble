#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
//#include <sys/types.h>

#if _WIN32
#include <direct.h> // mkdir
#include <process.h> // spawnl
#endif

bool my_mkdir(const char* path) {
    printf("[info] mkdir %s\n", path);
    struct stat buf;
    errno = 0;
    int res = stat(path, &buf);
    if (errno == ENOENT) {
        res = mkdir(path);
        if (res == -1) {
            fprintf(stderr, "Error: failed to create directory '%s': %s\n", path, strerror(errno));
            return false;
        }
        return true;
    } else if (res == -1) {
        fprintf(stderr, "Error: failed to get information for '%s': %s\n", path, strerror(errno));
        return false;
    } else {
        if (buf.st_mode & S_IFDIR) {
            return true;
        }
        fprintf(stderr, "Error: wanted to make directory '%s', but it already exists as a file.\n", path);
        return false;
    }
}

bool file_exists(const char* path) {
    printf("[info] file_exists? %s\n", path);
    struct stat buf;
    errno = 0;
    int res = stat(path, &buf);
    if (errno == ENOENT) {
        return false;
    } else if (res == -1) {
        fprintf(stderr, "Error: failed to get information for '%s': %s\n", path, strerror(errno));
        return false;
    } else {
        if (buf.st_mode & S_IFREG) {
            return true;
        } else {
            return false;
        }
    }
}

bool my_dirname(const char* path, char* buf, size_t bufsize) {
    printf("[info] dirname %s\n", path);
    const char* fs = strrchr(path, '/');
    const char* bs = strrchr(path, '\\');
    const char* end;
    if (fs && bs) end = fs > bs ? fs : bs;
    else if (fs) end = fs;
    else if (bs) end = bs;
    else end = path;
    size_t len = end - path;
    if (len == 0) {
        memset(buf, 0, bufsize);
        return true;
    } else if (len + 1 <= bufsize) {
        strncpy(buf, path, len);
        buf[len] = '\0';
        return true;
    } else {
        return false;
    }
}

// `system` doesn't work well on Windows if you pass forward slashes in the program name (build/bin/imgconv.exe results in system running build.exe!)
bool my_spawn(const char* program_name, const char** args) {
#if _WIN32
    intptr_t exit_status = spawnv(P_WAIT, program_name, args);
    if (exit_status != 0) {
        fprintf(stderr, "%s subproces failed\n", program_name);
        return false;
    }
    return true;
#else
#error my_spawn unimplemented on this platform
#endif
}

bool sys(const char* cmd) {
    printf("[info] system %s\n", cmd);
    int res = system(cmd);
    if (res == 0) {
        return true;
    } else {
        printf("command failed: %s\n", cmd);
        return false;
    }
}

bool my_chdir(const char* path) {
    printf("[info] chdir %s\n", path);
    if (-1 == chdir(path)) {
        fprintf(stderr, "Error: failed to change directories to '%s': %s\n", path, strerror(errno));
        return false;
    }
    return true;
}

// hello preprocessor string contatenation, my old friend...
int main(int argc, char** argv) {
    if (argc != 1) {
        fprintf(stderr, "Error: expected 0 arguments but got %d.\n", argc);
        return 1;
    }

    char my_dir[1024];
    if (!my_dirname(argv[0], my_dir, sizeof(my_dir))) return 1;

    if (strlen(my_dir)) {
        if (!my_chdir(my_dir)) return 1;
    }

    // TODO? check if submodules have been cloned

    // build dependencies only if needed (+10 seconds)
    {
        if (!my_chdir("deps")) return 1;

        if (!my_mkdir("build")) return 1;

#define DEPS_COMPILE_FLAGS "-Z7"

        if (!file_exists("build/libtiff.lib")) {
            // note: there doesn't seem to be a way to exclude the write portions of the library during compilation.  would need to modify source.
            // TODO: disable more of the internal compression schemes in tiffconf.h and then remove the corresponding source files here.  the tiffs I'm loading are lzw, so I don't need luv, fax3, and maybe others.
#define FILES(D, X) \
            D "tif_open"      X " " \
            D "tif_win32"     X " " \
            D "tif_close"     X " " \
            D "tif_dir"       X " " \
            D "tif_strip"     X " " \
            D "tif_read"      X " " \
            D "tif_version"   X " " \
            D "tif_dirread"   X " " \
            D "tif_swab"      X " " \
            D "tif_compress"  X " " \
            D "tif_tile"      X " " \
            D "tif_hash_set"  X " " \
            D "tif_aux"       X " " \
            D "tif_error"     X " " \
            D "tif_warning"   X " " \
            D "tif_codec"     X " " \
            D "tif_dirinfo"   X " " \
            D "tif_next"      X " " \
            D "tif_lzw"       X " " \
            D "tif_luv"       X " " \
            D "tif_thunder"   X " " \
            D "tif_fax3"      X " " \
            D "tif_fax3sm"    X " " \
            D "tif_write"     X " " \
            D "tif_packbits"  X " " \
            D "tif_dumpmode"  X " " \
            D "tif_predict"   X " " \
            D "tif_flush"     X " " \
            D "tif_dirwrite"  X " " \
            D "tif_getimage"  X " " \
            D "tif_color"     X " " \
            ""

            if (!sys("cl -nologo -c -I ./libtiff_config/ -Fo:./build/ " DEPS_COMPILE_FLAGS " " FILES("./libtiff/libtiff/", ".c"))) return 1;
            if (!sys("lib -nologo -out:./build/libtiff.lib " FILES("./build/", ".obj"))) return 1;

#undef FILES
        }

        if (!file_exists("build/raylib.lib")) {
#define RAYLIB_CONFIG_OVERRIDE "-I . -FI ./raylib_config/config.h"
#ifndef _MSC_VER
#error check for "force include (-FI)" equivalent on gcc - it's required.  it's how I'm replacing the pre-existing config.h in raylib repo with mine.  (I think normally they expect you to just edit it and have a dirty repo, but I like my repos clean...)
#endif

            if (!sys(
                    "cl -nologo -c " RAYLIB_CONFIG_OVERRIDE " -Fo:./build/ -I ./raylib/src/ " DEPS_COMPILE_FLAGS
                    " -I ./raylib/src/external/glfw/include -DPLATFORM_DESKTOP=1"
                    " ./raylib/src/rcore.c"
            )) return 1;

            if (!sys(
                    "cl -nologo -c " RAYLIB_CONFIG_OVERRIDE " -Fo:./build/ -I ./raylib/src/ " DEPS_COMPILE_FLAGS
                    " ./raylib/src/rshapes.c"
                    " ./raylib/src/rtextures.c"
                    " ./raylib/src/rtext.c"
                    " ./raylib/src/rmodels.c"
                    " ./raylib/src/utils.c"
                    " ./raylib/src/rglfw.c"
            )) return 1;

            if (!sys(
                    "lib -nologo -out:./build/raylib.lib"
                    " ./build/rcore.obj"
                    " ./build/rshapes.obj"
                    " ./build/rtextures.obj"
                    " ./build/rtext.obj"
                    " ./build/rmodels.obj"
                    " ./build/utils.obj"
                    " ./build/rglfw.obj"
            )) return 1;
        }

        if (!file_exists("build/imgui.lib")) {
            if (!sys(
                    "cl -nologo -c -MT -EHsc"
                    " -D IMGUI_DISABLE_OBSOLETE_FUNCTIONS" // required otherwise DebugCheckVersionAndDataLayout fails: "Assertion failed: sz_io == sizeof(ImGuiIO) && "Mismatched struct layout!", file imgui\imgui.cpp, line 10390".
                    " -D CIMGUI_NO_EXPORT" // static linking, so no need for this.  otherwise, creates .lib and .exp alongside .exe.
                    " -D NO_FONT_AWESOME" // rlImGui - don't think I want extra fonts right now, no matter how awesome they are.
                    " -I ./cimgui/imgui"
                    " -I ./cimgui"
                    " -I ./rlImGui"
                    " -I ./raylib/src"
                    " -Fo:./build/"
                    " " DEPS_COMPILE_FLAGS
                    " ./cimgui/imgui/imgui.cpp"
                    " ./cimgui/imgui/imgui_demo.cpp"
                    " ./cimgui/imgui/imgui_draw.cpp"
                    " ./cimgui/imgui/imgui_tables.cpp"
                    " ./cimgui/imgui/imgui_widgets.cpp"
                    " ./cimgui/cimgui.cpp"
                    " ./rlImGui/rlImGui.cpp"
            )) return 1;

            if (!sys(
                    "lib -nologo -out:./build/imgui.lib"
                    " ./build/imgui.obj"
                    " ./build/imgui_demo.obj"
                    " ./build/imgui_draw.obj"
                    " ./build/imgui_tables.obj"
                    " ./build/imgui_widgets.obj"
                    " ./build/cimgui.obj"
                    " ./build/rlImGui.obj"
            )) return 1;
        }

        if (!my_chdir("..")) return 1;
    }

    if (!my_mkdir("build")) return 1;
    if (!my_mkdir("build/bin")) return 1;
    if (!my_mkdir("build/obj")) return 1;

    const char* build_main = 
        "cl -nologo -W2 -Z7 -Fe:build/bin/marble.exe -Fo:build/obj/ "
        " -I deps/stb"
        " -I deps/qoi"
        " -I deps/raylib/src"
        " -I deps/libtiff_config"
        " -I deps/libtiff/libtiff"
        " -I deps/cimgui"
        " -I deps/rlImGui"
        // raylib.lib needs to come before user32.lib, otherwise there's a symbol clash with "CloseWindow".
        " deps/build/raylib.lib deps/build/libtiff.lib deps/build/imgui.lib"
        " gdi32.lib msvcrt.lib winmm.lib user32.lib shell32.lib" 
        " main.c"
        " -link -NODEFAULTLIB:libcmt"
        ;

    if (!sys(build_main)) return 1;


    const char* build_imgconv = 
        "cl -nologo -Z7 -Fe:build/bin/ -Fo:build/obj/"
        " -I deps/stb"
        " -I deps/qoi"
        " -I deps/libtiff_config"
        " -I deps/libtiff/libtiff"
        " deps/build/libtiff.lib"
        " imgconv.c"
        ;

    if (!sys(build_imgconv)) return 1;

    my_mkdir("local");

#define w0 "21600"
#define h0 "21600"
#define w1 "10800"
#define h1 "10800"
#define bmng_jpg "./deps/marble_data/bmng/world.200405.3x"w0"x"h0".A1.jpg"
#define bmng_raw "./local/world.200405.3x"w1"x"h1".A1.raw"

    // generating this file is slow, so only do it if necessary (+20 seconds)
    if (!file_exists(bmng_raw)) {
        if (!file_exists(bmng_jpg)) {
            fprintf(stderr, "Error: missing image data.  Make sure that git submodules have been initialized.  Looking for %s but it does not exist.", bmng_jpg);
            return 1;
        }
        const char* imgconv = "build/bin/imgconv";
        const char* args[] = {imgconv, "raw", bmng_jpg, "--width", w1, "--height", h1, "--output", bmng_raw, NULL};
        if (!my_spawn(imgconv, args)) return 1;
    }


    // build and run tests
    {
        if (!my_chdir("test")) return 1;

        if (!sys("cl -nologo -W2 -Z7 opt.c")) return 1;

        if (!sys(
            "cl -nologo -W2 -Z7"
            " -I ../deps/libtiff_config"
            " -I ../deps/libtiff/libtiff"
            " ../deps/build/libtiff.lib"
            " geotiff.c"
        )) return 1;

        if (!sys("opt")) return 1;

        if (!sys("geotiff")) return 1;
    }


    return 0;
}

// TODO experiment with rebuilding this exe, somehow.  can't overwrite currently-executing .exe on Windows, at least not easily.  if it seems not possible or too complicated, maybe just check build.c timestamp/cksum against one baked into the exe, and warn/error saying it needs to be rebuilt

