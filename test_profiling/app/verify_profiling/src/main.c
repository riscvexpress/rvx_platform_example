#include "uthash.h"
#include "ervp_profiling.h"
#include "ervp_delay.h"

int main()
{
	for(int i=0; i<50; i++)
	{
		profiling_start("check_100us");
		delay_us(100);
		profiling_end("check_100us");
	}
	for(int i=0; i<50; i++)
	{
		profiling_start("check_1ms");
		delay_ms(1);
		profiling_end("check_1ms");
	}
	for(int i=0; i<50; i++)
	{
		profiling_start("check_10ms");
		delay_ms(10);
		profiling_end("check_10ms");
	}
	for(int i=0; i<3; i++)
	{
		profiling_start("check_1500ms");
		delay_sec(1);
		for(int i=0; i<5; i++)
		{
			profiling_start("check_100ms");
			delay_ms(100);
			profiling_end("check_100ms");
		}
		profiling_end("check_1500ms");
	}
	return 0;
}
