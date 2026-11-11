#ifndef __YUNAKIM_IMAGE_H__
#define __YUNAKIM_IMAGE_H__

#include "ervp_image.h"

extern const unsigned char* yunakim_image_array;
extern const int yunakim_image_width;
extern const int yunakim_image_height;
extern const int yunakim_image_byte_size;
#ifndef GENERATE_HEX_USING_GCC
extern ErvpImage* yunakim_image;
#endif

#endif