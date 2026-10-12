#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_reg_util.h"
#include "ervp_interrupt.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"

// User interrupt example for the "test_interrupt" platform.
//
// A user IP (user/rtl/src/itr_gen_apb.v, instance i_itr_gen) drives the four
// user interrupt lines of the PLIC (include_user_interrupts in the XML).
// Software raises a line through the IP's SET register; the ISR lowers it
// through CLEAR. Replace ITR_GEN_APB with your own IP and keep the same
// rule: the ISR must lower the line before it returns, because the PLIC
// gateway is level sensitive.
//
// 1. each line calls its own ISR
// 2. lines raised together are served in priority order

#define NUM_USER_ITR 4

#define OFFSET_SET   0x0
#define OFFSET_CLEAR 0x4

static inline void itr_gen_set(unsigned int mask)
{
	REG32(I_ITR_GEN_SLAVE_BASEADDR + OFFSET_SET) = mask;
}

static inline void itr_gen_clear(unsigned int mask)
{
	REG32(I_ITR_GEN_SLAVE_BASEADDR + OFFSET_CLEAR) = mask;
}

#define MAX_EVENT 8
static volatile int event_list[MAX_EVENT];
static volatile int num_event = 0;

static void handle_user_itr(int index)
{
	itr_gen_clear(1<<index);
	if(num_event < MAX_EVENT)
		event_list[num_event] = index;
	num_event++;
}

static void isr_user0(void) { handle_user_itr(0); }
static void isr_user1(void) { handle_user_itr(1); }
static void isr_user2(void) { handle_user_itr(2); }
static void isr_user3(void) { handle_user_itr(3); }

static const function_ptr_t isr_list[NUM_USER_ITR] = {isr_user0, isr_user1, isr_user2, isr_user3};
static const int priority_list[NUM_USER_ITR] = {1, 4, 2, 3};
static const int expected_order[NUM_USER_ITR] = {1, 3, 2, 0};

static void wait_for_events(int expected)
{
	int i;
	for(i=0; i<100; i++)
	{
		if(num_event>=expected)
			break;
		delay_us(10);
	}
	assert_must_msg(num_event>=expected, "timeout: %d/%d events", num_event, expected);
}

int main()
{
	int i;

	if(EXCLUSIVE_ID==0)
	{
		for(i=0; i<NUM_USER_ITR; i++)
			register_isr_user(i, isr_list[i], priority_list[i]);

		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();

		// 1. one line at a time
		for(i=0; i<NUM_USER_ITR; i++)
		{
			num_event = 0;
			itr_gen_set(1<<i);
			wait_for_events(1);
			delay_us(10);
			assert_must_msg(num_event==1, "user %d: %d events", i, num_event);
			assert_must(event_list[0]==i);
			printf("user %d: ISR called\n", i);
		}

		// 2. all lines at once
		num_event = 0;
		disable_interrupt_trap();
		itr_gen_set((1<<NUM_USER_ITR)-1);
		enable_interrupt_trap();
		wait_for_events(NUM_USER_ITR);
		delay_us(10);
		assert_must(num_event==NUM_USER_ITR);
		for(i=0; i<NUM_USER_ITR; i++)
		{
			printf("order %d: user %d (priority %d)\n", i, event_list[i], priority_list[event_list[i]]);
			assert_must(event_list[i]==expected_order[i]);
		}

		printf("PASS\n");
	}
	return 0;
}
