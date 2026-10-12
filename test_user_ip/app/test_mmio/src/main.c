#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_reg_util.h"

#define OFFSET_X 0x0
#define OFFSET_Y 0x4
#define OFFSET_Z 0x8
#define OFFSET_ADD 0xC
#define OFFSET_SUB 0xC
#define OFFSET_MAX 0x10
#define OFFSET_NOI_VALID 0x3

static inline unsigned int get_test1_addr(unsigned int offset)
{
	return (I_TEST1_SLAVE_BASEADDR+offset);
}

void set_x(unsigned int value)
{
	REG32(get_test1_addr(OFFSET_X)) = value;
}

unsigned int get_x()
{
	return REG32(get_test1_addr(OFFSET_X));
}

void set_y(unsigned int value)
{
	REG32(get_test1_addr(OFFSET_Y)) = value;
}

unsigned int get_y()
{
	return REG32(get_test1_addr(OFFSET_Y));
}

void set_z(unsigned int value)
{
	REG32(get_test1_addr(OFFSET_Z)) = value;
}

unsigned int get_z()
{
	return REG32(get_test1_addr(OFFSET_Z));
}


void perform_add()
{
	REG32(get_test1_addr(OFFSET_ADD)) = 0;
}

void perform_sub()
{
	int temp;
	temp = REG32(get_test1_addr(OFFSET_SUB));
}

void perform_mac()
{
	REG32(get_test1_addr(OFFSET_MAX)) = 3;
}

int main() {
	set_x(3);
	printf("\n%d", get_x());
	set_y(6);
	printf("\n%d", get_y());
	set_z(4);
	printf("\n%d", get_z());
	perform_add();
	printf("\n%d", get_z());
	perform_sub();
	printf("\n%d", get_z());
	perform_mac();
	printf("\n%d", get_z());
	REG32(get_test1_addr(OFFSET_NOI_VALID)) = 0xffffffff;
	return 0;
}
