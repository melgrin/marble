#ifndef melgrin_marble_imgconv_h
#define melgrin_marble_imgconv_h

#include <stdbool.h>
#include <stdint.h>

bool imgconv(const char* in, const char* ext, uint32_t width, uint32_t height, const char* optional_output_filename);

#endif // melgrin_marble_imgconv_h
