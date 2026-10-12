#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_interrupt.h"
#include "ervp_timer.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// Timer interrupt example for the "test_interrupt" platform.
//
// Flow: register an ISR -> set the interval -> allow the timer interrupt
//       (mie.MTIP) -> enable interrupts globally (mstatus.MIE) -> start.
//
// Note: delay_us()/delay_ms() use the same timer block in its delay mode.
// Delay and interrupt modes coexist, but stop_timer() stops BOTH,
// so never call stop_timer() while a delay is in progress.

#define INTERVAL_US 200
#define NUM_PERIODIC 5

static volatile int timer_count = 0;

static void isr_timer(void)
{
	// runs with interrupts disabled; keep it short
	timer_count++;
}

static void wait_for_count(int expected)
{
	int i;
	for(i=0; i<100; i++)
	{
		if(timer_count>=expected)
			break;
		delay_us(INTERVAL_US/4);
	}
	assert_must_msg(timer_count>=expected, "timer interrupt timeout: %d/%d", timer_count, expected);
}

int main()
{
	int count_at_stop;

	if(EXCLUSIVE_ID==0)
	{
		register_isr_timer(isr_timer);
		allow_interrupt_timer();
		enable_interrupt_trap();

		// 1. periodic mode: an interrupt every INTERVAL_US
		config_timer_interval_us(INTERVAL_US);
		start_timer_periodic();
		wait_for_count(NUM_PERIODIC);
		stop_timer();
		count_at_stop = timer_count;
		printf("periodic: %d interrupts\n", count_at_stop);

		// after stop_timer(), no more interrupts
		delay_us(INTERVAL_US*5);
		assert_must_msg(timer_count==count_at_stop, "interrupt after stop: %d", timer_count);
		printf("stop: no interrupt\n");

		// 2. one-shot mode: exactly one interrupt
		timer_count = 0;
		config_timer_interval_us(INTERVAL_US);
		start_timer_once();
		wait_for_count(1);
		delay_us(INTERVAL_US*5);
		assert_must_msg(timer_count==1, "one-shot fired %d times", timer_count);
		printf("once: 1 interrupt\n");

		printf("PASS\n");
	}
	return 0;
}
