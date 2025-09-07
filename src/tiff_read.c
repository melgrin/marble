#include <stdbool.h>
#include <assert.h>
#include <tiffio.h>

bool tiff_read(const char* filename, u8** out_data, u32* out_w, u32* out_h, u32* out_n) {
    bool ret;

    TIFF* tif = TIFFOpen(filename, "r");
    if (!tif) {
        //printf("Failed to open %s\n", filename);
        ret = false;
        goto end;
    }
    //printf("Reading %s\n", filename);

    u32 w, h;
    u16 bips, spp;
    u16 interp;
    void* mem = 0;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bips);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &interp);
    u32 bypp;
    if (interp == PHOTOMETRIC_PALETTE) {
        bypp = (u32) sizeof(uint32_t);
    } else {
        assert(bips % 8 == 0);
        bypp = bips/8 * spp;
    }
    //printf("w %u, h %u, bytes/pixel %u\n", w, h, bypp);
    if (w == 0 || h == 0 || bypp == 0) { ret = false; goto end; }

    //assert(TIFFScanlineSize(tif) == w * bypp);
    //const u32 n = w * h * bypp;
    mem = malloc(w * h * sizeof(uint32_t));
    if (!mem) { ret = false; goto end; }

    //const u16 sample = 0;// if PlanarConfiguration!=2 (single plane), there are more samples at 1+
    //u8* p = scanlines;
    //for (u32 row = 0; row < h; ++row) {
    //    if (-1 == TIFFReadScanline(tif, p, row, sample)) { ret = false; goto end; }
    //    p += w;
    //}

    int stop_on_error = 1;
    if (!TIFFReadRGBAImageOriented(tif, w, h, mem, stop_on_error, ORIENTATION_TOPLEFT)) {
        ret = false;
        goto end;
    }

    if (out_data) *out_data = mem;
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    if (out_n) *out_n = bypp;

    ret = true;
end:
    if (mem && !ret) free(mem);
    if (tif) TIFFClose(tif);
    return ret;
}
