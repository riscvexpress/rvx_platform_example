#include "ervp_printf.h"
#include "ervp_external_peri_group_memorymap_offset.h"
#include "ervp_platform_controller_api.h"
#include "ervp_user_gpio.h"
#include "ervp_interrupt.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// This app is designed for the "test_gpio" platform.

void gpio_isr0(void)
{
	printf("gpio 0\n");
	delay_ms(get_test_period_ms() / 2);
}

void gpio_isr1(void)
{
	printf("gpio 1: %1d\n", user_gpio_get_input(GPIO_INDEX_FOR_SWITCH_SLIDE1));
	delay_ms(get_test_period_ms() / 2);
}

int main()
{
	if (EXCLUSIVE_ID == 0)
	{
		user_gpio_enable_interrupt(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0, ERVP_TRIGGER_COND_RISE, 0);
		register_isr_gpio(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0, gpio_isr0, 1);

		user_gpio_set_input_cfg(GPIO_INDEX_FOR_SWITCH_SLIDE1, 0);
		user_gpio_enable_interrupt(GPIO_INDEX_FOR_SWITCH_SLIDE1, ERVP_TRIGGER_COND_HIGH | ERVP_TRIGGER_COND_EQ, 2);
		register_isr_gpio(GPIO_INDEX_FOR_SWITCH_SLIDE1, gpio_isr1, 1);

		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();

		printf("test gpio\n");
		while (1)
		{
		};
	}
	return 0;
}
