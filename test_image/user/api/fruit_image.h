#ifndef __FRUIT_IMAGE_H__
#define __FRUIT_IMAGE_H__

#include "ervp_image.h"

extern const unsigned char* fruit_image_array;
extern const int fruit_image_width;
extern const int fruit_image_height;
extern const int fruit_image_byte_size;
#ifndef GENERATE_HEX_USING_GCC
extern ErvpImage* fruit_image;
#endif

#endif