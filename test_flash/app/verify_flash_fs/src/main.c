#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_flash_fs.h"
#include "trvp_spi_flash.h"

#define TEST_FILE_SIZE 256

static uint8_t buffer[TEST_FILE_SIZE];

static uint8_t pattern(int pos)
{
	return (uint8_t)((pos * 7) ^ 0x5A);
}

static void check_test_file(void)
{
	int i;
	FLASHFS_FILE* fp = flashfs_fopen("test", "r");
	assert_must(fp != NULL);
	assert_must(flashfs_get_file_size(fp) == TEST_FILE_SIZE);
	assert_must(flashfs_fread(buffer, 1, TEST_FILE_SIZE, fp) == TEST_FILE_SIZE);
	for(i = 0; i < TEST_FILE_SIZE; i++)
		assert_must(buffer[i] == pattern(i));
	assert_must(flashfs_fclose(fp) == 0);
}

int main()
{
	unsigned char id[3];
	flashfs_fsinfo_t fsinfo;
	FLASHFS_FILE* fp;
	int result, i;

	spi_flash_configure();
	spi_flash_read_id(3, id);
	printf("\n[LFS] flash id: 0x%x 0x%x 0x%x", id[0], id[1], id[2]);

	result = flashfs_mount();
	if(result == LFS_ERR_CORRUPT)
	{
		printf("\n[LFS] no file system: format");
		assert_must(flashfs_format() == LFS_ERR_OK);
		result = flashfs_mount();
	}
	assert_must(result == LFS_ERR_OK);
	assert_must(flashfs_fs_stat(&fsinfo) == LFS_ERR_OK);
	printf("\n[LFS] mounted: %d blocks of %d bytes", fsinfo.block_count, fsinfo.block_size);

	for(i = 0; i < TEST_FILE_SIZE; i++)
		buffer[i] = pattern(i);
	fp = flashfs_fopen("test", "w");
	assert_must(fp != NULL);
	assert_must(flashfs_fwrite(buffer, 1, TEST_FILE_SIZE, fp) == TEST_FILE_SIZE);
	assert_must(flashfs_fclose(fp) == 0);
	check_test_file();
	printf("\n[LFS] file check passed");

	assert_must(flashfs_unmount() == LFS_ERR_OK);
	assert_must(flashfs_mount() == LFS_ERR_OK);
	check_test_file();
	assert_must(flashfs_unmount() == LFS_ERR_OK);
	printf("\n[LFS] PASS\n");
	return 0;
}
