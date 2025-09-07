#include "./geotiff.h"
#include "./common.h"
#include <stdint.h>
#include <assert.h>
//#include <geotiff/xtiffio.h>  // for TIFF
//#include <geotiff/geotiffio.h> // for GeoTIFF
#include <tiffio.h>

// from libgeotiff/libxtiff/xtiffio.h
#define TIFFTAG_GEOPIXELSCALE       33550
#define TIFFTAG_GEOTIEPOINTS        33922
//#define TIFFTAG_GEOTRANSMATRIX      34264
//#define TIFFTAG_GEOKEYDIRECTORY     34735
//#define TIFFTAG_GEODOUBLEPARAMS     34736
//#define TIFFTAG_GEOASCIIPARAMS      34737

static bool _read_field_value(TIFF* tif,  const TIFFField* field, uint32_t* count_out, void** data_out) {
    const uint32_t tag = TIFFFieldTag(field);
    if (TIFFFieldPassCount(field)) {
        int count_size = TIFFFieldSetGetCountSize(field);
        assert(count_size > 0);
        // version issue?  using 4.7 on my laptop and it has count_size 4 for u32, and that's what the docs say too.  so where did 1 -> 16, 2 -> 32 come from (and how did it pass on the same file?  using the gebco tif from blue marble)
#if desktop__old_version
        if (count_size == 1) {
            uint16_t count;
            if (0 == TIFFGetField(tif, tag, &count, data_out)) return false;
            *count_out = (uint32_t) count;
        } else if (count_size == 2) {
            uint32_t count;
            if (0 == TIFFGetField(tif, tag, &count, data_out)) return false;
            *count_out = count;
        } else {
            assert(0);
            return false;
        }
#else
        if (count_size == 2) {
            uint16_t count;
            if (0 == TIFFGetField(tif, tag, &count, data_out)) return false;
            *count_out = (uint32_t) count;
        } else if (count_size == 4) {
            uint32_t count;
            if (0 == TIFFGetField(tif, tag, &count, data_out)) return false;
            *count_out = count;
        } else {
            assert(0);
            return false;
        }
#endif
    } else {
        //int c = TIFFFieldReadCount(field);
        //assert(c >= 0);
        //count = (uint32_t) c;
        if (0 == TIFFGetField(tif, tag, &data_out)) return false;
        *count_out = 1;
    }
    return true;
}

static bool _read_metadata(TIFF* tif, GeoTIFFMetadata* metadata_out) {
    GeoTIFFMetadata metadata = {0};

    {
        const TIFFField* field = TIFFFieldWithTag(tif, TIFFTAG_GEOPIXELSCALE);
        assert(field);

        // these are not universal truths, just what I'm expecting.  could expand if needed.  but generally expect array of floats.
        assert(TIFFFieldDataType(field) == TIFF_DOUBLE);

        uint32_t count;
        double* data;
        bool res = _read_field_value(tif, field, &count, &data);
        assert(res);
        assert(count == 3); // x,y,z
        assert(data[2] == 0); // haven't seen this non-zero before
        printf("geo pixel scale: ");
        for (uint32_t i = 0; i < count; ++i) {
            printf("%f ", data[i]);
        }
        printf("\n");

        metadata.scale_lon = data[0];
        metadata.scale_lat = data[1];
    }
    {
        const TIFFField* field = TIFFFieldWithTag(tif, TIFFTAG_GEOTIEPOINTS);
        assert(field);

        // these are not universal truths, just what I'm expecting.  could expand if needed.  but generally expect array of floats.
        assert(TIFFFieldDataType(field) == TIFF_DOUBLE);

        uint32_t count;
        double* data;
        bool res = _read_field_value(tif, field, &count, &data);
        assert(res);
        assert(count == 6); // x,y,z,lon,lat,alt

        assert(data[0] == 0); // corner of image - this is more of a "hope" assertion than an "expectation" assertion
        assert(data[1] == 0); // corner of image - this is more of a "hope" assertion than an "expectation" assertion

        assert(data[2] == 0); // haven't seen this non-zero before (tied to index 5)
        assert(data[5] == 0); // haven't seen this non-zero before (tied from index 2)

        printf("geo tie points: ");
        for (uint32_t i = 0; i < count; ++i) {
            printf("%f ", data[i]);
        }
        printf("\n");

        metadata.tie_lon = data[3];
        metadata.tie_lat = data[4];
    }

    *metadata_out = metadata;
    return true;
}

#if 0
Point2i geotiff_lat_lon_to_pixel(double lat, double lon, GeoTIFFMetadata geo) {
    double lat_tied = geo.tie_lat - lat;
    // top:    90 - 90 = 0
    // bottom: 90 - 0  = 90

    double yf = lat_tied / geo.scale_lat;
    int yi = (int) yf;

    double lon_tied = lon - geo.tie_lon;
    // left:  -180 - -180 = 0
    // right: -90  - -180 = 90

    double xf = lon_tied / geo.scale_lon;
    int xi = (int) xf;

    //printf("geo_to_pixel: lon %f -> x %ld, lat %f -> y %ld\n", lon, xi, lat, yi);

    return (Point2i){xi, yi};
}
#endif

Vec2 geotiff_lat_lon_to_x_y(double lat, double lon, GeoTIFFMetadata geo) {
    double lat_tied = geo.tie_lat - lat;
    // top:    90 - 90 = 0
    // bottom: 90 - 0  = 90

    double y = lat_tied / geo.scale_lat;

    double lon_tied = lon - geo.tie_lon;
    // left:  -180 - -180 = 0
    // right: -90  - -180 = 90

    double x = lon_tied / geo.scale_lon;

    //printf("geotiff_lat_lon_to_x_y: lon %f -> x %f, lat %f -> y %f\n", lon, x, lat, y);

    return (Vec2){x, y};
}

//LatLon geotiff_pixel_to_lat_lon(double x, double y, GeoTIFFMetadata geo)
LatLon geotiff_x_y_to_lat_lon(double x, double y, GeoTIFFMetadata geo) {
    double lat = geo.tie_lat - (y * geo.scale_lat);
    double lon = x * geo.scale_lon + geo.tie_lon;
    return (LatLon){lat, lon};
}


#include <stdlib.h> // malloc

bool geotiff_read(const char* filename, GeoTIFFData* img) {
    bool ret;

    //printf("geo_to_pixel: lon %f -> x %ld, lat %f -> y %ld\n", lon, xi, lat, yi);
    TIFF* tif = TIFFOpen(filename,"r");
    if (!tif) {
        printf("Failed to open %s\n", filename);
        ret = false;
        goto end;
    }
    printf("Reading %s\n", filename);

    //GTIF* gtif = GTIFNew(tif);
    //if (!gtif) exit(1);

    u32 w, h;
    u16 bips, spp;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bips);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    assert(bips % 8 == 0);
    u32 bypp = bips/8 * spp;
    printf("w %u, h %u, bytes/pixel %u\n", w, h, bypp);
    if (w == 0 || h == 0 || bypp == 0) { ret = false; goto end; }

    assert(TIFFScanlineSize(tif) == w * bypp);
    const u32 n = w * h * bypp;
    u8* scanlines = malloc(n);
    if (!scanlines) { ret = false; goto end; }

    const u16 sample = 0;// if PlanarConfiguration!=2 (single plane), there are more samples at 1+
    u8* p = scanlines;
    for (u32 row = 0; row < h; ++row) {
        if (-1 == TIFFReadScanline(tif, p, row, sample)) { ret = false; goto end; }
        p += w;
    }

    GeoTIFFMetadata meta;
    if (!_read_metadata(tif, &meta)) { ret = false; goto end; }

    img->width = w;
    img->height = h;
    img->bytes_per_pixel = bypp;
    img->data = scanlines;
    img->geo = meta;

    ret = true;
end:
    //if (scanlines) free(scanlines);
    //if (gtif) GTIFFree(gtif);
    if (tif) TIFFClose(tif);
    return ret;
}

void geotiff_free(GeoTIFFData img) {
    if (img.data) free(img.data);
}

