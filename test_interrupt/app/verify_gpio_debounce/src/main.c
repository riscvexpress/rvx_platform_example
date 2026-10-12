#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_gpio.h"
#include "ervp_user_gpio.h"
#include "ervp_interrupt.h"
#include "ervp_core_id.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"

// This app is designed for the "test_interrupt" platform.
// The testbench (user/sim/include/sim_user_region.vh) waits until switch_slide1 has
// debouncing enabled, presses switch_slide_readymade_0 NUM_PRESS times with contact
// bounce, and then drives switch_slide1 = 3 as the end marker.
// With working debouncing, each press raises exactly one rising-edge interrupt.

#define NUM_PRESS 3

static volatile int num_rise = 0;

void gpio_isr0(void)
{
	num_rise++;
}

static int debounce_enabled(unsigned int index)
{
	return (gpio_get_config(get_user_gpio_addr(index)) & GPIO_CONFIG_DEBOUNCE_ENABLE) != 0;
}

int main()
{
	if (EXCLUSIVE_ID == 0)
	{
		// init_switch() in the platform init enables debouncing on the ready-made switches
		assert_must_msg(debounce_enabled(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0), "debounce is not enabled by init_switch()");

		user_gpio_enable_interrupt(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0, ERVP_TRIGGER_COND_RISE, 0);
		register_isr_gpio(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0, gpio_isr0, 1);
		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();

		printf("waiting for %d bouncy presses\n", NUM_PRESS);

		// enabling debouncing on switch_slide1 starts the testbench stimulus
		user_gpio_set_input_cfg(GPIO_INDEX_FOR_SWITCH_SLIDE1, GPIO_CONFIG_DEBOUNCE_ENABLE);
		assert_must_msg(debounce_enabled(GPIO_INDEX_FOR_SWITCH_SLIDE1), "user_gpio_set_input_cfg() ignores its config");

		while (user_gpio_get_input(GPIO_INDEX_FOR_SWITCH_SLIDE1) != 3)
		{
		};

		printf("rising edges: %d\n", num_rise);
		assert_must_msg(num_rise == NUM_PRESS, "expected %d rising edges, got %d", NUM_PRESS, num_rise);
		assert_must(get_slide_switch(0) == 0);
		printf("PASS\n");
	}
	return 0;
}
