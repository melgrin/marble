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

// raylib has `Image ImageFromImage(Image image, Rectangle rec)` and `void ImageCrop(Image* image, Rectangle crop)`, either of which I could probably use instead of this, except that they make copies of the src data instead of views/slices
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


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
// by default, raylib copmiles-out its ability to load jpg images.  there is no way to check if the version of raylib you're using has it enabled or not, because it's via source file preprocessor macros (see rtextures.c, SUPPORT_FILEFORMAT_JPG).  there is no way to change this without rebuilding all of raylib.  since I don't want to blindly use a custom raylib build, and I don't want to build raylib from source in the first place, this circumvents that part of raylib.
static Image load_image(const char* filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) {
        printf("Failed to load %s\n", filename);
        exit(1);
    }
    assert(data);
    assert(x > 0);
    assert(y > 0);

    PixelFormat format;
    if (n == 1) format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (n == 2) format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (n == 3) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (n == 4) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else assert(false);

    Image image = {
        .data = data,
        .width = x,
        .height = y,
        .mipmaps = 1, // default, according to struct definition
        .format = format,
    };

    return image;
}

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb/stb_image_write.h>

int main() {

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


    const char* filename = "local/gebco_08_rev_elev_A1_grey_geo.tif";
    GeoTIFFData topo_image_full;
    if (!geotiff_read(filename, &topo_image_full)) return 1;
    Point2i tl = geotiff_lat_lon_to_pixel(48.106075, -123.495817, topo_image_full.geo);
    Point2i br = geotiff_lat_lon_to_pixel(46.757553, -120.915573, topo_image_full.geo);
    u8* region = get_rect(topo_image_full.data, topo_image_full.width, topo_image_full.height, tl.x, tl.y, br.x, br.y);
    const int imgw = abs(br.x - tl.x);
    const int imgh = abs(br.y - tl.y);
    //if (!stbi_write_bmp("tmp.bmp", imgw, imgh, 1, region)) { printf("failed to write bmp\n"); }

    PixelFormat format;
    if (topo_image_full.bytes_per_pixel == 1) format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (topo_image_full.bytes_per_pixel == 2) format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (topo_image_full.bytes_per_pixel == 3) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (topo_image_full.bytes_per_pixel == 4) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else assert(false);

    Image topo_image = {
        .data = region,
        .width = imgw,
        .height = imgh,
        .mipmaps = 1, // default, according to struct definition
        .format = format,
    };


    Mesh mesh = GenMeshHeightmap(topo_image, (Vector3) {
        16 * ((float) topo_image.width / (float) topo_image.height),
        8, // TODO topo image elevations are scaled 0-6400 meters, according to the description on https://visibleearth.nasa.gov/images/73934/topography, but this is just a hardcoded value for now instead
        16 });
    Model model = LoadModelFromMesh(mesh);


    Image color_image_full = load_image("local/world.200405.3x21600x21600.A1.jpg");
    if (color_image_full.width != topo_image_full.width ||
        color_image_full.height != topo_image_full.height) {
        ImageResize(&color_image_full, topo_image_full.width, topo_image_full.height); // this frees the original image.data
    }
    Rectangle crop = {
        .x = (float) tl.x,
        .y = (float) tl.y,
        .width  = (float) abs(br.x - tl.x),
        .height = (float) abs(br.y - tl.y),
    };
    //ImageCrop(&color_image, crop); // this frees the original image.data
    Image color_image = ImageFromImage(color_image_full, crop); // mallocs new image, retains original

    //Texture2D texture = LoadTextureFromImage(topo_image); // TODO add key press toggle
    Texture2D texture = LoadTextureFromImage(color_image);

    Model grid_model = LoadModelFromMesh(mesh);
    grid_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIME;

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture
    Vector3 mapPosition = { 0.0f, 0.0f, 0.0f }; // Define model position

    // Unload images from RAM, already uploaded to VRAM
    //this is a region, so don't free it: // UnloadImage(topo_image);
    //UnloadImage(color_image);
    //UnloadImage(color_image_full);

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
                if (drawWires) DrawModelWiresEx(grid_model, mapPosition, rotationAxis, rotationAngle, vScale, WHITE);

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

    geotiff_free(topo_image_full);

    return 0;
}


// TODO topo and color are handled differently wrt cropping.  topo is subregioned by my get_rect function.  that function doesn't work for color right now, so instead I'm using ImageFromImage.  but ImageFromImage makes a copy.  the end goal is to reuse the same cropped buffer, probably, so ImageFromImage isn't what I want.
// TODO preprocess color image so it's already deflated and cropped.  right now it takes ~15 seconds during startup to do this.  note that nasa does provide .png, so could do that.  consider download speed/size tradeoff.
// TODO region selection - lat/lon vs which images are downloaded

