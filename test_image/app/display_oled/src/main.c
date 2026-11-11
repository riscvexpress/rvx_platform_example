#include "oled_rgb.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"
#include "ervp_printf.h"
#include "fruit_image.h"
#include "tiger_image.h"
#include "yunakim_image.h"

#define RECURSION 1

int main()
{
	if(EXCLUSIVE_ID==0)
	{
		oled_rgb_start();
		do
		{
			oled_rgb_clear();
      oled_rgb_draw_rvx_image(fruit_image);
      delay_ms(1000);
      oled_rgb_draw_rvx_image(tiger_image);
      delay_ms(1000);
      oled_rgb_draw_rvx_image(yunakim_image);
      delay_ms(1000);
		} while(RECURSION);
		oled_rgb_end();
	}
	return 0;

}
