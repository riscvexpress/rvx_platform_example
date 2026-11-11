#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_flash_fs.h"

#define TEST_FILE_SIZE (128*1024)
#define CHUNK_SIZE 256

static uint8_t chunk[CHUNK_SIZE];

static uint8_t pattern(int pos)
{
	return (uint8_t)((pos * 7) ^ (pos >> 8));
}

static int update_boot_count(void)
{
	uint32_t boot_count = 0;
	FLASHFS_FILE* fp = flashfs_fopen("boot_count", "r+");
	if(fp == NULL)
		fp = flashfs_fopen("boot_count", "w+");
	assert_must(fp != NULL);
	flashfs_fread(&boot_count, sizeof(boot_count), 1, fp);
	assert_must(flashfs_ferror(fp) == LFS_ERR_OK);
	boot_count++;
	flashfs_frewind(fp);
	assert_must(flashfs_fwrite(&boot_count, sizeof(boot_count), 1, fp) == 1);
	assert_must(flashfs_fclose(fp) == 0);
	return boot_count;
}

static void write_test_file(void)
{
	int pos, i;
	FLASHFS_FILE* fp = flashfs_fopen("test", "w");
	assert_must(fp != NULL);
	for(pos = 0; pos < TEST_FILE_SIZE; pos += CHUNK_SIZE)
	{
		for(i = 0; i < CHUNK_SIZE; i++)
			chunk[i] = pattern(pos + i);
		assert_must(flashfs_fwrite(chunk, 1, CHUNK_SIZE, fp) == CHUNK_SIZE);
	}
	assert_must(flashfs_fclose(fp) == 0);
}

static void check_test_file(void)
{
	int pos, i;
	FLASHFS_FILE* fp = flashfs_fopen("test", "r");
	assert_must(fp != NULL);
	assert_must(flashfs_get_file_size(fp) == TEST_FILE_SIZE);
	for(pos = 0; pos < TEST_FILE_SIZE; pos += CHUNK_SIZE)
	{
		assert_must(flashfs_fread(chunk, 1, CHUNK_SIZE, fp) == CHUNK_SIZE);
		for(i = 0; i < CHUNK_SIZE; i++)
			assert_must(chunk[i] == pattern(pos + i));
	}
	assert_must(!flashfs_feof(fp));
	assert_must(flashfs_fgetc(fp) == EOF);
	assert_must(flashfs_feof(fp));
	assert_must(flashfs_fclose(fp) == 0);
}

static int is_same_string(const char* a, const char* b)
{
	while((*a != '\0') && (*a == *b))
	{
		a++;
		b++;
	}
	return *a == *b;
}

static void check_file_api(void)
{
	char line[16];
	FLASHFS_FILE* fp_list[FLASHFS_MAX_OPEN_FILES];
	FLASHFS_FILE* fp;
	int i;

	fp = flashfs_fopen("text", "w");
	assert_must(fp != NULL);
	assert_must(flashfs_fwrite("one\n", 1, 4, fp) == 4);
	assert_must(flashfs_fclose(fp) == 0);
	assert_must(flashfs_fclose(fp) == EOF);

	fp = flashfs_fopen("text", "a");
	assert_must(fp != NULL);
	assert_must(flashfs_fputc('t', fp) == 't');
	assert_must(flashfs_fwrite("wo\n", 1, 3, fp) == 3);
	assert_must(flashfs_fflush(fp) == 0);
	assert_must(flashfs_fread(line, 1, 1, fp) == 0);
	assert_must(flashfs_ferror(fp) == LFS_ERR_BADF);
	assert_must(flashfs_fclose(fp) == 0);

	fp = flashfs_fopen("text", "r");
	assert_must(fp != NULL);
	assert_must(flashfs_fwrite("x", 1, 1, fp) == 0);
	assert_must(flashfs_ferror(fp) == LFS_ERR_BADF);
	assert_must(flashfs_fgets(line, sizeof(line), fp) == line);
	assert_must(is_same_string(line, "one\n"));
	assert_must(flashfs_ftell(fp) == 4);
	assert_must(flashfs_fgets(line, sizeof(line), fp) == line);
	assert_must(is_same_string(line, "two\n"));
	assert_must(flashfs_fgets(line, sizeof(line), fp) == NULL);
	assert_must(flashfs_feof(fp));
	assert_must(flashfs_fseek(fp, -4, SEEK_END) == 0);
	assert_must(!flashfs_feof(fp));
	assert_must(flashfs_fgetc(fp) == 't');
	flashfs_frewind(fp);
	assert_must(flashfs_ferror(fp) == LFS_ERR_OK);
	assert_must(flashfs_fgetc(fp) == 'o');
	assert_must(flashfs_fclose(fp) == 0);

	fp = flashfs_fopen("text", "r+");
	assert_must(fp != NULL);
	assert_must(flashfs_ftruncate(fp, 3) == 0);
	assert_must(flashfs_get_file_size(fp) == 3);
	assert_must(flashfs_fclose(fp) == 0);

	assert_must(flashfs_fopen("no_file", "r") == NULL);
	assert_must(flashfs_fopen("text", "x") == NULL);

	for(i = 0; i < FLASHFS_MAX_OPEN_FILES; i++)
	{
		fp_list[i] = flashfs_fopen("text", "r");
		assert_must(fp_list[i] != NULL);
	}
	for(i = 0; i < FLASHFS_MAX_OPEN_FILES; i++)
		assert_must(flashfs_fclose(fp_list[i]) == 0);

	assert_must(flashfs_remove("text") == LFS_ERR_OK);
}

static int find_max_block(void* data, lfs_block_t block)
{
	lfs_block_t* max_block = data;
	if(block > *max_block)
		*max_block = block;
	return 0;
}

int main()
{
	flashfs_info_t info;
	flashfs_fsinfo_t fsinfo;
	lfs_block_t max_block = 0;
	int result;

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
	printf("\n[LFS] boot count: %d", update_boot_count());

	check_file_api();
	printf("\n[LFS] file api check passed");

	write_test_file();
	check_test_file();
	printf("\n[LFS] file check passed: %d bytes", TEST_FILE_SIZE);

	assert_must(flashfs_fs_traverse(find_max_block, &max_block) == LFS_ERR_OK);
	printf("\n[LFS] highest block in use: %d", max_block);

	assert_must(flashfs_unmount() == LFS_ERR_OK);
	assert_must(flashfs_mount() == LFS_ERR_OK);
	check_test_file();
	printf("\n[LFS] file check after remount passed");

	assert_must(flashfs_remove("test") == LFS_ERR_OK);
	assert_must(flashfs_stat("test", &info) == LFS_ERR_NOENT);
	assert_must(flashfs_unmount() == LFS_ERR_OK);
	printf("\n[LFS] PASS\n");
	return 0;
}
