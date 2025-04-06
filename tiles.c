/*

 1  ...
 2 .xxx.
 3 .xOx.
 4 .xxx.
 5  ... 

 1  ... 
 2  ...
 3 .xxx.
 4 .xOx.
 5 .xxx.

 // [x] make and track 3x3 visible tile indices, centered around camera grid cell
 // [x] display visible tile indices - yellow grid lines above world
 // [x] loading
 //     - diff prev vs cur
 //     - if -1, unload
 //     - if +1, load
 // [x] prev = cur (swap?)
 // [x] actual load/unload


*/

#define TILE_X_COUNT   3
#define TILE_Y_COUNT   3
#define TILE_COUNT     (TILE_X_COUNT * TILE_Y_COUNT)
#define TILE_POOL_SIZE (TILE_COUNT * 2) // double so that the whole tile set can move at once

static_assert(TILE_X_COUNT % 2 != 0, "Need odd number of tile columns, so that there's a center one.");
static_assert(TILE_Y_COUNT % 2 != 0, "Need odd number of tile rows, so that there's a center one.");

typedef struct {
    Mesh mesh;
    Model model;
    Model grid_model;
    Texture topo_texture;
    Texture color_texture;
    Vector3 model_position;
} TileData;

typedef struct {
    Vec2i index;

    u8* topo_buffer;
    u8* color_buffer;

    Image topo_image;
    Image color_image;

    // these are recreated any time the tile grid changes
    int data_serial;
    TileData data;
} Tile;

typedef struct {
    int tilew; // tile width in pixels
    int tileh; // tile height in pixels

    Tile pool[TILE_POOL_SIZE];

    // the tile grid
    Vec2i visible[TILE_COUNT]; // x, y tile index, corresponds to Tile.index
    Tile* visible_tiles[TILE_COUNT]; // pointer to a tile in pool

    int current_serial;

    Logger* logger;

} Tiles;


void tiles_init(
    Tiles* tiles,
    int tilew,
    int tileh,
    int topo_num_components,
    int color_num_components,
    Logger* logger
) {
    if (!tiles) return;
    memset(tiles, 0, sizeof(*tiles));

    tiles->tilew = tilew;
    tiles->tileh = tileh;
    tiles->current_serial = 1;
    tiles->logger = logger;

    PixelFormat topo_image_format;
    if (topo_num_components == 1) topo_image_format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (topo_num_components == 2) topo_image_format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (topo_num_components == 3) topo_image_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (topo_num_components == 4) topo_image_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else { assert(false); return; }

    PixelFormat color_image_format;
    if (color_num_components == 1) color_image_format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    else if (color_num_components == 2) color_image_format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    else if (color_num_components == 3) color_image_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (color_num_components == 4) color_image_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    else { assert(false); return; }

    for (int i = 0; i < arraylen(tiles->pool); ++i) {
        Tile* t = &tiles->pool[i];
        t->topo_buffer  = malloc(tilew * tileh * topo_num_components);
        t->color_buffer = malloc(tilew * tileh * color_num_components);

        t->topo_image = (Image){
            .data = t->topo_buffer,
            .width = tilew,
            .height = tileh,
            .mipmaps = 1, // default, according to struct definition
            .format = topo_image_format,
        };
        t->color_image = (Image){
            .data = t->color_buffer,
            .width = tilew,
            .height = tileh,
            .mipmaps = 1, // default, according to struct definition
            .format = color_image_format,
        };
    }
}

static void _tiles_load_one(Tile* tile, Vec2i index, Img topo_full, Img color_full, int tilew, int tileh, int* current_serial) {

    TileData* data = &tile->data;

    int tl_x = index.x * tilew;
    int tl_y = index.y * tileh;
    int br_x = (index.x + 1) * tilew;
    int br_y = (index.y + 1) * tileh;

    get_rect_into_buffer(tile->topo_buffer,
        topo_full.data, topo_full.w, topo_full.h, topo_full.n,
        tl_x, tl_y, br_x, br_y, tilew, tileh);
    get_rect_into_buffer(tile->color_buffer,
        color_full.data, color_full.w, color_full.h, color_full.n,
        tl_x, tl_y, br_x, br_y, tilew, tileh);

    data->mesh = GenMeshHeightmap(tile->topo_image, (Vector3) { // "RAM and VRAM" // XXX do I need to unload this?
        (float) tilew,
        (float) tileh / 2, // TODO @Elevation - topo image elevations are scaled 0-6400 meters, according to the description on https://visibleearth.nasa.gov/images/73934/topography, but this is just a hardcoded value for now instead
        (float) tileh });

    data->model = LoadModelFromMesh(data->mesh);
    data->grid_model = LoadModelFromMesh(data->mesh);

    assert(tile->topo_image.data == tile->topo_buffer);
    assert(tile->color_image.data == tile->color_buffer);
    data->topo_texture = LoadTextureFromImage(tile->topo_image); // VRAM
    data->color_texture = LoadTextureFromImage(tile->color_image); // VRAM

    //data->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = data->color_texture; // this is set in tiles_update
    data->grid_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIME;

    data->model_position = (Vector3){ (float) tl_x, 0.0f, (float) tl_y };

    tile->data_serial = *current_serial;
    *current_serial += 1;
    if (*current_serial > 1000000000) *current_serial = 1;

    tile->index = index;
}

static void _tiles_unload_one(Tile* tile) {
    assert(tile->data_serial > 0);
    if (tile->data_serial > 0) {
        //UnloadMesh(tile->data.mesh); // seems like I should do this, but it crashes.  does UnloadModel also free the mesh?  or something?
        UnloadModel(tile->data.model);
        //UnloadModel(tile->data.grid_model); // FIXME why tho
        UnloadTexture(tile->data.topo_texture);
        UnloadTexture(tile->data.color_texture);
        tile->data_serial *= -1;

        memset(&tile->data, 0, sizeof(tile->data)); // this isn't really necessary - should always be checking if data_serial > 0
    }
}

static int _tiles_get_first_available_index(Tiles* tiles) {
    for (int i = 0; i < arraylen(tiles->pool); ++i) {
        if (tiles->pool[i].data_serial <= 0) {
            return i;
        }
    }
    return -1;
}

void tiles_update(Tiles* tiles, int current_tile_x_index, int current_tile_y_index, Img topo_full, Img color_full, bool useTopo) {

    for (int y = 0; y < TILE_Y_COUNT; ++y) {
        for (int x = 0; x < TILE_X_COUNT; ++x) {
            int i = y * TILE_Y_COUNT + x;
            static const int center_x_index = TILE_X_COUNT / 2;
            static const int center_y_index = TILE_Y_COUNT / 2;

            // y distance from center
            // top row    = y - center = 0 - 1 = -1
            // middle row = y - center = 1 - 1 =  0
            // bottom row = y - center = 2 - 1 =  1

            // x distance from center
            // left column   = x - center = 0 - 1 = -1
            // middle column = x - center = 1 - 1 =  0
            // right column  = x - center = 2 - 1 =  1

            int x_distance_from_center = x - center_x_index;
            int y_distance_from_center = y - center_y_index;
            tiles->visible[i].x = current_tile_x_index + x_distance_from_center;
            tiles->visible[i].y = current_tile_y_index + y_distance_from_center;
        }
    }

    // unload tiles that are loaded but not needed
    for (int i = 0; i < arraylen(tiles->pool); ++i) {
        bool found = false;
        for (int j = 0; j < arraylen(tiles->visible); ++j) {
            if (tiles->pool[i].index.x == tiles->visible[j].x  &&
                tiles->pool[i].index.y == tiles->visible[j].y ) {
                found = true;
                break;
            }
        }
        if (!found) {
            Tile* tile = &tiles->pool[i];
            if (tile->data_serial > 0) {
                log_info(tiles->logger, "[tiles] unloaded: x %d, y %d, serial %d, pool index %d, color buffer 0x%x, pos (%.1f, %.1f, %.1f)\n",
                    tile->index.x, tile->index.y, tile->data_serial, i, tile->color_buffer, Vec3Unpack(tile->data.model_position));
                _tiles_unload_one(tile);
                //should probably do this, but would be nicer if it was a stub instead of null pointer:
                //tiles->visible_tiles[j] = 0;
            }
        }
    }

    // load tiles that are needed but not loaded
    for (int i = 0; i < arraylen(tiles->visible); ++i) {
        bool found = false;
        for (int j = 0; j < arraylen(tiles->pool); ++j) {
            if (tiles->visible[i].x == tiles->pool[j].index.x &&
                tiles->visible[i].y == tiles->pool[j].index.y) {
                found = true;
                break;
            }
        }
        if (!found) {
            int pool_index = _tiles_get_first_available_index(tiles);
            if (pool_index >= 0) {
                Tile* tile = &tiles->pool[pool_index];
                _tiles_load_one(tile, tiles->visible[i], topo_full, color_full, tiles->tilew, tiles->tileh, &tiles->current_serial);
                assert(tiles->visible[i].x == tile->index.x);
                assert(tiles->visible[i].y == tile->index.y);
                tiles->visible_tiles[i] = tile;
                log_info(tiles->logger, "[tiles] loaded: x %d, y %d, serial %d, pool index %d, visible index %d, color buffer 0x%x, pos (%.1f, %.1f, %.1f)\n",
                    tile->index.x, tile->index.y, tile->data_serial, pool_index, i, tile->color_buffer, Vec3Unpack(tile->data.model_position));
            }
        }
    }

    // reassign tiles that are staying (accounts for moves)
    for (int i = 0; i < arraylen(tiles->visible); ++i) {
        for (int j = 0; j < arraylen(tiles->pool); ++j) {
            if (tiles->visible[i].x == tiles->pool[j].index.x &&
                tiles->visible[i].y == tiles->pool[j].index.y) {
                Tile* old = tiles->visible_tiles[i];
                Tile* new = &tiles->pool[j];
                if (old != new) {
                    tiles->visible_tiles[i] = new;
                    log_info(tiles->logger, "[tiles] reassigned: pool index %d, visible index %d\n"
                        "[tiles]    old: x %d, y %d, serial %d, color buffer 0x%x, pos (%.1f, %.1f, %.1f)\n"
                        "[tiles]    new: x %d, y %d, serial %d, color buffer 0x%x, pos (%.1f, %.1f, %.1f)\n",
                        j, i,
                        old->index.x, old->index.y, old->data_serial, old->color_buffer, Vec3Unpack(old->data.model_position),
                        new->index.x, new->index.y, new->data_serial, new->color_buffer, Vec3Unpack(new->data.model_position));
                }
            }
        }
    }

    // choose texture
    for (int i = 0; i < arraylen(tiles->visible); ++i) {
        TileData* data = &tiles->visible_tiles[i]->data;
        data->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = useTopo ? data->topo_texture : data->color_texture;
    }

}

// 10800 makes my computer freeze up
// 10800/2 makes my computer freeze up
// 10800/4 makes my computer freeze up
//const int tilew = 10800/10;
//const int tileh = 10800/10;

