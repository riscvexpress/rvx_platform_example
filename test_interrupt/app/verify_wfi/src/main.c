#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_interrupt.h"
#include "ervp_timer.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// wfi example for the "test_interrupt" platform.
// The core must stop at wfi until an interrupt allowed in mie is pending.
// A one-shot timer fires INTERVAL_US later.
//   1. mstatus.MIE=1: wfi returns after the ISR has run.
//   2. mstatus.MIE=0: wfi wakes up without a trap; the ISR runs only when
//      interrupts are enabled again.
// A core that implements wfi as a no-op fails both checks.

#define INTERVAL_US 500

static volatile int timer_count = 0;

static void isr_timer(void)
{
	timer_count++;
}

int main()
{
	int count_after_wfi;

	if(EXCLUSIVE_ID==0)
	{
		register_isr_timer(isr_timer);
		allow_interrupt_timer();

		// 1. interrupts enabled
		enable_interrupt_trap();
		config_timer_interval_us(INTERVAL_US);
		start_timer_once();
		wait_for_interrupt();
		count_after_wfi = timer_count;
		printf("MIE=1: count after wfi %d\n", count_after_wfi);
		assert_must_msg(count_after_wfi==1, "wfi did not wait for the interrupt");

		// 2. interrupts disabled
		timer_count = 0;
		disable_interrupt_trap();
		config_timer_interval_us(INTERVAL_US);
		start_timer_once();
		wait_for_interrupt();
		count_after_wfi = timer_count;
		printf("MIE=0: count after wfi %d\n", count_after_wfi);
		assert_must_msg(count_after_wfi==0, "ISR ran while interrupts were disabled");
		assert_must_msg((read_csr(mip) & MIP_MTIP)!=0, "wfi returned before the interrupt");
		enable_interrupt_trap();
		delay_us(10);
		printf("MIE=0: count after enable %d\n", timer_count);
		assert_must(timer_count==1);

		printf("PASS\n");
	}
	return 0;
}
