// Started life as raylib/examples/models/models_heightmap.c

#include <raylib/raylib.h>

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

    Image topo_image = LoadImage("resources/sea_region_topo.png"); // Load heightmap image (RAM)
    //Texture2D texture = LoadTextureFromImage(image); // Convert image to texture (VRAM)

    // Generate heightmap mesh (RAM and VRAM)
    Mesh mesh = GenMeshHeightmap(topo_image, (Vector3) {
        16 * ((float) topo_image.width / (float) topo_image.height),
        8, // TODO topo image elevations are scaled 0-6400 meters
        16 });
    Model model = LoadModelFromMesh(mesh);

    //Image color_image = LoadImage("resources/sea_region_color.jpg");
    Image color_image = LoadImage("resources/sea_region_color.png"); // "Data format not supported" w/jpg, and needed to resize it anyway (though could use ImageResize for that)
    Texture2D texture = LoadTextureFromImage(color_image); // Convert image to texture (VRAM)

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

    return 0;
}
