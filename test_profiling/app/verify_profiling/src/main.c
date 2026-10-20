#include "uthash.h"
#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_profiling.h"
#include "ervp_delay.h"
#include "core_dependent.h"

#ifdef INCLUDE_MCYCLE
#define PROFILING_TICK_HZ CORE_CLK_HZ
#else
#define PROFILING_TICK_HZ TICK_HZ
#endif

static uint64_t get_average_us(const profile_t *section)
{
	uint64_t total_tick = section->num_tick_rshifted << section->shift_amount;
	return total_tick / section->num_called / (PROFILING_TICK_HZ / 1000000);
}

static uint64_t check_section(const profile_t *section, unsigned int time_in_us, int num_called)
{
	uint64_t total_tick = section->num_tick_rshifted << section->shift_amount;
	uint64_t average_us = get_average_us(section);
	printf_must("\n[section %s] called %u, total tick %llu, average %llu us", section->name, section->num_called, total_tick, average_us);
	assert_must(section->num_called == num_called);
	assert_must(average_us >= time_in_us);
	assert_must(average_us <= time_in_us + 20 + time_in_us / 100);
	return average_us;
}

static profile_t *repeat_section(const char *name, unsigned int time_in_us, int num_iter)
{
	profile_t *section = NULL;
	for (int i = 0; i < num_iter; i++)
	{
		section = profiling_start_by_name(name);
		delay_us(time_in_us);
		profiling_end(name);
	}
	return section;
}

int main()
{
	profile_t *section_100us = repeat_section("check_100us", 100, 10);
	profile_t *section_1ms = repeat_section("check_1ms", 1000, 5);
	profile_t *section_5ms = repeat_section("check_5ms", 5000, 3);
	profile_t *section_overhead = NULL;
	profile_t *section_outer = NULL;
	profile_t *section_inner = NULL;
	for (int i = 0; i < 2; i++)
	{
		section_overhead = profiling_start_by_name("check_overhead");
		repeat_section("check_empty", 0, 3);
		profiling_end("check_overhead");
	}
	for (int i = 0; i < 2; i++)
	{
		section_outer = profiling_start_by_name("check_outer");
		delay_us(400);
		section_inner = repeat_section("check_inner_200us", 200, 3);
		profiling_end("check_outer");
	}
	check_section(section_100us, 100, 10);
	check_section(section_1ms, 1000, 5);
	check_section(section_5ms, 5000, 3);
	uint64_t inner_us = check_section(section_inner, 200, 6);
	uint64_t overhead_us = get_average_us(section_overhead);
	printf_must("\n[overhead of 3 nested sections] %llu us", overhead_us);
	check_section(section_outer, 400 + 3 * inner_us + overhead_us, 2);
	printf_must("\n[verify_profiling] PASS\n");
	return 0;
}
