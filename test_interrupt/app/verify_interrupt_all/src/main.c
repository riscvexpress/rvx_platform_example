#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_reg_util.h"
#include "ervp_interrupt.h"
#include "ervp_timer.h"
#include "ervp_delay.h"
#include "ervp_core_id.h"
#include "ervp_gpio.h"
#include "ervp_user_gpio.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"
#include "ervp_variable_allocation.h"

// All interrupt checks of the "test_interrupt" platform in one run.
// Core 0 runs the steps in this order. The GPIO steps follow the switch
// sequence in user/sim/include/sim_user_region.vh, so they come first.
//   1. GPIO: edge (RISE) and level (EQ) conditions
//   2. GPIO debouncing
//   3. timer: periodic, stop, one-shot
//   4. wfi with mstatus.MIE on and off
//   5. user IP interrupts: one ISR per line, PLIC priority order
//   6. PLIC masking
//   7. software interrupt between core 0 and core 1
// Keep printf out of the switch stimulus windows: with several cores,
// printf holds a lock with interrupts disabled for milliseconds.

#include "check_config.h"

#define SLIDE0 GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0
#define SLIDE1 GPIO_INDEX_FOR_SWITCH_SLIDE1

#if CHECK_SW_INTERRUPT && (defined(INCLUDE_RVC_ORCA_PLUS) || defined(INCLUDE_RVC_ROCKET_OPT))
#define USE_SW_INTERRUPT
#endif

static void wait_until(volatile int* value, int expected, int timeout_us, const char* name)
{
	int i;
	for(i=0; i<timeout_us; i+=10)
	{
		if(*value>=expected)
			break;
		delay_us(10);
	}
	assert_must_msg(*value>=expected, "%s timeout: %d/%d", name, *value, expected);
}

////////////
/* 1. GPIO */
////////////

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
		disable_isr_gpio(SLIDE1);
}

static int is_gpio_done()
{
	return (slide0_rise_count>=2) && (slide1_step>=NUM_SLIDE1_STEP);
}

static void check_gpio()
{
	int i;
	int period_ms = get_test_period_ms();

	user_gpio_set_input_cfg(SLIDE1, 0);
	assert_must_msg((get_slide_switch(0)==0) && (user_gpio_get_input(SLIDE1)==0), "set all switches down before start");

	user_gpio_enable_interrupt(SLIDE0, ERVP_TRIGGER_COND_RISE, 0);
	register_isr_gpio(SLIDE0, isr_slide0, 1);
	user_gpio_enable_interrupt(SLIDE1, ERVP_TRIGGER_COND_EQ, slide1_expected[0]);
	register_isr_gpio(SLIDE1, isr_slide1, 1);

	for(i=0; i<(40*period_ms); i++)
	{
		if(is_gpio_done())
			break;
		delay_ms(1);
	}
	assert_must_msg(is_gpio_done(), "GPIO timeout: slide0 %d, slide1 %d", slide0_rise_count, slide1_step);
	delay_ms(6*period_ms);
	disable_isr_gpio(SLIDE0);

	assert_must_msg(slide0_rise_count==2, "slide0 rise: %d", slide0_rise_count);
	for(i=0; i<NUM_SLIDE1_STEP; i++)
		assert_must_msg(slide1_seen[i]==slide1_expected[i], "slide1 eq %d: read %d", slide1_expected[i], slide1_seen[i]);
	printf("1. GPIO: PASS\n");
}

///////////////////////
/* 2. GPIO debouncing */
///////////////////////

#define NUM_PRESS 3

static volatile int debounce_rise_count = 0;

static void isr_debounce(void)
{
	debounce_rise_count++;
}

static int debounce_enabled(unsigned int index)
{
	return (gpio_get_config(get_user_gpio_addr(index)) & GPIO_CONFIG_DEBOUNCE_ENABLE) != 0;
}

static void check_gpio_debounce()
{
	assert_must_msg(debounce_enabled(SLIDE0), "debounce is not enabled by init_switch()");

	user_gpio_enable_interrupt(SLIDE0, ERVP_TRIGGER_COND_RISE, 0);
	register_isr_gpio(SLIDE0, isr_debounce, 1);

	// enabling debouncing on switch_slide1 starts the testbench stimulus
	user_gpio_set_input_cfg(SLIDE1, GPIO_CONFIG_DEBOUNCE_ENABLE);
	while(user_gpio_get_input(SLIDE1)!=3);
	disable_isr_gpio(SLIDE0);

	assert_must_msg(debounce_rise_count==NUM_PRESS, "expected %d rising edges, got %d", NUM_PRESS, debounce_rise_count);
	assert_must(get_slide_switch(0)==0);
	printf("2. GPIO debouncing: PASS\n");
}

/////////////
/* 3. timer */
/////////////

#define TIMER_INTERVAL_US 200
#define NUM_PERIODIC 5

static volatile int timer_count = 0;

static void isr_timer(void)
{
	timer_count++;
}

static void check_timer()
{
	int count_at_stop;

	timer_count = 0;
	register_isr_timer(isr_timer);
	allow_interrupt_timer();

	config_timer_interval_us(TIMER_INTERVAL_US);
	start_timer_periodic();
	wait_until(&timer_count, NUM_PERIODIC, 100*TIMER_INTERVAL_US, "periodic timer");
	stop_timer();
	count_at_stop = timer_count;
	delay_us(TIMER_INTERVAL_US*5);
	assert_must_msg(timer_count==count_at_stop, "interrupt after stop: %d", timer_count);

	timer_count = 0;
	config_timer_interval_us(TIMER_INTERVAL_US);
	start_timer_once();
	wait_until(&timer_count, 1, 100*TIMER_INTERVAL_US, "one-shot timer");
	delay_us(TIMER_INTERVAL_US*5);
	assert_must_msg(timer_count==1, "one-shot fired %d times", timer_count);
	printf("3. timer: PASS\n");
}

///////////
/* 4. wfi */
///////////

#define WFI_INTERVAL_US 500

static void check_wfi()
{
	int count_after_wfi;

	timer_count = 0;
	register_isr_timer(isr_timer);
	allow_interrupt_timer();

	config_timer_interval_us(WFI_INTERVAL_US);
	start_timer_once();
	wait_for_interrupt();
	count_after_wfi = timer_count;
	assert_must_msg(count_after_wfi==1, "MIE=1: wfi did not wait for the interrupt (%d)", count_after_wfi);

	timer_count = 0;
	disable_interrupt_trap();
	config_timer_interval_us(WFI_INTERVAL_US);
	start_timer_once();
	wait_for_interrupt();
	count_after_wfi = timer_count;
	assert_must_msg(count_after_wfi==0, "MIE=0: ISR ran while interrupts were disabled");
	assert_must_msg((read_csr(mip) & MIP_MTIP)!=0, "MIE=0: wfi returned before the interrupt");
	enable_interrupt_trap();
	delay_us(10);
	assert_must_msg(timer_count==1, "MIE=0: count after enable %d", timer_count);
	printf("4. wfi: PASS\n");
}

/////////////////////////
/* 5, 6. user IP, PLIC */
/////////////////////////

#define NUM_USER_ITR 4

#define ITR_GEN_OFFSET_SET   0x0
#define ITR_GEN_OFFSET_CLEAR 0x4

static inline void itr_gen_set(unsigned int mask)
{
	REG32(I_ITR_GEN_SLAVE_BASEADDR + ITR_GEN_OFFSET_SET) = mask;
}

static inline void itr_gen_clear(unsigned int mask)
{
	REG32(I_ITR_GEN_SLAVE_BASEADDR + ITR_GEN_OFFSET_CLEAR) = mask;
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

static const function_ptr_t user_isr_list[NUM_USER_ITR] = {isr_user0, isr_user1, isr_user2, isr_user3};
static const int user_priority_list[NUM_USER_ITR] = {1, 4, 2, 3};
static const int user_expected_order[NUM_USER_ITR] = {1, 3, 2, 0};

static void check_user_itr()
{
	int i;

	for(i=0; i<NUM_USER_ITR; i++)
		register_isr_user(i, user_isr_list[i], user_priority_list[i]);

	for(i=0; i<NUM_USER_ITR; i++)
	{
		num_event = 0;
		itr_gen_set(1<<i);
		wait_until(&num_event, 1, 1000, "user interrupt");
		delay_us(10);
		assert_must_msg(num_event==1, "user %d: %d events", i, num_event);
		assert_must(event_list[0]==i);
	}

	num_event = 0;
	disable_interrupt_trap();
	itr_gen_set((1<<NUM_USER_ITR)-1);
	enable_interrupt_trap();
	wait_until(&num_event, NUM_USER_ITR, 1000, "user interrupts");
	delay_us(10);
	assert_must(num_event==NUM_USER_ITR);
	for(i=0; i<NUM_USER_ITR; i++)
		assert_must_msg(event_list[i]==user_expected_order[i], "order %d: user %d", i, event_list[i]);
	printf("5. user IP interrupts and PLIC priority: PASS\n");
}

static void check_plic_mask()
{
	num_event = 0;
	disable_isr_user(0);
	itr_gen_set(1<<0);
	delay_us(100);
	assert_must_msg(num_event==0, "masked interrupt was delivered");

	enable_isr_user(0);
	wait_until(&num_event, 1, 1000, "unmasked interrupt");
	delay_us(10);
	assert_must(num_event==1);
	assert_must(event_list[0]==0);
	printf("6. PLIC masking: PASS\n");
}

//////////////////////////
/* 7. software interrupt */
//////////////////////////

#ifdef USE_SW_INTERRUPT

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

static void init_software_interrupt()
{
	register_isr_software(isr_software);
	allow_interrupt_software();
}

static void run_core1()
{
	int i;
	init_software_interrupt();
	enable_interrupt_trap();
	core1_ready = 1;
	for(i=1; i<=NUM_ROUND; i++)
	{
		wait_for_software_interrupt(i);
		send_software_interrupt(0);
	}
}

static void check_software_interrupt()
{
	int i;
	init_software_interrupt();
	while(core1_ready==0);
	for(i=1; i<=NUM_ROUND; i++)
	{
		send_software_interrupt(1);
		wait_for_software_interrupt(i);
	}
	assert_must_msg(received[0]==NUM_ROUND, "core0 received %d", received[0]);
	assert_must_msg(received[1]==NUM_ROUND, "core1 received %d", received[1]);
	printf("7. software interrupt: PASS\n");
}

#endif

int main()
{
#ifdef USE_SW_INTERRUPT
	if((EXCLUSIVE_ID==1) && (NUM_CORE_PHYSICAL>=2))
		run_core1();
#endif

	if(EXCLUSIVE_ID==0)
	{
		register_plic_grant();
		allow_interrupt_plic();
		enable_interrupt_trap();

		if(CHECK_GPIO)
			check_gpio();
		if(CHECK_GPIO_DEBOUNCE)
			check_gpio_debounce();
		if(CHECK_TIMER)
			check_timer();
		if(CHECK_WFI)
			check_wfi();
		if(CHECK_USER_ITR)
			check_user_itr();
		if(CHECK_PLIC_MASK)
			check_plic_mask();
#ifdef USE_SW_INTERRUPT
		if(NUM_CORE_PHYSICAL>=2)
			check_software_interrupt();
		else
			printf("7. software interrupt: skipped (1 core)\n");
#else
		if(CHECK_SW_INTERRUPT)
			printf("7. software interrupt: skipped (core)\n");
#endif
		printf("PASS\n");
	}
	return 0;
}
