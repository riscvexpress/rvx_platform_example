#include "core_dependent.h"
#include "ervp_core_id.h"
#include "ervp_uart.h"
#include "ervp_mcom_input.h"
#include "ervp_printf.h"
#include "ervp_variable_allocation.h"

#include "platform_info.h"
#include "arducam.h"
#include "oled_rgb.h"

#ifdef INCLUDE_JPEG_ENCODER
#include "jpeg_encoder.h"
#endif

ErvpImage raw_image;

int main()
{
  if(EXCLUSIVE_ID==0)
  {
    ErvpImage *image_to_export = 0;
    char key;

    /* take picture */
    arducam_init(ARDUCAM_RGB_565_QVGA);
    arducam_alloc_image(&raw_image);

    oled_rgb_start();
    oled_rgb_clear();

    printf("Camera On\n");

    arducam_config_spi();
    arducam_single_capture(&raw_image);
    flush_cache();

    oled_rgb_config_spi();
    oled_rgb_draw_rvx_image(&raw_image);

    printf("Camera Stop\n");

#ifdef INCLUDE_JPEG_ENCODER
    while(1)
    {
      printf("types \'j\' for jpeg or \'b\' for bmp\n");
      key = getc_from_mcom();
      if((key=='b') || (key=='j'))
        break;
    }
#else
    key = 'b';
#endif

    switch(key)
    {
      case 'j':
#ifdef INCLUDE_JPEG_ENCODER
        /* convert jpeg */
        printf("convert jpeg\n");
        image_to_export = jpegenc_convert_image(&raw_image, 0);
#endif
        break;
      case 'b':
        /* convert bmp */
        printf("convert bmp\n");
        image_to_export = image_convert_format(&raw_image, 0, IMAGE_FMT_BMP);
        break;
      default:
        break;
    }
    flush_cache();
    image_prepare_dump(image_to_export);
  }
	return 0;
}
