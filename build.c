#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
//#include <sys/types.h>

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h> // spawnl
#endif

#include "./src/common.c"
#include "./src/file.c"

bool my_mkdir(const char* path) {
    printf("[info] mkdir %s\n", path);
    return create_directory(path);
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

// `system` doesn't work well on Windows if you pass forward slashes in the program name (build/bin/imgconv.exe results in system running build.exe!) // TODO check if this is a problem if using subprocess.h instead
bool my_spawn(const char* program_name, const char** args) {
#if _WIN32
    intptr_t exit_status = spawnv(P_WAIT, program_name, args);
    if (exit_status != 0) {
        fprintf(stderr, "%s subprocess failed\n", program_name);
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

#if _WIN32
static char* get_windows_error_string(int err) {
    static char buf[512]; // @ThreadSafety
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,           // source
        err,            // messageId
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // languageId
        buf,            // buffer
        sizeof(buf),    // size
        NULL);          // va_list*

    char* last = &buf[strlen(buf)-1];
    if (*last == '\n') *last = '\0';
    return buf;
}
#endif

bool sys_capture_stdout(const char* cmd, char* stdout_buf, size_t stdout_bufsize) {
    if (!cmd) return false;
    if (!stdout_buf) return false;
    memset(stdout_buf, 0, stdout_bufsize);
    if (stdout_bufsize <= 1) return false; // null terminator
#if _WIN32
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE child_out_read = NULL;
    HANDLE child_out_write = NULL;

    if (!CreatePipe(&child_out_read, &child_out_write, &sa, 0)) {
        fprintf(stderr, "Failed to create child's stdout pipes: CreatePipe error %d: %s\n", GetLastError(), get_windows_error_string(GetLastError()));
        return false;
    }
    if (!SetHandleInformation(child_out_read, HANDLE_FLAG_INHERIT, 0)) {
        fprintf(stderr, "Failed to disable 'inherit' for child's stdout output: SetHandleInformation error %d: %s\n", GetLastError(), get_windows_error_string(GetLastError()));
        return false;
    }

    STARTUPINFO startupInfo = {0};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = child_out_write;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION processInfo = {0};

    if (!CreateProcess(
        NULL,          // application name
        (char*) cmd,   // command line  // it wants non-const
        NULL,          // process security attributes
        NULL,          // primary thread security attributes
        TRUE,          // handles are inherited
        0,             // creation flags
        NULL,          // use parent's environment
        NULL,          // use parent's current directory
        &startupInfo,  // STARTUPINFO pointer
        &processInfo)  // receives PROCESS_INFORMATION
    ) {
        fprintf(stderr, "Failed to create child process: CreateProcess error %d: %s\n", GetLastError(), get_windows_error_string(GetLastError()));
        return false;
    }

    // printf("Created process '%s'\n", cmd);

    // this program hangs on ReadFile if these aren't closed first
    CloseHandle(child_out_write);

    DWORD count;
    char* cursor = stdout_buf;
    size_t rem = stdout_bufsize;
    while (1) {
        if (rem <= 0) {
            fprintf(stderr, "sys_capture_stdout: not enough size in output buffer.  Filled all %zu bytes.\n", stdout_bufsize);
            stdout_buf[stdout_bufsize-1] = '\0';
            return false;
        }
        BOOL ok = ReadFile(child_out_read, cursor, rem, &count, NULL);
        cursor += count;
        rem -= count;
        if (!ok) {
            DWORD err = GetLastError();
            if (err != ERROR_BROKEN_PIPE) { // this seems to be typical when the pipe has nothing on it, or is closed
                fprintf(stderr, "sys_capture_stdout: ReadFile error %u: %s\n", err, get_windows_error_string(err));
                return false;
            }
        }
        if (!ok || count == 0) break; // done
    }

    CloseHandle(child_out_read);

    rstrip(stdout_buf);

    return true;
#else
#error TODO
#endif // _WIN32
}

bool copy_file(const char* src, const char* dest) {
#if _WIN32
    printf("[info] copy_file: %s -> %s\n", src, dest);
    if (!CopyFile(src, dest, 0)) {
        fprintf(stderr, "CopyFile failed: Windows error %u\n", GetLastError());
        return false;
    }
    return true;
#else
#error TODO
#endif
}

// hello preprocessor string contatenation, my old friend...
int main(int argc, char** argv) {
    if (argc != 1) {
        fprintf(stderr, "Error: expected 0 arguments but got %d.\n", argc);
        return 1;
    }

    char my_dir[1024];
    if (!get_program_directory(my_dir, sizeof(my_dir))) return 1;
    if (!change_directory(my_dir)) return 1;

    // TODO? check if submodules have been cloned

    // build dependencies only if needed (+10 seconds)
    {
        if (!change_directory("deps")) return 1;

        if (!my_mkdir("build")) return 1;

#if RELEASE
#define DEPS_COMPILE_FLAGS " -nologo "
#else
#define DEPS_COMPILE_FLAGS " -nologo -Z7 "
#endif

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

            if (!sys("cl -c -I ./libtiff_config/ -Fo:./build/ " DEPS_COMPILE_FLAGS " " FILES("./libtiff/libtiff/", ".c"))) return 1;
            if (!sys("lib -nologo -out:./build/libtiff.lib " FILES("./build/", ".obj"))) return 1;

#undef FILES
        }

        if (!file_exists("build/raylib.lib")) {
#define RAYLIB_CONFIG_OVERRIDE "-I . -FI ./raylib_config/config.h"
#ifndef _MSC_VER
#error check for "force include (-FI)" equivalent on gcc - it's required.  it's how I'm replacing the pre-existing config.h in raylib repo with mine.  (I think normally they expect you to just edit it and have a dirty repo, but I like my repos clean...)
#endif

            if (!sys(
                    "cl -c " RAYLIB_CONFIG_OVERRIDE " -Fo:./build/ -I ./raylib/src/ " DEPS_COMPILE_FLAGS
                    " -I ./raylib/src/external/glfw/include -DPLATFORM_DESKTOP=1"
                    " ./raylib/src/rcore.c"
            )) return 1;

            if (!sys(
                    "cl -c " RAYLIB_CONFIG_OVERRIDE " -Fo:./build/ -I ./raylib/src/ " DEPS_COMPILE_FLAGS
                    " -DSUPPORT_FILEFORMAT_JPG" // this is a workaround to deconflict the version of stbi_load_image that's used in main via imgconv.  it needs to be able to load jpgs, but raylib disables it by default.  and I need raylib's implementation of stb_image so that raylib works.  the right solution is to use dlls to separate raylib from the rest of marble (there are a number of spots they overlap, or may in the future, like stb, qoi, glfw).  raylib tends to fork or configure the library for its own uses.  but dlls are more work, so doing this for now.  TODO.
                    " ./raylib/src/rtextures.c"
            )) return 1;

            if (!sys(
                    "cl -c " RAYLIB_CONFIG_OVERRIDE " -Fo:./build/ -I ./raylib/src/ " DEPS_COMPILE_FLAGS
                    " ./raylib/src/rshapes.c"
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
                    "cl -c -MT -EHsc"
                    " -D IMGUI_DISABLE_OBSOLETE_FUNCTIONS" // required otherwise DebugCheckVersionAndDataLayout fails: "Assertion failed: sz_io == sizeof(ImGuiIO) && "Mismatched struct layout!", file imgui\imgui.cpp, line 10390".
                    " -D CIMGUI_NO_EXPORT" // static linking, so no need for this.  otherwise, creates .lib and .exp alongside .exe.
                    " -D NO_FONT_AWESOME" // rlImGui - don't think I want extra fonts right now, no matter how awesome they are.
                    " -I ./cimgui/imgui"
                    " -I ./cimgui"
                    " -I ./rlImGui"
                    " -I ./raylib/src"
                    " -wd5287" // ImGuiHoveredFlags
                    " -Fo:./build/"
                    DEPS_COMPILE_FLAGS
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

        if (!change_directory(my_dir)) return 1;
    }

    if (!my_mkdir("build")) return 1;
    if (!my_mkdir("build/bin")) return 1;
    if (!my_mkdir("build/obj")) return 1;

#if RELEASE
#define MAIN_COMPILE_FLAGS " -nologo -I src "
#else
#define MAIN_COMPILE_FLAGS " -nologo -I src -Z7 "
#endif

#define MAIN_WARN_FLAGS " -W3 -WX -wd4996 -wd4101 "

    if (!sys("cl -c -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " src/common.c"
    )) return 1;

    if (!sys("cl -c -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " -I deps/stb"
        " -I deps/qoi"
        " -I deps/libtiff_config"
        " -I deps/libtiff/libtiff"
        " src/imgconv.c"
    )) return 1;

    if (!sys("cl -c -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " src/file.c"
    )) return 1;

    if (!sys("cl -c -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " src/logger.c"
    )) return 1;

    char version[64];
    if (!sys_capture_stdout("git describe --always --dirty", version, sizeof(version))) return false;

    char build_main[1024];
    snprintf(build_main, sizeof(build_main),
        "cl -Fe:build/bin/marble.exe -Fo:build/obj/ "
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " -wd5287" // ImGuiHoveredFlags
        " -I deps/stb"
        " -D VERSION=\\\"%s\\\""
        " -I deps/qoi"
        " -I deps/raylib/src"
        " -I deps/libtiff_config"
        " -I deps/libtiff/libtiff"
        " -I deps/cimgui"
        " -I deps/rlImGui"
        " build/obj/common.obj"
        " build/obj/imgconv.obj"
        " build/obj/file.obj"
        " build/obj/logger.obj"
        // raylib.lib needs to come before user32.lib, otherwise there's a symbol clash with "CloseWindow".
        " deps/build/raylib.lib deps/build/libtiff.lib deps/build/imgui.lib"
        " gdi32.lib msvcrt.lib winmm.lib user32.lib shell32.lib" 
        " src/main.c"
        " -link -NODEFAULTLIB:libcmt"
        , version
    );

    if (!sys(build_main)) return 1;


    if (!sys("cl -c -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        " -W2 -WX"
        " -I deps/qoi"
        " -TC" // force .c
        " -D QOI_IMPLEMENTATION"
        " deps/qoi/qoi.h"
    )) return 1;

    const char* build_imgconv = 
        "cl -Fe:build/bin/imgconv.exe -Fo:build/obj/"
        MAIN_COMPILE_FLAGS
        MAIN_WARN_FLAGS
        " -D MAIN"
        " -I deps/stb"
        " -I deps/qoi"
        " -I deps/libtiff_config"
        " -I deps/libtiff/libtiff"
        " build/obj/common.obj"
        " build/obj/file.obj"
        " build/obj/qoi.obj"
        " deps/build/libtiff.lib"
        " src/imgconv.c"
        ;

    if (!sys(build_imgconv)) return 1;


#if 0
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

#endif


    // build and run tests
    {
        if (!change_directory("test")) return 1;

        if (!sys("cl -nologo -W2 -Z7 common.c")) return 1;

        if (!sys("cl -nologo -W2 -Z7 opt.c")) return 1;

        if (!sys(
            "cl -nologo -W2 -Z7"
            " -I ../deps/libtiff_config"
            " -I ../deps/libtiff/libtiff"
            " ../deps/build/libtiff.lib"
            " geotiff.c"
        )) return 1;

        if (!sys("common")) return 1;

        if (!sys("opt")) return 1;

        if (!sys("geotiff")) return 1;
    }

    // install
    {
        if (!change_directory(my_dir)) return 1;

        if (!my_mkdir("install")) return 1;
        if (!my_mkdir("install/bin")) return 1;
        if (!my_mkdir("install/data")) return 1;
        if (!my_mkdir("install/data/bmng")) return 1;
        if (!my_mkdir("install/data/topo")) return 1;

        if (!copy_file("README.md", "install/README.md")) return 1;
        if (!copy_file("LICENSE.txt", "install/LICENSE.txt")) return 1;
        
        const char* bmng = "install/data/bmng/A1.jpg";
        if (!file_exists(bmng)) {
            const char* src = "deps/marble_data/bmng/world.200405.3x21600x21600.A1.jpg";
            const char* imgconv = "build/bin/imgconv";
            const char* args[] = {imgconv, "jpg", src, "--width", "10800", "--height", "10800", "--output", bmng, NULL};
            if (!my_spawn(imgconv, args)) return 1;
        }

        if (!copy_file("deps/marble_data/topo/gebco_08_rev_elev_A1_grey_geo.tif", "install/data/topo/A1.tif")) return 1;

        if (!copy_file("build/bin/marble.exe", "install/bin/marble.exe")) return 1;
    }

    return 0;
}

// TODO experiment with rebuilding this exe, somehow.  can't overwrite currently-executing .exe on Windows, at least not easily.  if it seems not possible or too complicated, maybe just check build.c timestamp/cksum against one baked into the exe, and warn/error saying it needs to be rebuilt

