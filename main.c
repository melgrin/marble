// Started life as raylib/examples/models/models_heightmap.c

#include <assert.h>

#include <geotiff/xtiffio.h>  // for TIFF

#include "./geotiff.c"

#include <stdlib.h> // malloc
#include <string.h> // memcpy

#include "./typedefs.h"


#define swap(type,a,b) do { \
    type tmp = a; \
    a = b; \
    b = tmp; \
} while(0);

static u8* get_rect(const u8* src, u32 wsrc, u32 hsrc, u32 x0, u32 y0, u32 x1, u32 y1) {
    if (x0 > x1) swap(u32, x0, x1);
    if (y0 > y1) swap(u32, y0, y1);

    u32 w = x1 - x0;
    u32 h = y1 - y0;
    u8* mem = malloc(w * h);
    assert(mem);
    u8* dst = mem;
    src += x0;
    src += wsrc * y0;
    u32 row;
    for (row = y0; row < y1 && row < hsrc; ++row) {
        memcpy(dst, src, w);
        dst += w;
        src += wsrc;
    }
    printf("get_rect: x: %u-%u (w = %u), y: %u-%u (h = %u)\n", x0, x1, w, y0, y1, h);
    printf("row = %u, y1 = %u, hsrc = %u\n", row, y1, hsrc);
    return mem;
}

#include <raylib/raylib.h>

#ifdef LOAD_GEOTIFF
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif

int main(int argc, char** argv) {

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");

    Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 8.0f, 18.0f };      // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    //const char* filename = argc > 1 ? argv[1] : "resources/heightmap.png";

    // https://visibleearth.nasa.gov/grid
    // https://visibleearth.nasa.gov/collection/1484/blue-marble
    // https://visibleearth.nasa.gov/collection/1723/visible-earth

    // https://visibleearth.nasa.gov/images/73934/topography
    // "Data in these images were scaled 0-6400 meters."

    // https://visibleearth.nasa.gov/images/147190/explorer-base-map
    // https://visibleearth.nasa.gov/images/144898/earth-at-night-black-marble-2016-color-maps



#if LOAD_GEOTIFF
    const char* filename = "local/gebco_08_rev_elev_A1_grey_geo.tif"; // nocheckin
    GeoTIFFData img;
    if (!geotiff_read(filename, &img)) return 1;
    Point2i tl = geotiff_lat_lon_to_pixel(48.106075, -123.495817, img.geo);
    Point2i br = geotiff_lat_lon_to_pixel(46.757553, -120.915573, img.geo);
    u8* region = get_rect(img.data, img.width, img.height, tl.x, tl.y, br.x, br.y);
    int imgw = abs(br.x - tl.x);
    int imgh = abs(br.y - tl.y);
    if (!stbi_write_bmp("tmp.bmp", imgw, imgh, 1, region)) { printf("failed to write bmp\n"); }

    //Image topo_image = LoadImageFromMemory(".bmp", region, imgw * imgh * img.bytes_per_pixel);
    Image topo_image = {
        .data = region,
        .width = imgw,
        .height = imgh,
        .mipmaps = 1, // default, according to struct definition
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, // XXX this is only because I know the image I'm loading is grayscale (gebco_08_rev_elev_A1_grey_geo.tif)
    };

#else
    Image topo_image = LoadImage("resources/sea_region_topo.png"); // Load heightmap image (RAM)
#endif
    //Texture2D texture = LoadTextureFromImage(image); // Convert image to texture (VRAM)

    // Generate heightmap mesh (RAM and VRAM)
    Mesh mesh = GenMeshHeightmap(topo_image, (Vector3) {
        16 * ((float) topo_image.width / (float) topo_image.height),
        8, // TODO topo image elevations are scaled 0-6400 meters
        16 });
    Model model = LoadModelFromMesh(mesh);

    //Image color_image = LoadImage("resources/sea_region_color.jpg");
    Image color_image = LoadImage("resources/sea_region_color.png"); // "Data format not supported" w/jpg, and needed to resize it anyway (though could use ImageResize for that)
#if LOAD_GEOTIFF
    Texture2D texture = LoadTextureFromImage(topo_image);
#else
    Texture2D texture = LoadTextureFromImage(color_image);
#endif

    Model solid_color_model_for_grid = LoadModelFromMesh(mesh);
    solid_color_model_for_grid.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIME;

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture
    Vector3 mapPosition = { 0.0f, 0.0f, 0.0f }; // Define model position

    // Unload images from RAM, already uploaded to VRAM
    UnloadImage(topo_image);
    UnloadImage(color_image);

    SetTargetFPS(60);

    int cameraMode = CAMERA_FREE;
    if (cameraMode == CAMERA_FREE) DisableCursor();

    bool showFloor = false;
    bool showGrid = false;
    bool drawWires = false;
    bool drawSolid = true;
    bool showImage = false;

    const float rotationAngle = 0.0f;
    const Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 vScale = { 1.0f, 0.2f, 1.0f }; // XXX 0.2 is to scale down the vertical in the Seattle region heightmap I'm using.  There's probably a definition of what the scaling should be somewhere and/or I need to think about it more.  (The scaling is initially controlled by the 'size' Vector3 passed to GenMeshHeightmap)

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

                if (drawSolid) DrawModelEx(model, mapPosition, rotationAxis, rotationAngle, vScale, WHITE);
                if (drawWires) DrawModelWiresEx(solid_color_model_for_grid, mapPosition, rotationAxis, rotationAngle, vScale, WHITE);

                if (showGrid) DrawGrid(20, 1.0f);

            EndMode3D();

            if (showImage) {
                DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
                DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);
            }

            DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadTexture(texture);     // Unload texture
    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context

#if LOAD_GEOTIFF
    geotiff_free(img);
#endif

    return 0;
}

