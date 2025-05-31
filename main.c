#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // malloc
#include <string.h> // memcpy
#include <math.h> // floor

#include <raylib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include <cimgui.h>

#define NO_FONT_AWESOME
#include <rlImGui.h>

//#define STB_IMAGE_IMPLEMENTATION // raylib defines this
//#define STBI_FAILURE_USERMSG
#include <stb_image.h>

//#define QOI_IMPLEMENTATION // raylib defines this
#include <qoi.h>

#include "./common.c"
#include "./geotiff.c"
#include "./raw_file.c"
#include "./camera.c"
#include "./world_to_screen.c"
#include "./logger.c"
#include "./tiles.c"


static Img load_image(const char* filename) {
    double t0 = GetTime();

    const char* ext = strrchr(filename, '.');
    if (!ext) exit(1);

    u8* data;
    u32 w, h, n;
    if (0 == strcmp(ext, ".raw")) {
        RawImageInfo info;
        if (!raw_read(filename, &data, &info)) {
            printf("Failed to load %s\n", filename);
            exit(1);
        }
        w = info.width;
        h = info.height;
        n = info.channels;
    } else if (0 == strcmp(ext, ".qoi")) {
        qoi_desc desc;
        void* mem = qoi_read(filename, &desc, 0);
        if (!mem) {
            printf("Failed to load %s\n", filename);
            exit(1);
        }
        data = mem;
        w = desc.width;
        h = desc.height;
        n = desc.channels;
    } else {
        int x, y, comp;
        data = stbi_load(filename, &x, &y, &comp, 0);
        if (!data) {
            printf("Failed to load %s: %s\n", filename, stbi_failure_reason());
            exit(1);
        }
        assert(x > 0);
        assert(y > 0);
        w = (u32) x;
        h = (u32) y;
        n = (u32) comp;
    }

    double elapsed = GetTime() - t0;
    printf("loaded %s in %.3f seconds; %u x %u, num channels = %u\n", filename, elapsed, w, h, n);

    assert(n == 1 || n == 2 || n == 3 || n == 4);

    Img image = {
        .data = data,
        .w = w,
        .h = h,
        .n = n,
    };

    return image;
}


// in my brain, z is up.  in raylib, y is up.  this function converts from my brain to raylib.
static void draw_line_3d(int x0, int y0, int z0, int x1, int y1, int z1, Color color) {
    DrawLine3D((Vector3){(float) x0, (float) z0, (float) y0}, (Vector3){(float) x1, (float) z1, (float) y1}, color);
}

static void draw_grid_3d(int x, int y, int z, int w, int h, int xstep, int ystep, Color color) {
    for (int i = 0; i <= h; i += ystep) { // across
        draw_line_3d(x, y+i, z, x+w, y+i, z, color);
    }
    for (int i = 0; i <= w; i += xstep) { // down
        draw_line_3d(x+i, y, z, x+i, y+h, z, color);
    }
}


static Vec2i world_to_screen(int x, int y, int z, Camera camera) {
    Vector3 pos = {(float) x, (float) z, (float) y};
    Vector2 v = _GetWorldToScreen(pos, camera);
    return (Vec2i){(int) v.x, (int) v.y};
}

static void draw_text(const char* text, int x, int y, int text_height, Color color) {
    if (x >= 0 && y >= 0) {
        DrawText(text, x, y, text_height, color);
    }
}

int main() {
    assert(igDebugCheckVersionAndDataLayout(igGetVersion(), sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx)));

    const int screenWidth = 800;
    const int screenHeight = 600;

    double t0 = GetTime();
    InitWindow(screenWidth, screenHeight, "marble");
    printf("%.3f seconds for InitWindow\n", GetTime() - t0);
    SetExitKey(KEY_NULL); // prevent Escape from closing window

    Logger logger = {0};
    logger.file = fopen("local/log.txt", "ab");
    log_info(&logger, "\nsession start\n");

    const char* color_image_filename = "deps/marble_data/topo/gebco_08_rev_elev_A1_grey_geo.tif";
    GeoTIFFData topo_image_full;
    if (!geotiff_read(color_image_filename, &topo_image_full)) return 1;
    printf("geo tie lat: %f\ngeo tie lon: %f\ngeo scale lat: %f\ngeo scale lon %f\n",
        topo_image_full.geo.tie_lat, topo_image_full.geo.tie_lon,
        topo_image_full.geo.scale_lat, topo_image_full.geo.scale_lon);


    // ok with cull distance 1000 and 10000
    const int tilew = 200;
    const int tileh = 200;

    Vector2 tl = {0, 0};
    Vector2 br = {tl.x + tilew, tl.y + tileh};

    const int TILE_X_INDEX_MAX = 10800 / tilew;
    const int TILE_Y_INDEX_MAX = 10800 / tileh;
    int tile_x_index = 0;
    int tile_y_index = 0;

    Camera camera = { 0 };
    //camera.position = (Vector3){ (float) (tl.x + tilew/2), 8.0f /* @Elevation */, (float) (tl.y + tileh/2) };
    camera.position = (Vector3){ (float) (10800/2), 8.0f /* @Elevation */, (float) (10800/2) };
    camera.target = (Vector3){ br.x, 8.0f /* @Elevation */, br.y };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60; // 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // UpdateCamera breaks if position == target
    assert(!(
        camera.position.x == camera.target.x &&
        camera.position.y == camera.target.y &&
        camera.position.z == camera.target.z));


    const char* topo_image_filename = "local/world.200405.3x10800x10800.A1.raw";
    Img color_image_full = load_image(topo_image_filename);
    if (color_image_full.w != topo_image_full.width ||
        color_image_full.h != topo_image_full.height) {
        printf("Image size mismatch: color image is %ux%u, topo image is %ux%u.\n", color_image_full.w, color_image_full.h, topo_image_full.width, topo_image_full.height);
        exit(1);
    }
    if (color_image_full.n != 3) {
        printf("Unexpected image format %d.  Expected 3.\n", color_image_full.n);
        exit(1);
    }

    Img _topo_full = {
        .data = topo_image_full.data,
        .w = topo_image_full.width,
        .h = topo_image_full.height,
        .n = topo_image_full.bytes_per_pixel,
    };
    Img _color_full = {
        .data = color_image_full.data,
        .w = color_image_full.w,
        .h = color_image_full.h,
        .n = color_image_full.n,
    };


    const int target_fps = 60;
    SetTargetFPS(target_fps);

    rlImGuiSetup(true);

    DisableCursor();
    bool ui_focused = false;
    ImGuiWindowFlags debug_window_flags = ImGuiWindowFlags_NoInputs;

    bool useTopo = false;
    bool drawTerrain = true;
    bool drawWires = false;
    bool drawTileDebug = false;
    bool showImGuiDemoWindow = false;
    bool showImage = false;

    typedef struct {
        bool* flag;
        KeyboardKey key;
        const char* description;
        const char* tooltip;
    } KeyboardShortcut;
    KeyboardShortcut keyboard_shortcuts[] = {
        {&useTopo,             KEY_T,    "useTopo (T)", "Use topographic data instead of satellite imagery for terrain colors"},
        {&drawTerrain,         KEY_ONE,  "drawTerrain (1)"},
        {&drawWires,           KEY_TWO,  "drawWires (2)", "Draw terrain wireframe"},
        {&drawTileDebug,       KEY_NULL, "drawTileDebug"},
        {&showImGuiDemoWindow, KEY_NULL, "showImGuiDemoWindow"},
    };
    const size_t keyboard_shortcuts_len = arraylen(keyboard_shortcuts);
    KeyboardShortcut debug_window_key = {NULL, KEY_ESCAPE, "Escape"};

    Vector3 model_position = (Vector3){ tl.x, 0.0f, tl.y };
    const float rotationAngle = 0.0f;
    const Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 vScale = { 1.0f, 0.2f, 1.0f }; // XXX 0.2 is to scale down the vertical in the Seattle region heightmap I'm using.  There's probably a definition of what the scaling should be somewhere and/or I need to think about it more.  (The scaling is initially controlled by the 'size' Vector3 passed to GenMeshHeightmap)

    Tiles tiles;
    tiles_init(&tiles, tilew, tileh, _topo_full.n, _color_full.n, &logger);

    LatLon prev_latlon = {NAN, NAN};
    bool ui_focused_prev = ui_focused;
    bool first_frame = true;
    bool second_frame = false;
    const char* debug_window = "Debug";

    while (!WindowShouldClose()) {

        if (IsKeyPressed(debug_window_key.key) && !igGetIO()->WantCaptureKeyboard) {
            ui_focused = !ui_focused;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !igGetIO()->WantCaptureMouse) {
            ui_focused = false;
        }

        if (second_frame || ui_focused != ui_focused_prev) { // second frame because in the first frame, imgui/rlImGui does some first time init that re-enables the mouse cursor and takes focus
            if (ui_focused) {
                EnableCursor();
                igSetWindowFocus_Str(debug_window);
                debug_window_flags &= ~ImGuiWindowFlags_NoInputs;
            } else {
                DisableCursor();
                igSetWindowFocus_Nil();
                debug_window_flags |= ImGuiWindowFlags_NoInputs;
            }
        }

        if (!ui_focused) {
            UpdateCamera_custom(&camera, CAMERA_FREE);
            if (IsKeyDown(KEY_J)) vScale.y -= 0.1f * GetFrameTime();
            if (IsKeyDown(KEY_K)) vScale.y += 0.1f * GetFrameTime();
            for (size_t i = 0; i < keyboard_shortcuts_len; ++i) {
                KeyboardShortcut* ks = &keyboard_shortcuts[i];
                if (ks->key != KEY_NULL && ks->flag != NULL && IsKeyPressed(ks->key)) {
                    *ks->flag = !*ks->flag;
                }
            }
        }

        LatLon current_latlon = geotiff_x_y_to_lat_lon(camera.position.x, camera.position.z, topo_image_full.geo);
        bool moved = !first_frame && 0 != memcmp(&current_latlon, &prev_latlon, sizeof(LatLon));

        int derived_tile_x_index = (int) floor(camera.position.x / tilew);
        int derived_tile_y_index = (int) floor(camera.position.z / tileh);
        tile_x_index = clamp(derived_tile_x_index, 0, TILE_X_INDEX_MAX);
        tile_y_index = clamp(derived_tile_y_index, 0, TILE_Y_INDEX_MAX);

        tiles_update(&tiles, tile_x_index, tile_y_index, _topo_full, _color_full, useTopo);

        BeginDrawing();

            ClearBackground(SKYBLUE);

            BeginMode3D(camera);

                for (int i = 0; i < arraylen(tiles.visible_tiles); ++i) {
                    Tile* tile = tiles.visible_tiles[i];
                    if (tile) {
                        if (drawTerrain) {
                            DrawModelEx(tile->data.model, tile->data.model_position, rotationAxis, rotationAngle, vScale, WHITE);
                        }
                        if (drawWires) {
                            DrawModelWiresEx(tile->data.grid_model, tile->data.model_position, rotationAxis, rotationAngle, vScale, WHITE);
                        }
                    }
                }

                if (drawTileDebug) {

                    DrawLine3D((Vector3){camera.position.x, 4.0f, camera.position.z}, (Vector3){0.0f, 4.0f, 0.0f}, RED);

                    DrawLine3D((Vector3){tl.x, 0.0f, tl.y}, (Vector3){tl.x, 20.0f, tl.y}, DARKBLUE);
                    DrawLine3D((Vector3){br.x, 0.0f, br.y}, (Vector3){br.x, 20.0f, br.y}, DARKBLUE);

                    for (float x = br.x; x >= 0; x -= tilew) {
                        DrawLine3D((Vector3){x, 0, br.y}, (Vector3){x, 0, 0}, BLACK);
                    }
                    for (float y = br.y; y >= 0; y -= tileh) {
                        DrawLine3D((Vector3){br.x, 0, y}, (Vector3){0, 0, y}, BLACK);
                    }

                    for (float x = tl.x; x <= topo_image_full.width; x += tilew) {
                        DrawLine3D((Vector3){x, 0, br.y}, (Vector3){x, 0, (float)topo_image_full.height}, RED);
                    }
                    for (float y = tl.y; y <= topo_image_full.height; y += tileh) {
                        DrawLine3D((Vector3){br.x, 0, y}, (Vector3){(float)topo_image_full.width, 0, y}, RED);
                    }

                    draw_grid_3d(tiles.visible_tiles[0]->index.x * tilew, tiles.visible_tiles[0]->index.y * tileh, 200, tilew * 3, tileh * 3, tilew, tileh, YELLOW);

                    DrawCubeWires((Vector3){
                            (float) tiles.visible_tiles[4]->index.x * tilew + tilew/2,
                            0,
                            (float) tiles.visible_tiles[4]->index.y * tileh + tileh/2
                        },
                        (float) tilew, camera.position.y * 2, (float) tileh,
                        BLUE
                    );

                }


            EndMode3D();

            if (showImage) {
                //DrawTexture(*texture, screenWidth - texture->width - 20, 20, WHITE); // TODO link with tile useTopo
                //DrawRectangleLines(screenWidth - texture->width - 20, 20, texture->width, texture->height, GREEN);
            }

            {
                static const int pad_y = 3;
                static const int text_y = 3; // top
                static const int text_height = 10;
                int fps = GetFPS();
                DrawRectangle(0, 0, screenWidth, text_y + text_height + pad_y, Fade(BLACK, 0.8f));
                DrawText(TextFormat("%2d", fps),
                    5,
                    text_y,
                    text_height,
                    fps < target_fps/4 ? RED : fps < target_fps/2 ? ORANGE : WHITE);
                DrawText(TextFormat("%0.6f", current_latlon.lat),
                    screenWidth - 130,
                    text_y,
                    text_height,
                    WHITE);
                DrawText(TextFormat("%0.6f", current_latlon.lon),
                    screenWidth - 70,
                    text_y,
                    text_height,
                    WHITE);
            }

            if (drawTileDebug) {
                static const int text_height = 10;

                for (int i = 0; i < arraylen(tiles.visible_tiles); ++i) {
                    Tile* tile = tiles.visible_tiles[i];
                    Vec2i v = world_to_screen(
                        tile->index.x * tilew + tilew/2,
                        tile->index.y * tileh + tileh/2,
                        200,
                        camera);
                    DrawRectangle(v.x-5, v.y, 200, text_height, BLACK);
                    draw_text(
                        TextFormat("i %d: (x %d, y %d): serial %d, cbuf %x", i, tile->index.x, tile->index.y, tile->data_serial, tile->color_buffer),
                        v.x, v.y,
                        text_height,
                        WHITE);

                    bool overlap = false;
                    int overlapping_serial = -1;
                    for (int j = 0; j < arraylen(tiles.visible_tiles); ++j) {
                        Tile* tile2 = tiles.visible_tiles[j];
                        if (tile->data_serial == tile2->data_serial) continue;
                        if (tile->color_buffer == tile2->color_buffer) {
                            overlap = true;
                            overlapping_serial = tile2->data_serial;
                            break;
                        }
                    }
                    DrawRectangle(v.x-5, v.y - text_height, 200, text_height, BLACK);
                    draw_text(
                        TextFormat("%x (%d)", tile->color_buffer, overlapping_serial),
                        v.x, v.y - text_height,
                        text_height,
                        overlap ? RED : WHITE);
                }
            }


            rlImGuiBegin();

            if (showImGuiDemoWindow) {
                igShowDemoWindow(&showImGuiDemoWindow);
            }

            static const ImVec2 debug_window_size = {.x = 340, .y = 300};
            igSetNextWindowSize(debug_window_size, ImGuiCond_Once);
            igSetNextWindowPos((ImVec2){10, 50}, ImGuiCond_Once, (ImVec2){0, 0});
            if (igBegin(debug_window, 0, debug_window_flags)) {

                igText("Press %s to select this window", debug_window_key.description);

                if (igTreeNodeEx_Str("Position", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen)) {

                    static const float input_width = 90;

                    {
                        static LatLon new;
                        static bool initialized = false;
                        if (moved || !initialized) {
                            new = current_latlon;
                            initialized = true;
                        }
                        igSetNextItemWidth(input_width);
                        igInputDouble("Latitude", &new.lat, 0, 0, "%.6f", ImGuiInputTextFlags_None);
                        igSetNextItemWidth(input_width);
                        igInputDouble("Longitude", &new.lon, 0, 0, "%.6f", ImGuiInputTextFlags_None);
                        bool go_button_enabled = 0 != memcmp(&current_latlon, &new, sizeof(LatLon));
                        igBeginDisabled(!go_button_enabled);
                        bool go_button_pressed = igButton("Go to Lat/Lon", (ImVec2){0,0});
                        igEndDisabled();
                        if (go_button_pressed) {
                            Vec2 pos = geotiff_lat_lon_to_x_y(new.lat, new.lon, topo_image_full.geo);
                            camera.position.x = (float) pos.x;
                            camera.position.z = (float) pos.y;
                        }
                    }

                    igSeparator();

                    {
                        static int new_x_index;
                        static int new_y_index;
                        static bool initialized = false;
                        if (moved || !initialized) {
                            new_x_index = tile_x_index;
                            new_y_index = tile_y_index;
                            initialized = true;
                        }
                        static char x_format[16] = {0};
                        static char y_format[16] = {0};
                        if (!x_format[0]) snprintf(x_format, sizeof(x_format), "%%d / %d", TILE_X_INDEX_MAX);
                        if (!y_format[0]) snprintf(y_format, sizeof(y_format), "%%d / %d", TILE_Y_INDEX_MAX);
                        igSetNextItemWidth(input_width);
                        igSliderInt("Tile x index", &new_x_index, 0, TILE_X_INDEX_MAX, x_format, ImGuiSliderFlags_AlwaysClamp);
                        if (new_x_index != derived_tile_x_index) {
                            igSameLine(0, -1);
                            igText("(%d)", derived_tile_x_index);
                        }
                        igSetNextItemWidth(input_width);
                        igSliderInt("Tile y index", &new_y_index, 0, TILE_Y_INDEX_MAX, y_format, ImGuiSliderFlags_AlwaysClamp);
                        if (new_y_index != derived_tile_y_index) {
                            igSameLine(0, -1);
                            igText("(%d)", derived_tile_y_index);
                        }
                        bool go_button_enabled = new_x_index != tile_x_index || new_y_index != tile_y_index;
                        igBeginDisabled(!go_button_enabled);
                        bool go_button_pressed = igButton("Go to tile x/y index", (ImVec2){0,0});
                        igEndDisabled();
                        if (go_button_pressed) {
                            camera.position.x = new_x_index * tilew + tilew/2.0;
                            camera.position.z = new_y_index * tileh + tileh/2.0;
                        }
                    }

                    igTreePop();
                }

                if (igTreeNodeEx_Str("Options", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                    for (size_t i = 0; i < keyboard_shortcuts_len; ++i) {
                        KeyboardShortcut* ks = &keyboard_shortcuts[i];
                        if (ks->description != NULL && ks->flag != NULL) {
                            igCheckbox(ks->description, ks->flag);
                            if (ks->tooltip) {
                                igSameLine(0, -1);
                                igTextDisabled("(?)");
                                igSetItemTooltip(ks->tooltip);
                            }
                        }
                    }
                    igTreePop();
                }

                if (igTreeNodeEx_Str("Terrain Scale", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                    igSetNextItemWidth(debug_window_size.x / 2);
                    igSliderFloat("##Terrain Scale", &vScale.y, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);
                    igTreePop();
                }

                if (igTreeNodeEx_Str("Images", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                    if (igBeginTable("##ImageTable", 3, ImGuiTableFlags_None, (ImVec2){0,0}, 0)) {
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 0, 0);
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 0, 0);
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthStretch, 0, 0);
                        igTableNextColumn(); igText("Topo");
                        igTableNextColumn(); igText("%ux%ux%u", _topo_full.w, _topo_full.h, _topo_full.n);
                        igTableNextColumn(); igText(topo_image_filename);
                        igTableNextColumn(); igText("Color");
                        igTableNextColumn(); igText("%ux%ux%u", _color_full.w, _color_full.h, _color_full.n);
                        igTableNextColumn(); igText(color_image_filename);
                        igEndTable();
                    }
                    igTreePop();
                }

                if (igTreeNodeEx_Str("Camera", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                    if (igBeginTable("##CameraTable", 4, ImGuiTableFlags_None, (ImVec2){0,0}, 0)) {
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 60, 0);
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 60, 0);
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 60, 0);
                        igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 60, 0);
                        igTableNextColumn(); igText("Position");
                        igTableNextColumn(); igText("%.3f", camera.position.x);
                        igTableNextColumn(); igText("%.3f", camera.position.y);
                        igTableNextColumn(); igText("%.3f", camera.position.z);
                        igTableNextColumn(); igText("Target");
                        igTableNextColumn(); igText("%.3f", camera.target.x);
                        igTableNextColumn(); igText("%.3f", camera.target.y);
                        igTableNextColumn(); igText("%.3f", camera.target.z);
                        igTableNextColumn(); igText("Up");
                        igTableNextColumn(); igText("%.3f", camera.up.x);
                        igTableNextColumn(); igText("%.3f", camera.up.y);
                        igTableNextColumn(); igText("%.3f", camera.up.z);
                        igEndTable();
                    }
                    igTreePop();
                }

            }
            igEnd();

            rlImGuiEnd();

        EndDrawing();

        prev_latlon = current_latlon;
        ui_focused_prev = ui_focused;
        second_frame = first_frame;
        first_frame = false;
    }

    CloseWindow();

    geotiff_free(topo_image_full);

    return 0;
}


// RAM: LoadImage
// VRAM: LoadTextureFromImage
// RAM and VRAM: GenMeshHeightmap
// so...no optimizing memory/pre-fetching then I guess?
