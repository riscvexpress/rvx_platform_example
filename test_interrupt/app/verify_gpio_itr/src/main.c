#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_user_gpio.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"
#include "ervp_interrupt.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// GPIO interrupt example for the "test_interrupt" platform.
// The switches are driven by user/sim/include/sim_user_region.vh in RTL
// simulation; on FPGA, start with all switches down and follow that sequence.
//
// GPIO interrupts go through the PLIC:
//   user_gpio_enable_interrupt()  trigger condition in the GPIO block
//   register_isr_gpio()           ISR + PLIC enable + PLIC priority
//   allow_interrupt_plic()        mie.MEIP
//   enable_interrupt_trap()       mstatus.MIE
// The library clears the GPIO interrupt after the ISR returns.
//
// slide0 (1 bit): RISE is an edge condition; it fires once per 0->1.
// slide1 (2 bit): EQ is a level condition; it fires again right after the
//   clear while the value still matches. The ISR re-arms it with the next
//   value, and masks it at the PLIC after the last one.

#define SLIDE0 GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0
#define SLIDE1 GPIO_INDEX_FOR_SWITCH_SLIDE1

#define NUM_SLIDE1_STEP 4
static const int slide1_expected[NUM_SLIDE1_STEP] = {1, 2, 3, 0};

static volatile int slide0_rise_count = 0;
static volatile int slide1_step = 0;
static volatile int slide1_seen[NUM_SLIDE1_STEP];

static void isr_slide0(void)
{
	slide0_rise_count++;
}

static void isr_slide1(void)
{
	slide1_seen[slide1_step] = user_gpio_get_input(SLIDE1);
	slide1_step++;
	if(slide1_step < NUM_SLIDE1_STEP)
		user_gpio_enable_interrupt(SLIDE1, ERVP_TRIGGER_COND_EQ, slide1_expected[slide1_step]);
	else
		// do not use user_gpio_disable_interrupt() here: a disabled GPIO ignores
		// the clear, so its interrupt line would stay high
		disable_isr_gpio(SLIDE1);
}

static int is_done()
{
	return (slide0_rise_count>=2) && (slide1_step>=NUM_SLIDE1_STEP);
}

int main()
{
	int i;
	int period_ms = get_test_period_ms();

	if(EXCLUSIVE_ID==0)
	{
		user_gpio_set_input_cfg(SLIDE1, 0);
		assert_must_msg((get_slide_switch(0)==0) && (user_gpio_get_input(SLIDE1)==0), "set all switches down before start");

		user_gpio_enable_interrupt(SLIDE0, ERVP_TRIGGER_COND_RISE, 0);
		register_isr_gpio(SLIDE0, isr_slide0, 1);

		user_gpio_enable_interrupt(SLIDE1, ERVP_TRIGGER_COND_EQ, slide1_expected[0]);
		register_isr_gpio(SLIDE1, isr_slide1, 1);

		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();
		printf("waiting for switch events\n");

		for(i=0; i<(40*period_ms); i++)
		{
			if(is_done())
				break;
			delay_ms(1);
		}
		assert_must_msg(is_done(), "timeout: slide0 %d, slide1 %d", slide0_rise_count, slide1_step);

		// stay a while to catch extra interrupts (slide0 falls at 28 ms)
		delay_ms(6*period_ms);

		printf("slide0 rise: %d\n", slide0_rise_count);
		assert_must(slide0_rise_count==2);
		for(i=0; i<NUM_SLIDE1_STEP; i++)
		{
			printf("slide1 eq %d: read %d\n", slide1_expected[i], slide1_seen[i]);
			assert_must(slide1_seen[i]==slide1_expected[i]);
		}
		assert_must(slide1_step==NUM_SLIDE1_STEP);

		printf("PASS\n");
	}
	return 0;
}
