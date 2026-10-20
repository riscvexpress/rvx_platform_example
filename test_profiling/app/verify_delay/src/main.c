#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_delay.h"
#include "ervp_real_clock.h"
#include "core_dependent.h"

#ifdef INCLUDE_MCYCLE
static uint64_t real_clock_read_cycle;
static uint64_t measure_overhead;

static void measure_real_clock_read()
{
	uint64_t cycle_start = get_mcycle();
	get_real_clock_tick();
	real_clock_read_cycle = get_mcycle() - cycle_start;
	printf_must("\n[real clock read] %llu cycles", real_clock_read_cycle);
}
#endif

static void check_delay(unsigned int time_in_us)
{
	uint64_t real_start, real_end, real_us;
	if (time_in_us < 20)
		delay_us(time_in_us);
	real_start = get_real_clock_tick();
#ifdef INCLUDE_MCYCLE
	uint64_t cycle_start, cycle_end, cycle_us;
	cycle_start = get_mcycle();
#endif
	delay_us(time_in_us);
#ifdef INCLUDE_MCYCLE
	cycle_end = get_mcycle();
#endif
	real_end = get_real_clock_tick();
	real_us = real_end - real_start;
	printf_must("\n[delay %u us] real_clock %llu us", time_in_us, real_us);
	assert_must(real_us >= time_in_us);
	assert_must(real_us <= time_in_us + 10 + time_in_us / 100);
#ifdef INCLUDE_MCYCLE
	cycle_us = (cycle_end - cycle_start) / (CORE_CLK_HZ / 1000000);
	printf_must(", mcycle %llu cycles (%llu us)", cycle_end - cycle_start, cycle_us);
	assert_must(cycle_us <= real_us + 1);
	assert_must(cycle_us + (2 * real_clock_read_cycle) / (CORE_CLK_HZ / 1000000) + 2 + real_us / 100 >= real_us);
#endif
}

#ifdef INCLUDE_MCYCLE
static void check_elapsed_cycle(const char *name, unsigned int requested, uint64_t elapsed, uint64_t expected)
{
	printf_must("\n[%s %u] mcycle %llu (expected %llu)", name, requested, elapsed, expected);
	assert_must(elapsed >= expected);
	assert_must(elapsed <= expected + 100);
}

static void check_delay_cycle(unsigned int num_cycle)
{
	delay_cycle(num_cycle);
	uint64_t cycle_start = get_mcycle();
	delay_cycle(num_cycle);
	check_elapsed_cycle("delay_cycle", num_cycle, get_mcycle() - cycle_start, num_cycle);
}

static void check_delay_ns(unsigned int time_in_ns)
{
	const unsigned int cycle_per_us = CORE_CLK_HZ / 1000000;
	uint64_t expected = (((uint64_t)time_in_ns) * cycle_per_us + 999) / 1000;
	delay_ns(time_in_ns);
	uint64_t cycle_start = get_mcycle();
	delay_ns(time_in_ns);
	check_elapsed_cycle("delay_ns", time_in_ns, get_mcycle() - cycle_start, expected);
}
#endif

int main()
{
#ifdef INCLUDE_MCYCLE
	printf_must("\nmcycle @ %u Hz", CORE_CLK_HZ);
	for (int i = 0; i < 2; i++)
	{
		uint64_t cycle_first = get_mcycle();
		uint64_t cycle_second = get_mcycle();
		assert_must(cycle_second > cycle_first);
		measure_overhead = cycle_second - cycle_first;
	}
	printf_must("\n[mcycle measure] %llu cycles", measure_overhead);
	measure_real_clock_read();
#else
	printf_must("\nreal clock @ %u Hz", TICK_HZ);
#endif
	uint32_t real_tick32 = get_real_clock_tick32();
	uint64_t real_tick = get_real_clock_tick();
	printf_must("\n[real clock] tick32 %u, tick %llu", real_tick32, real_tick);
	assert_must((((uint32_t)real_tick) - real_tick32) <= 10);
	check_delay(1);
	check_delay(10);
	check_delay(100);
	check_delay(1000);
	check_delay(5000);
#ifdef INCLUDE_MCYCLE
	check_delay_cycle(10);
	check_delay_cycle(100);
	check_delay_cycle(1000);
	check_delay_ns(100);
	check_delay_ns(500);
	check_delay_ns(2500);
	check_delay_ns(10000);
	check_delay_ns(1234567);
#endif
	printf_must("\n[verify_delay] PASS\n");
	return 0;
}
