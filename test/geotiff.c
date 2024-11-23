#include <stdio.h>
#include <stdint.h>

#include "../geotiff.c"

#define expect_eq(A,B) do { \
    if ((A) == (B)) printf("Pass: %s == %s\n", #A, #B); \
    else { printf("Fail: %s = %d, %s = %d\n", #A, A, #B, B); return 1; }\
} while(0);

int test_geo_to_pixel() {
    double tl[2];
    //double br[2];
    tl[LAT] = 49.01;
    tl[LON] = -126.33;
    //br[LAT] = 45.94;
    //br[LON] = -119.36;

    const GeoInfo geo_info = {
        .tie   = { -180.0, 90.0 },
        .scale = { 0.00833333333, 0.00833333333 }
    };

    geo_to_pixel(tl[LAT], tl[LON], geo_info);


    {
        Point2i p = geo_to_pixel(90, -180, geo_info);
        expect_eq(p.x, 0);
        expect_eq(p.y, 0);
    }
    {
        Point2i p = geo_to_pixel(0, -180, geo_info);
        expect_eq(p.x, 0);
        expect_eq(p.y, 10800);
    }
    {
        Point2i p = geo_to_pixel(90, -90, geo_info);
        expect_eq(p.x, 10800);
        expect_eq(p.y, 0);
    }
    {
        Point2i p = geo_to_pixel(0, -90, geo_info);
        expect_eq(p.x, 10800);
        expect_eq(p.y, 10800);
    }

    // TODO test other quadrants.  should get the Geo info out of those tiffs.  tie points will  be different, but not sure if they'll affect anything really.  but also I might be doing the flip wrong for non-nw quadrants, because that's all I've tested so far.


    return 0;
}


int main() {
    return test_geo_to_pixel();
}

