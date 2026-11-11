#include "platform_info.h"
#include "flash_info.h"
#include "ervp_printf.h"
#include "trvp_spi.h"
#include "trvp_spi_flash.h"

#define SLOW_DIVISOR 49
#define MAX_TEST_DIVISOR 9
#define NUM_PASS 3
#define TEST_SIZE 4096

#define TEST_ADDR (FLASH_FS_BASEADDR + FLASH_FS_SIZE - FLASH_SECTOR_SIZE)

static unsigned char golden[TEST_SIZE];
static unsigned char buffer[TEST_SIZE];
static unsigned char id_golden[3];

static void set_divisor(int divisor)
{
	const SpiConfig config = {(1<<SPI_INDEX_FOR_SPI_FLASH), divisor, SPI_SCKMODE_3, SPI_CS_ACTIVE_LOW};
	spi_restore_config(&config);
}

static void read_single(unsigned int addr, int size, unsigned char* data)
{
	const unsigned char cmd[5] = {SPI_FLASH_INST_4READ, (addr>>24)&0xFF, (addr>>16)&0xFF, (addr>>8)&0xFF, addr&0xFF};
	spi_read(5, cmd, 0, size, data, 0);
}

static int count_error(const unsigned char* a, const unsigned char* b, int size)
{
	int i, error = 0;
	for(i=0; i<size; i++)
		if(a[i] != b[i])
			error++;
	return error;
}

static unsigned char pattern(int pos, int seed)
{
	return (unsigned char)((pos * 7) ^ (pos >> 8) ^ (seed * 37));
}

static int test_write(int divisor, int seed)
{
	int i;
	for(i=0; i<TEST_SIZE; i++)
		golden[i] = pattern(i, seed);
	spi_flash_erase_sector(TEST_ADDR);
	spi_flash_program(TEST_ADDR, TEST_SIZE, golden);
	set_divisor(SLOW_DIVISOR);
	read_single(TEST_ADDR, TEST_SIZE, buffer);
	set_divisor(divisor);
	return count_error(golden, buffer, TEST_SIZE);
}

int main()
{
	int divisor, pass;
	int write_ok = 1;
	unsigned char id[3];

	spi_flash_configure();
	set_divisor(SLOW_DIVISOR);
	spi_flash_read_id(3, id_golden);
	printf("\n[SPEED] spi_clk %d id 0x%x 0x%x 0x%x test_addr 0x%x", SPI_CLK_HZ, id_golden[0], id_golden[1], id_golden[2], TEST_ADDR);
	if(test_write(SLOW_DIVISOR, 0))
	{
		printf("\n[SPEED] write at sck %d failed", SPI_CLK_HZ/(2*(SLOW_DIVISOR+1)));
		write_ok = 0;
	}

	for(divisor=MAX_TEST_DIVISOR; divisor>=0; divisor--)
	{
		int id_error = 0, single_error = 0, quad_error = 0, write_error = 0;
		for(pass=0; pass<NUM_PASS; pass++)
		{
			set_divisor(SLOW_DIVISOR);
			read_single(TEST_ADDR, TEST_SIZE, golden);
			set_divisor(divisor);
			spi_flash_read_id(3, id);
			id_error += count_error(id_golden, id, 3);
			read_single(TEST_ADDR, TEST_SIZE, buffer);
			single_error += count_error(golden, buffer, TEST_SIZE);
			spi_flash_read(TEST_ADDR, TEST_SIZE, buffer);
			quad_error += count_error(golden, buffer, TEST_SIZE);
		}
		if(write_ok && (id_error==0) && (single_error==0))
		{
			for(pass=0; pass<NUM_PASS; pass++)
				write_error += test_write(divisor, divisor*NUM_PASS+pass);
			if(write_error)
				write_ok = 0;
		}
		else
			write_error = -1;
		printf("\n[SPEED] div %d sck %d id_err %d single_err %d quad_err %d write_err %d", divisor, SPI_CLK_HZ/(2*(divisor+1)), id_error, single_error, quad_error, write_error);
	}
	set_divisor(SLOW_DIVISOR);
	printf("\n[SPEED] done\n");
	return 0;
}
