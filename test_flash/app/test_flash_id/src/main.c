#include "platform_info.h"
#include "ervp_printf.h"
#include "trvp_spi.h"
#include "trvp_spi_flash.h"

int main()
{
	const SpiConfig config = {(1<<SPI_INDEX_FOR_SPI_FLASH), SPI_DIVSOR(1000000), SPI_SCKMODE_3, SPI_CS_ACTIVE_LOW, 0};
	const unsigned char rdsr = SPI_FLASH_INST_RDSR;
	const unsigned char rdcr = SPI_FLASH_INST_RDCR;
	unsigned char id[5];
	unsigned char sr, cr;

	spi_configure(&config);
	spi_flash_read_id(5, id);
	spi_read(1, &rdsr, 0, 1, &sr, 0);
	spi_read(1, &rdcr, 0, 1, &cr, 0);
	printf("\n[FLASH] id: 0x%x 0x%x 0x%x 0x%x 0x%x", id[0], id[1], id[2], id[3], id[4]);
	printf("\n[FLASH] sr1: 0x%x cr1: 0x%x", sr, cr);
	printf("\n[FLASH] latency code: %d quad: %d\n", (cr>>6)&3, (cr>>1)&1);
	return 0;
}
