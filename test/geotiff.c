#include <stdio.h>
#include <stdint.h>

#include "../src/geotiff.c"

#define expect_eq(A,B) do { \
    if ((A) == (B)) printf("Pass: %s == %s\n", #A, #B); \
    else { printf("Fail: %s = %d, %s = %d\n", #A, A, #B, B); return 1; }\
} while(0);

int test_geotiff_lat_lon_to_x_y() {

    const GeoTIFFMetadata geo = {
        .tie_lat = 90.0,
        .tie_lon  = -180.0,
        .scale_lat = 0.00833333333,
        .scale_lon = 0.00833333333,
    };

    //geotiff_lat_lon_to_x_y(49.01, -126.33, geo);

    {
        Vec2 p = geotiff_lat_lon_to_x_y(90, -180, geo);
        expect_eq((int) p.x, 0);
        expect_eq((int) p.y, 0);
    }
    {
        Vec2 p = geotiff_lat_lon_to_x_y(0, -180, geo);
        expect_eq((int) p.x, 0);
        expect_eq((int) p.y, 10800);
    }
    {
        Vec2 p = geotiff_lat_lon_to_x_y(90, -90, geo);
        expect_eq((int) p.x, 10800);
        expect_eq((int) p.y, 0);
    }
    {
        Vec2 p = geotiff_lat_lon_to_x_y(0, -90, geo);
        expect_eq((int) p.x, 10800);
        expect_eq((int) p.y, 10800);
    }

    // TODO test other quadrants.  should get the Geo info out of those tiffs.  tie points will  be different, but not sure if they'll affect anything really.  but also I might be doing the flip wrong for non-nw quadrants, because that's all I've tested so far.

    return 0;
}


int main() {
    return test_geotiff_lat_lon_to_x_y();
}

