#ifndef melgrin_marble_geotiff_h
#define melgrin_marble_geotiff_h

#include <stdbool.h>
#include <stdint.h>

#include "./common.h"

/*
   Example GeoTIFF

   From the GeoTIFF header within the image metadata:
   GeoPixelScale: 0.008333,0.008333,0.000000
   GeoTiePoints: 0.000000,0.000000,0.000000,-180.000000,90.000000,0.000000
   GeoKeyDirectory: 1,1,0,5,1024,0,1,2,1025,0,1,1,2048,0,1,4326,2052,0,1,9001,2054,0,1,9102

   image is 10800,10800
   scaled with 0.008333, that's 90,90
   in TIFF, 0,0 is top left

   -180,90 -> 0,0 ________________ -90,90 -> 10800,0
                 |                |
                 |                |
                 |                |
                 |                |
                 |                |
                 |                |
                 |________________|
   -180,0 ->  0,10800              -90,0 -> 10800,10800

*/

typedef struct {
    // geo tie points, x:lon, y:lat
    double tie_lat;
    double tie_lon;
    // geo pixel scale, x:lon, y:lat
    double scale_lat;
    double scale_lon;
} GeoTIFFMetadata;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
    uint8_t* data; // width * height * bytes_per_pixel
    GeoTIFFMetadata geo;
} GeoTIFFData;

//typedef struct {
//    int x;
//    int y;
//} Point2i;

typedef struct {
    double lat;
    double lon;
} LatLon;

bool geotiff_read(const char* filename, GeoTIFFData* img);
void geotiff_free(GeoTIFFData img);
//Point2i geotiff_lat_lon_to_pixel(double lat, double lon, GeoTIFFMetadata geo);
//LatLon geotiff_pixel_to_lat_lon(double x, double y, GeoTIFFMetadata geo);
Vec2 geotiff_lat_lon_to_x_y(double lat, double lon, GeoTIFFMetadata geo);
LatLon geotiff_x_y_to_lat_lon(double x, double y, GeoTIFFMetadata geo);



#endif // melgrin_marble_geotiff_h
