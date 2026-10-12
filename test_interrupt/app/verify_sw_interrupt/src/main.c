#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_interrupt.h"
#include "ervp_core_id.h"
#include "ervp_variable_allocation.h"

// Software interrupt example for the "test_interrupt" platform (2 cores).
//
// Core 0 and core 1 send software interrupts to each other NUM_ROUND times.
// send_software_interrupt(id) sets bit id of cpg_misc.sw_itr_list; that bit
// reaches core id as mip.MSIP. The library clears the bit before the ISR.
//
// The receiver waits with wfi. Interrupts are disabled around the check so
// that an interrupt arriving between the check and wfi is not lost: wfi
// still wakes up on a pending interrupt allowed in mie, and the ISR then
// runs when interrupts are enabled again.

#if !defined(INCLUDE_RVC_ORCA_PLUS) && !defined(INCLUDE_RVC_ROCKET_OPT)
#error "software interrupt needs rvc_orca_plus or rvc_rocket_opt"
#endif

#define NUM_ROUND 4

static volatile int received[NUM_CORE_PHYSICAL] NOTCACHED_DATA;
static volatile int core1_ready NOTCACHED_DATA;

static void isr_software(void)
{
	received[EXCLUSIVE_ID]++;
}

static void wait_for_software_interrupt(int expected)
{
	disable_interrupt_trap();
	while(received[EXCLUSIVE_ID] < expected)
	{
		wait_for_interrupt();
		enable_interrupt_trap();
		disable_interrupt_trap();
	}
	enable_interrupt_trap();
}

int main()
{
	int i;

	assert_must_msg(NUM_CORE_PHYSICAL==2, "needs 2 cores: %d", NUM_CORE_PHYSICAL);

	register_isr_software(isr_software);
	allow_interrupt_software();
	enable_interrupt_trap();

	if(EXCLUSIVE_ID==1)
	{
		core1_ready = 1;
		for(i=1; i<=NUM_ROUND; i++)
		{
			wait_for_software_interrupt(i);
			send_software_interrupt(0);
		}
	}
	else if(EXCLUSIVE_ID==0)
	{
		while(core1_ready==0);
		for(i=1; i<=NUM_ROUND; i++)
		{
			send_software_interrupt(1);
			wait_for_software_interrupt(i);
			printf("round %d: core0 %d, core1 %d\n", i, received[0], received[1]);
		}
		assert_must(received[0]==NUM_ROUND);
		assert_must(received[1]==NUM_ROUND);
		printf("PASS\n");
	}
	return 0;
}
