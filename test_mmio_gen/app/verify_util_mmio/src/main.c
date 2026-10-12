#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_mmio_util.h"
#include "demo_ip_memorymap.h"

// The user RTL of i_demo is written in "test_mmio_gen_user_region.vh".
// cap = {wide_out[47:32], stall_count, start_count, tick_count, kick_acc}

#define CAP_KICK(x) ((x)&0xF)
#define CAP_TICK(x) (((x)>>4)&0xF)
#define CAP_START(x) (((x)>>8)&0xF)
#define CAP_STALL(x) (((x)>>12)&0xF)
#define CAP_WIDE_OUT_HIGH(x) (((x)>>16)&0xFFFF)

int main()
{
  unsigned int value;

  // simple: reset value, then 8-bit storage
  value = mmio_read_data(MMAP_CTRL_CONFIG);
  printf("config reset 0x%x\n", value);
  assert(value==5);
  mmio_write_data(MMAP_CTRL_CONFIG, 0x1A5);
  value = mmio_read_data(MMAP_CTRL_CONFIG);
  printf("config 0x%x\n", value);
  assert(value==0xA5);

  // read with readyin: the user RTL holds the access for 10 cycles
  value = mmio_read_data(MMAP_CTRL_WAIT_DONE);
  assert(value==1);
  value = mmio_read_data(MMAP_CTRL_CAP);
  printf("wait_done stalled %d cycles\n", CAP_STALL(value));
  assert(CAP_STALL(value)==10);

  // write_cmd, read_cmd, onehot_cmd
  mmio_write_data(MMAP_CTRL_START, 0);
  mmio_write_data(MMAP_CTRL_START, 0);
  mmio_write_data(MMAP_CTRL_START, 0);
  mmio_read_data(MMAP_CTRL_TICK);
  mmio_read_data(MMAP_CTRL_TICK);
  mmio_write_data(MMAP_CTRL_KICK, DEMO_KICK_A);
  mmio_write_data(MMAP_CTRL_KICK, DEMO_KICK_C);
  value = mmio_read_data(MMAP_CTRL_CAP);
  printf("start %d tick %d kick 0x%x\n", CAP_START(value), CAP_TICK(value), CAP_KICK(value));
  assert(CAP_START(value)==3);
  assert(CAP_TICK(value)==2);
  assert(CAP_KICK(value)==(DEMO_KICK_A|DEMO_KICK_C));

  // general array: the user RTL returns the inverted value
  assert(NUM_CTRL_DATA==2);
  mmio_write_data(MMAP_CTRL_DATA00, 0x12345678);
  mmio_write_data(MMAP_CTRL_DATA01, 0);
  assert(mmio_read_data(MMAP_CTRL_DATA00)==~0x12345678u);
  assert(mmio_read_data(MMAP_CTRL_DATA01)==0xFFFFFFFF);

  // general with regvalue
  mmio_write_data(MMAP_CTRL_MODE, CTRL_MODE_STOP);
  assert(mmio_read_data(MMAP_CTRL_MODE)==CTRL_MODE_STOP);

  // read_reg wider than the bus: lower word first, then wrap around
  {
    unsigned int w0 = mmio_read_data(MMAP_CTRL_WIDE_IN);
    unsigned int w1 = mmio_read_data(MMAP_CTRL_WIDE_IN);
    unsigned int w2 = mmio_read_data(MMAP_CTRL_WIDE_IN);
    printf("wide_in 0x%08x 0x%08x 0x%08x\n", w0, w1, w2);
    assert(w0==0x55667788);
    assert(w1==0x11223344);
    assert(w2==0x55667788);
  }

  // write_reg wider than the bus: lower word first
  mmio_write_data(MMAP_CTRL_WIDE_OUT, 0xAABBCCDD);
  mmio_write_data(MMAP_CTRL_WIDE_OUT, 0x1234);
  value = mmio_read_data(MMAP_CTRL_VER);
  printf("wide_out low 0x%08x\n", value);
  assert(value==0xAABBCCDD);
  value = mmio_read_data(MMAP_CTRL_CAP);
  printf("wide_out high 0x%04x\n", CAP_WIDE_OUT_HIGH(value));
  assert(CAP_WIDE_OUT_HIGH(value)==0x1234);

  // wfifo -> user RTL (+1) -> rfifo
  value = mmio_read_data(MMAP_CTRL_CMDQ);
  printf("cmdq read 0x%x\n", value);
  assert(value==4);
  for(int i=0; i<3; i++)
    mmio_write_data(MMAP_CTRL_CMDQ, 100+i);
  value = mmio_read_data(MMAP_CTRL_RESULT_VALID_NUM);
  printf("result valid_num %d\n", value);
  assert(value==3);
  for(int i=0; i<3; i++)
  {
    value = mmio_read_data(MMAP_CTRL_RESULT);
    printf("result %d\n", value);
    assert(value==(101+i));
  }
  assert(mmio_read_data(MMAP_CTRL_RESULT_VALID_NUM)==0);
  assert(mmio_read_data(MMAP_CTRL_CMDQ)==4);

  printf("verify_util_mmio passed\n");
  return 0;
}
