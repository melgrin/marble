#ifndef melgrin_marble_imgconv_h
#define melgrin_marble_imgconv_h

#include <stdbool.h>
#include <stdint.h>
#include "./common.h"

bool imgconv(const char* in, const char* ext, u32 width, u32 height, const char* optional_output_filename);

bool read_image(const char* filename, u8** pdata, u32* pwidth, u32* pheight, u32* pchannels);
bool write_image(const char* filename, u8* data, const u32 w, const u32 h, const u32 channels);
bool resize_image(const u8* in, const u32 inw, const u32 inh, u8** out, const u32 outw, const u32 outh, const u32 channels);

#endif // melgrin_marble_imgconv_h
