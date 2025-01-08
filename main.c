#include <stdio.h>
#include <assert.h>

#include "./geotiff.c"

#include <stdlib.h> // malloc
#include <string.h> // memcpy

#include "./typedefs.h"


#define swap(type,a,b) do { \
    type tmp = a; \
    a = b; \
    b = tmp; \
} while(0);

static void get_rect_into_buffer(u8* dst, const u8* src, u32 wsrc, u32 hsrc, u32 nsrc, u32 x0, u32 y0, u32 x1, u32 y1, u32 w, u32 h) {
    src += x0 * nsrc;
    src += wsrc * y0 * nsrc;
    u32 row;
    for (row = y0; row < y1 && row < hsrc; ++row) {
        memcpy(dst, src, w * nsrc);
        dst += w * nsrc;
        src += wsrc * nsrc;
    }
    printf("get_rect: x: %u-%u (w = %u), y: %u-%u (h = %u)\n", x0, x1, w, y0, y1, h);
    printf("row = %u, y1 = %u, hsrc = %u\n", row, y1, hsrc);
}

static u8* get_rect(const u8* src, u32 wsrc, u32 hsrc, u32 nsrc, u32 x0, u32 y0, u32 x1, u32 y1) {
    if (x0 > x1) swap(u32, x0, x1);
    if (y0 > y1) swap(u32, y0, y1);

    u32 w = x1 - x0;
    u32 h = y1 - y0;
    u8* mem = malloc(w * h * nsrc);
    assert(mem);
    get_rect_into_buffer(mem, src, wsrc, hsrc, nsrc, x0, y0, x1, y1, w, h);
    return mem;
}

#include <raylib.h>


#include "./raw_file.c"

//#define STB_IMAGE_IMPLEMENTATION // raylib includes this
//#define STBI_FAILURE_USERMSG
#include <stb_image.h>
// by default, raylib compiles-out its ability to load jpg images.  there is no way to check if the version of raylib you're using has it enabled or not, because it's via source file preprocessor macros (see rtextures.c, SUPPORT_FILEFORMAT_JPG).  there is no way to change this without rebuilding all of raylib.  since I don't want to blindly use a custom raylib build, and I don't want to build raylib from source in the first place, this circumvents that part of raylib.  [will probably reconsider the not-from-source decision in the future, because I'm currently planning to use rlImGui+dear-imgui which are both source-only libraries.  so might as well go full submodule on all the dependencies.]
//#define QOI_IMPLEMENTATION // raylib includes this
#include <qoi.h>
static Image load_image(const char* filename) {
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

    PixelFormat format;
    if (n == 1) format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (n == 2) format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (n == 3) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (n == 4) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else { assert(false); exit(1); }

    Image image = {
        .data = data,
        .width = w,
        .height = h,
        .mipmaps = 1, // default, according to struct definition
        .format = format,
    };

    return image;
}

////#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb_image_write.h>

typedef struct Vec2i { int x; int y; } Vec2i;
typedef struct Rect { int x; int y; int w; int h; } Rect;

#define Vec2Unpack(vec) vec.x, vec.y

int main() {

    const int screenWidth = 800;
    const int screenHeight = 600;

    double t0 = GetTime();
    InitWindow(screenWidth, screenHeight, "marble");
    printf("%.3f seconds for InitWindow\n", GetTime() - t0);


    const char* filename = "local/gebco_08_rev_elev_A1_grey_geo.tif";
    GeoTIFFData topo_image_full;
    if (!geotiff_read(filename, &topo_image_full)) return 1;
    const double tl_lat = 48.106075;
    const double tl_lon = -123.495817;
    const double br_lat = 46.757553;
    const double br_lon = -120.915573;
    Point2i tl = geotiff_lat_lon_to_pixel(tl_lat, tl_lon, topo_image_full.geo);
    Point2i br = geotiff_lat_lon_to_pixel(br_lat, br_lon, topo_image_full.geo);
    const int imgw = abs(br.x - tl.x);
    const int imgh = abs(br.y - tl.y);


    Camera camera = { 0 };
    camera.position = (Vector3){ (float) tl.x, 8.0f /* @Elevation */, (float) tl.y };
    camera.target = (Vector3){ (float) br.x, 8.0f /* @Elevation */, (float) br.y };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // UpdateCamera breaks if position == target
    assert(!(
        camera.position.x == camera.target.x &&
        camera.position.y == camera.target.y &&
        camera.position.z == camera.target.z));


    u8* topo_region = malloc(imgw * imgh * topo_image_full.bytes_per_pixel);
    get_rect_into_buffer(topo_region, topo_image_full.data, topo_image_full.width, topo_image_full.height, topo_image_full.bytes_per_pixel, tl.x, tl.y, br.x, br.y, imgw, imgh);

    PixelFormat format;
    if (topo_image_full.bytes_per_pixel == 1) format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (topo_image_full.bytes_per_pixel == 2) format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (topo_image_full.bytes_per_pixel == 3) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (topo_image_full.bytes_per_pixel == 4) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else assert(false);

    Image topo_image = {
        .data = topo_region,
        .width = imgw,
        .height = imgh,
        .mipmaps = 1, // default, according to struct definition
        .format = format,
    };


    Mesh mesh = GenMeshHeightmap(topo_image, (Vector3) {
        (float) imgw,
        (float) imgh / 2, // TODO @Elevation - topo image elevations are scaled 0-6400 meters, according to the description on https://visibleearth.nasa.gov/images/73934/topography, but this is just a hardcoded value for now instead
        (float) imgh });
    Model model = LoadModelFromMesh(mesh);


    Image color_image_full = load_image("local/world.200405.3x10800x10800.A1.raw");
    if (color_image_full.width != topo_image_full.width ||
        color_image_full.height != topo_image_full.height) {
        printf("Image size mismatch: color image is %ux%x, topo image is %ux%u.\n", color_image_full.width, color_image_full.height, topo_image_full.width, topo_image_full.height);
        exit(1);
    }
    if (color_image_full.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8) { // hand off to myself on this one... maybe raylib Image isn't the best format to store these in
        printf("Unexpected image format %d.  Expected %d (PIXELFORMAT_UNCOMPRESSED_R8G8B8).\n", color_image_full.format, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
        exit(1);
    }
    const int color_image_num_components = 3;

    u8* color_region = malloc(imgw * imgh * color_image_num_components);
    get_rect_into_buffer(color_region, color_image_full.data, color_image_full.width, color_image_full.height, color_image_num_components, tl.x, tl.y, br.x, br.y, imgw, imgh);
    //stbi_write_bmp("out.bmp", imgw, imgh, color_image_num_components, color_region);
    Image color_image = {
        .data = color_region,
        .width = imgw,
        .height = imgh,
        .mipmaps = 1, // default, according to struct definition
        .format = color_image_full.format,
    };

    //Texture2D texture = LoadTextureFromImage(topo_image); // TODO add key press toggle
    Texture2D texture = LoadTextureFromImage(color_image); // Convert image to texture (VRAM)

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture


    Model grid_model = LoadModelFromMesh(mesh);
    grid_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIME;


    SetTargetFPS(60);

    DisableCursor();

    bool showFloor = false;
    bool showGrid = false;
    bool drawWires = false;
    bool drawSolid = true;
    bool showImage = false;

    const Vector3 model_position = (Vector3){ (float) tl.x, 0.0f, (float) tl.y };
    const float rotationAngle = 0.0f;
    const Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 vScale = { 1.0f, 0.2f, 1.0f }; // XXX 0.2 is to scale down the vertical in the Seattle region heightmap I'm using.  There's probably a definition of what the scaling should be somewhere and/or I need to think about it more.  (The scaling is initially controlled by the 'size' Vector3 passed to GenMeshHeightmap)

    const int x0 = 590;
    const int y0 = 15;
    const int text_height = 10;
    int x = x0;
    int y = y0;

    Vec2i camera_status_title_text_position = {x, y}; y += text_height;
    Vec2i camera_status_position_text_position = {x, y}; y += text_height;
    Vec2i camera_status_target_text_position = {x, y}; y += text_height;
    Vec2i camera_status_up_text_position = {x, y}; y += text_height;
    y += text_height;
    Vec2i current_lat_text_position = {x, y}; y += text_height;
    Vec2i current_lon_text_position = {x, y}; y += text_height;

    int end_x = screenWidth - 5;
    int start_x = x0 - 10;
    int w = end_x - start_x;
    Rect border = {.x = x0 - 10, .y = y0 - 10, .w = w, .h = y};

    // Main game loop
    while (!WindowShouldClose()) {

        UpdateCamera(&camera, CAMERA_FREE);
        if (IsKeyPressed(KEY_F)) showFloor = !showFloor;
        if (IsKeyPressed(KEY_G)) showGrid = !showGrid;
        if (IsKeyDown(KEY_J)) vScale.y -= 0.1f * GetFrameTime();
        if (IsKeyDown(KEY_K)) vScale.y += 0.1f * GetFrameTime();
        if (IsKeyPressed(KEY_I)) showImage = !showImage;
        if (IsKeyPressed(KEY_ONE)) drawWires = !drawWires;
        if (IsKeyPressed(KEY_TWO)) drawSolid = !drawSolid;

        BeginDrawing();

            ClearBackground(SKYBLUE);

            BeginMode3D(camera);

                if (showFloor) DrawPlane((Vector3){0,0,0}, (Vector2){20,20}, GRAY);

                if (drawSolid) DrawModelEx(model, model_position, rotationAxis, rotationAngle, vScale, WHITE);
                if (drawWires) DrawModelWiresEx(grid_model, model_position, rotationAxis, rotationAngle, vScale, WHITE);

                if (showGrid) DrawGrid(20, 1.0f);

            EndMode3D();

            if (showImage) {
                DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
                DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);
            }


            {
                DrawRectangle     (border.x, border.y, border.w, border.h, Fade(WHITE, 0.8f));
                DrawRectangleLines(border.x, border.y, border.w, border.h, BLACK);

                DrawText("Camera status:",
                    Vec2Unpack(camera_status_title_text_position),
                    text_height,
                    BLACK);
                DrawText(TextFormat("- Position: (%6.3f, %6.3f, %6.3f)", camera.position.x, camera.position.y, camera.position.z),
                    Vec2Unpack(camera_status_position_text_position),
                    text_height,
                    BLACK);
                DrawText(TextFormat("- Target: (%6.3f, %6.3f, %6.3f)", camera.target.x, camera.target.y, camera.target.z),
                    Vec2Unpack(camera_status_target_text_position),
                    text_height,
                    BLACK);
                DrawText(TextFormat("- Up: (%6.3f, %6.3f, %6.3f)", camera.up.x, camera.up.y, camera.up.z),
                    Vec2Unpack(camera_status_up_text_position),
                    text_height,
                    BLACK);

                LatLon current = geotiff_pixel_to_lat_lon(camera.position.x, camera.position.z, topo_image_full.geo);
                DrawText(TextFormat("Current Latitude: %0.6f", current.lat),
                    Vec2Unpack(current_lat_text_position),
                    text_height,
                    BLACK);
                DrawText(TextFormat("Current Longitude: %0.6f", current.lon),
                    Vec2Unpack(current_lon_text_position),
                    text_height,
                    BLACK);
            }


            DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadTexture(texture);     // Unload texture
    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context

    geotiff_free(topo_image_full);

    return 0;
}


// TODO region selection - lat/lon vs which images are downloaded

