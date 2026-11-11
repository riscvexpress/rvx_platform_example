#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_external_peri_group_memorymap.h"
#include "ervp_platform_controller_api.h"
#include "trvp_spi_flash.h"
#include "ervp_core_id.h"
#include "flash_info.h"

#if defined(INCLUDE_FLASH_FS) && defined(FLASH_FS_BASEADDR)
#define USE_FLASH_FS_CMD
#include "ervp_flash_fs.h"
#endif

#define ENABLE_LOG 0
#if defined(FLASH_SECTOR_SIZE) && (FLASH_SECTOR_SIZE < 0x2000)
#define BUFFER_SIZE FLASH_SECTOR_SIZE
#else
#define BUFFER_SIZE 0x2000
#endif

#define DATA_INDIVIDUAL 0
#define DATA_ALL_ZERO 1
#define DATA_ALL_ONE 2
#define DATA_NA 3

#define RET_OK 0
#define RET_ERROR -1

#ifdef USE_FLASH_FS_CMD
#define NUM_FS_BLOCKS (FLASH_FS_SIZE/FLASH_SECTOR_SIZE)

static unsigned char used_block[(NUM_FS_BLOCKS+7)/8];
static int fs_is_ready = 0;
static unsigned int fs_cursor = 0;

static int mark_used_block(void* data, lfs_block_t block)
{
	if(block < NUM_FS_BLOCKS)
		used_block[block>>3] |= (1<<(block&7));
	return 0;
}

static int is_used_block(unsigned int block)
{
	return (used_block[block>>3]>>(block&7))&1;
}

static int prepare_fs_blocks()
{
	int i;
	int result;
	for(i=0; i<sizeof(used_block); i++)
		used_block[i] = 0;
	fs_cursor = 0;
	result = flashfs_mount();
	if(result==LFS_ERR_OK)
	{
		result = flashfs_fs_traverse(mark_used_block, 0);
		flashfs_unmount();
	}
	return result;
}

static int read_next_fs_chunk(unsigned char* buffer)
{
	unsigned int offset;
	while(1)
	{
		if(fs_cursor >= FLASH_FS_SIZE)
			return -1;
		if(is_used_block(fs_cursor/FLASH_SECTOR_SIZE))
			break;
		fs_cursor = (fs_cursor/FLASH_SECTOR_SIZE + 1)*FLASH_SECTOR_SIZE;
	}
	offset = fs_cursor;
	spi_flash_read(FLASH_FS_BASEADDR+offset,BUFFER_SIZE,buffer);
	fs_cursor += BUFFER_SIZE;
	return offset;
}

#define FS_PATH_SIZE 256
#define FS_MARGIN_BLOCKS 4

static char fs_path[FS_PATH_SIZE];
static char fs_temp_path[FS_PATH_SIZE+4];
static FLASHFS_FILE* fs_file = 0;

static void finish_fs_file(int removes_temp)
{
	if(fs_file)
		flashfs_fclose(fs_file);
	fs_file = 0;
	if(removes_temp)
		flashfs_remove(fs_temp_path);
	flashfs_unmount();
}

static int make_parent_dir(char* path)
{
	int i;
	int result;
	for(i=1; path[i]!=0; i++)
		if(path[i]=='/')
		{
			path[i] = 0;
			result = flashfs_mkdir(path);
			path[i] = '/';
			if((result!=LFS_ERR_OK) && (result!=LFS_ERR_EXIST))
				return result;
		}
	return LFS_ERR_OK;
}

static int open_fs_file(const char* buffer, unsigned int file_size)
{
	int i;
	lfs_ssize_t used;
	unsigned int need;
	flashfs_info_t info;
	if(fs_file)
		finish_fs_file(1);
	fs_is_ready = 0;
	for(i=0; i<FS_PATH_SIZE; i++)
	{
		fs_path[i] = buffer[i];
		fs_temp_path[i] = buffer[i];
		if(buffer[i]==0)
			break;
	}
	if((i==0) || (i==FS_PATH_SIZE) || (fs_path[0]!='/'))
	{
		printf("\n[RVC|ERROR] invalid fs path");
		return RET_ERROR;
	}
	fs_temp_path[i] = '.';
	fs_temp_path[i+1] = 't';
	fs_temp_path[i+2] = 'm';
	fs_temp_path[i+3] = 'p';
	fs_temp_path[i+4] = 0;
	if(flashfs_mount()!=LFS_ERR_OK)
	{
		printf("\n[RVC|ERROR] flash fs mount failed");
		return RET_ERROR;
	}
	if(flashfs_stat(fs_path,&info)==LFS_ERR_OK)
	{
		printf("\n[RVC|ERROR] %s already exists", fs_path);
		finish_fs_file(0);
		return RET_ERROR;
	}
	flashfs_remove(fs_temp_path);
	used = flashfs_fs_size();
	need = file_size/(FLASH_SECTOR_SIZE-128) + 1 + FS_MARGIN_BLOCKS;
	if((used<0) || ((used+need)>NUM_FS_BLOCKS))
	{
		printf("\n[RVC|ERROR] no space for %d bytes (used %d/%d blocks)", file_size, used, NUM_FS_BLOCKS);
		finish_fs_file(0);
		return RET_ERROR;
	}
	if(make_parent_dir(fs_path)!=LFS_ERR_OK)
	{
		printf("\n[RVC|ERROR] mkdir failed for %s", fs_path);
		finish_fs_file(0);
		return RET_ERROR;
	}
	fs_file = flashfs_fopen(fs_temp_path,"wb");
	if(fs_file==0)
	{
		printf("\n[RVC|ERROR] open failed for %s", fs_temp_path);
		finish_fs_file(1);
		return RET_ERROR;
	}
	printf("\n[RVC|INFO] fs file open %s %d bytes", fs_path, file_size);
	return RET_OK;
}

static int write_fs_file(const unsigned char* buffer, int num_bytes)
{
	if((fs_file==0) || ((unsigned int)num_bytes>BUFFER_SIZE))
	{
		printf("\n[RVC|ERROR] fs file write is not allowed");
		return RET_ERROR;
	}
	if(flashfs_fwrite(buffer,1,num_bytes,fs_file)!=(size_t)num_bytes)
	{
		printf("\n[RVC|ERROR] fs file write failed");
		finish_fs_file(1);
		return RET_ERROR;
	}
	return num_bytes;
}

static int close_fs_file()
{
	long size;
	int result;
	if(fs_file==0)
	{
		printf("\n[RVC|ERROR] no open fs file");
		return RET_ERROR;
	}
	size = flashfs_get_file_size(fs_file);
	result = flashfs_fclose(fs_file);
	fs_file = 0;
	if(result==LFS_ERR_OK)
		result = flashfs_rename(fs_temp_path,fs_path);
	if(result!=LFS_ERR_OK)
	{
		printf("\n[RVC|ERROR] fs file close failed %d", result);
		finish_fs_file(1);
		return RET_ERROR;
	}
	finish_fs_file(0);
	printf("\n[RVC|INFO] fs file %s %d bytes", fs_path, size);
	return size;
}
#endif

static int get_data_pattern(const unsigned char* data, int num_bytes)
{
	int i;
	int pattern;
	if(num_bytes<=0)
		return DATA_INDIVIDUAL;
	if(data[0]==0)
		pattern = DATA_ALL_ZERO;
	else if(data[0]==0xFF)
		pattern = DATA_ALL_ONE;
	else
		return DATA_INDIVIDUAL;
	for(i=1; i<num_bytes; i++)
		if(data[i]!=data[0])
			return DATA_INDIVIDUAL;
	return pattern;
}

int main() {
	int i;
	int cmd;
	int num_bytes;
	int pattern;
	int ret;
	unsigned int flash_addr;
	int offset;
#ifdef SMALL_RAM_BASEADDR
	volatile int* data_array = (volatile int*)(SMALL_RAM_BASEADDR+SMALL_RAM_SIZE-BUFFER_SIZE);
#else
  volatile int* data_array = (volatile int*)(LARGE_RAM_BASEADDR+LARGE_RAM_SIZE-BUFFER_SIZE);
#endif

	if(EXCLUSIVE_ID==0)
	{
#ifdef INCLUDE_SPI_FLASH
		printf("\n[RVC|INFO] flash server is running");
		printf("\n[RVC|INFO] flash base addr: 0x%x", get_flash_binary_baseaddr());
		printf("\n[RVC|INFO] buffer is set to 0x%x", data_array);
		spi_flash_configure();
		set_gpreg(3,DATA_INDIVIDUAL);
		set_gpreg(4,DATA_INDIVIDUAL);
		set_gpreg(0,OCD_FLASH_CMD_IDLE);
		while(1)
		{
			cmd = get_gpreg(0);
			ret = RET_OK;
			switch(cmd)
			{
				case OCD_FLASH_CMD_IDLE:
					break;
				case OCD_FLASH_CMD_CHECK:
					set_gpreg(7,BUFFER_SIZE);
					set_gpreg(6,(int)data_array);
					printf("\n[RVC|INFO] cmd is normally working!");
					break;
				case OCD_FLASH_CMD_ERASE:
					flash_addr = get_gpreg(1);
					printf("\n[RVC|INFO] flash erase 0x%8x", flash_addr);
					spi_flash_erase_sector(flash_addr);
					printf("\n[RVC|INFO] flash erase complete");
					break;
				case OCD_FLASH_CMD_READ:
					flash_addr = get_gpreg(1);
					num_bytes = get_gpreg(2);
					if((unsigned int)num_bytes>BUFFER_SIZE)
					{
						ret = RET_ERROR;
						printf("\n[RVC|ERROR] read size %d is larger than buffer %d", num_bytes, BUFFER_SIZE);
						break;
					}
					spi_flash_read(flash_addr,num_bytes,(unsigned char*)data_array);
					set_gpreg(4,get_data_pattern((unsigned char*)data_array,num_bytes));
					break;
				case OCD_FLASH_CMD_WRITE:
					flash_addr = get_gpreg(1);
					num_bytes = get_gpreg(2);
					pattern = get_gpreg(3);
					set_gpreg(3,DATA_INDIVIDUAL);
					if((unsigned int)num_bytes>BUFFER_SIZE)
					{
						ret = RET_ERROR;
						printf("\n[RVC|ERROR] write size %d is larger than buffer %d", num_bytes, BUFFER_SIZE);
						break;
					}
					printf("\n[RVC|INFO] flash write 0x%x num_bytes: %d first data: 0x%8x", flash_addr, num_bytes, data_array[0]);
          if((flash_addr&3)!=0)
          {
            ret = RET_ERROR;
            printf("\n[RVC|ERROR] addr is NOT aligned to 4");
            break;
          }
					if(pattern==DATA_ALL_ZERO)
						for(i=0; i<num_bytes; i++)
							((unsigned char*)data_array)[i] = 0;
					if(pattern!=DATA_ALL_ONE)
						spi_flash_program(flash_addr,num_bytes,(unsigned char*)data_array);
					if(ENABLE_LOG==1)
						for(i=0; i<num_bytes; i++)
							printf("\n[RVC|INFO] write data: 0x%8x", data_array[i]);
					printf("\n[RVC|INFO] flash write complete");
					break;
#ifdef USE_FLASH_FS_CMD
				case OCD_FLASH_CMD_FS_READ_BLOCK:
					if(fs_file)
						finish_fs_file(1);
					if(fs_is_ready==0)
					{
						if(prepare_fs_blocks()!=LFS_ERR_OK)
						{
							ret = RET_ERROR;
							printf("\n[RVC|ERROR] flash fs mount failed");
							break;
						}
						fs_is_ready = 1;
					}
					offset = read_next_fs_chunk((unsigned char*)data_array);
					if(offset<0)
					{
						set_gpreg(4,DATA_NA);
						fs_is_ready = 0;
					}
					else
					{
						set_gpreg(4,get_data_pattern((unsigned char*)data_array,BUFFER_SIZE));
						ret = offset;
					}
					break;
				case OCD_FLASH_CMD_FS_FILE_OPEN:
					ret = open_fs_file((const char*)data_array,get_gpreg(2));
					break;
				case OCD_FLASH_CMD_FS_FILE_WRITE:
					ret = write_fs_file((const unsigned char*)data_array,get_gpreg(2));
					break;
				case OCD_FLASH_CMD_FS_FILE_CLOSE:
					ret = close_fs_file();
					break;
#endif
				default:
					ret = RET_ERROR;
					printf("\n[RVC|INFO] undefined flash cmd %d", cmd);
			}
			if(cmd!=OCD_FLASH_CMD_IDLE)
			{
				set_gpreg(5,ret);
				set_gpreg(0,OCD_FLASH_CMD_IDLE);
			}
		}
#else
    printf("\n[RVC|ERROR] flash is NOT included");
    set_gpreg(5,RET_ERROR);
    set_gpreg(0,OCD_FLASH_CMD_IDLE);
#endif
	}
	return 0;
}

