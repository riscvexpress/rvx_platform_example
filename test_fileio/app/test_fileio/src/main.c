#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_fakefile.h"
#include "ervp_image.h"
#include "ervp_image_file.h"

int main() {
  if(EXCLUSIVE_ID==0)
  {
    ErvpImage* tiger_image_rgb888;

    FAKEFILE *src, *dst;
    src = ffopen("tiger_small.jpg", "r");
    dst = ffopen("tiger_small_2.jpg", "w");
    fakefile_copy(src, dst);
    ffclose(src);
    ffclose(dst);
    src = ffopen("tiger_small_2.jpg", "r");
    dst = ffopen("tiger_small_3.jpg", "w");
    fakefile_copy(src, dst);
    ffclose(src);
    ffclose(dst);
    
    // image file format conversion
    tiger_image_rgb888 = image_file_read("tiger_small.jpg", NULL);
    image_file_write(tiger_image_rgb888, "tiger_small_4.bmp", 80);
  }
	return 0;
}
