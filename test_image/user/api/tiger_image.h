#ifndef __TIGER_IMAGE_H__
#define __TIGER_IMAGE_H__

#include "ervp_image.h"

extern const unsigned char* tiger_image_array;
extern const int tiger_image_width;
extern const int tiger_image_height;
extern const int tiger_image_byte_size;
#ifndef GENERATE_HEX_USING_GCC
extern ErvpImage* tiger_image;
#endif

#endif