#include "ervp_printf.h"
#include "ervp_delay.h"
#include "ervp_user_gpio.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"

int main() {
	int value;
	user_gpio_set_input_cfg(GPIO_INDEX_FOR_SWITCH_SLIDE1, 0);
	while(1)
	{
		value = get_slide_switch(0);
		value |= user_gpio_get_input(GPIO_INDEX_FOR_SWITCH_SLIDE1)<<1;
		printf("read_data %03b\n", value);
		delay_ms(get_test_period_ms());
	}
	return 0;
}
