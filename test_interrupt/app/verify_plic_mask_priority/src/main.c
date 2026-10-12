#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_user_gpio.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"
#include "ervp_interrupt.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// PLIC masking and priority example for the "test_interrupt" platform.
// Uses the switch sequence in user/sim/include/sim_user_region.vh.
//
// 1. Masking: disable_isr_gpio() masks a source at the PLIC. An interrupt
//    raised while masked is held, not lost; it is delivered as soon as
//    enable_isr_gpio() unmasks it.
// 2. Priority: when two sources are pending together, the PLIC hands out the
//    one with the higher priority first (1 lowest .. 7 highest, 0 = never).
//    Around 24 ms slide0 rises and slide1 falls (3->0). Interrupts are
//    disabled globally while both arrive, then enabled at once, so the
//    service order depends only on the priority.
//
// ISRs are not nested: the next ISR starts only after the current one returns.

#define SLIDE0 GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0
#define SLIDE1 GPIO_INDEX_FOR_SWITCH_SLIDE1

#define PRIORITY_SLIDE0 1
#define PRIORITY_SLIDE1 3

#define ID_SLIDE0 0
#define ID_SLIDE1 1

#define MAX_EVENT 8
static volatile int event_list[MAX_EVENT];
static volatile int num_event = 0;

static void record_event(int id)
{
	if(num_event < MAX_EVENT)
		event_list[num_event] = id;
	num_event++;
}

static void isr_slide0(void)
{
	record_event(ID_SLIDE0);
}

static void isr_slide1(void)
{
	record_event(ID_SLIDE1);
}

static void wait_for_slide0(int value)
{
	int i;
	for(i=0; i<(40*get_test_period_ms()); i++)
	{
		if(get_slide_switch(0)==value)
			break;
		delay_ms(1);
	}
	assert_must_msg(get_slide_switch(0)==value, "timeout: slide0 did not become %d", value);
}

static void wait_for_events(int expected)
{
	int i;
	for(i=0; i<(40*get_test_period_ms()); i++)
	{
		if(num_event>=expected)
			break;
		delay_ms(1);
	}
	assert_must_msg(num_event>=expected, "timeout: %d/%d events", num_event, expected);
}

int main()
{
	int period_ms = get_test_period_ms();

	if(EXCLUSIVE_ID==0)
	{
		user_gpio_set_input_cfg(SLIDE1, 0);
		assert_must_msg((get_slide_switch(0)==0) && (user_gpio_get_input(SLIDE1)==0), "set all switches down before start");

		user_gpio_enable_interrupt(SLIDE0, ERVP_TRIGGER_COND_RISE, 0);
		register_isr_gpio(SLIDE0, isr_slide0, PRIORITY_SLIDE0);
		// a multi-bit FALL fires when the value decreases: only 3->0 here
		user_gpio_enable_interrupt(SLIDE1, ERVP_TRIGGER_COND_FALL, 0);
		register_isr_gpio(SLIDE1, isr_slide1, PRIORITY_SLIDE1);

		// register_isr_gpio() also enables the source; mask slide0 for step 1
		disable_isr_gpio(SLIDE0);

		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();

		// 1. masking
		printf("slide0 masked\n");
		wait_for_slide0(1);
		delay_ms(period_ms);
		assert_must_msg(num_event==0, "masked interrupt was delivered");
		printf("slide0 rose while masked: no interrupt\n");

		enable_isr_gpio(SLIDE0);
		wait_for_events(1);
		assert_must(event_list[0]==ID_SLIDE0);
		printf("slide0 unmasked: held interrupt delivered\n");

		// 2. priority
		disable_interrupt_trap();
		wait_for_slide0(0);
		wait_for_slide0(1);
		delay_ms(period_ms);
		assert_must_msg(user_gpio_get_input(SLIDE1)==0, "slide1 did not fall");
		assert_must(num_event==1);
		printf("both pending, interrupts disabled\n");
		enable_interrupt_trap();
		wait_for_events(3);
		delay_ms(4*period_ms);
		printf("served: %d then %d\n", event_list[1], event_list[2]);
		assert_must_msg(num_event==3, "%d events", num_event);
		assert_must_msg(event_list[1]==ID_SLIDE1, "lower priority served first");
		assert_must(event_list[2]==ID_SLIDE0);
		printf("higher priority (slide1) served first\n");

		printf("PASS\n");
	}
	return 0;
}
