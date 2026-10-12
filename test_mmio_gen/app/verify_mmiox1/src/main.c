#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"
#include "ervp_mmiox1.h"
#include "ervp_assert.h"
#include "ip_instance_info.h"

// the behavior of the accelerator is written in "XXXX_user_region.vh" file.

uint64_t config_data = 0x01020304050607;
uint32_t config_to_verify[2];

// only "unsigned int" and "int" is allowed when bit field is specified.
typedef struct {
  unsigned int waddr: 32;
  unsigned int wnum: 8;
  unsigned int opcode: 8;
} MyAccInst;

MyAccInst inst[2];

uint32_t acc_core_status;

const uint64_t acc_input_data[3] = {
  1, 3, 5
};

uint64_t acc_output_data[3];

uint32_t acc_core_log;

int main()
{
  inst[0].waddr = 0x30000000;
  inst[0].wnum = 4;
  inst[0].opcode = 7;
  inst[1].waddr = 0xF0000000;
  inst[1].wnum = 8;
  inst[1].opcode = 2;
  mmiox1_config_write(i_test_mmio_gen_mmiox_info, &config_data);
  mmiox1_config_read(i_test_mmio_gen_mmiox_info, config_to_verify);
  assert((*((uint64_t*)config_to_verify))==config_data);
  mmiox1_inst_push(i_test_mmio_gen_mmiox_info, inst, 2, 0);
  assert(mmiox1_inst_num_vacant(i_test_mmio_gen_mmiox_info)==8);
  assert(mmiox1_inst_num_busy(i_test_mmio_gen_mmiox_info)==0); // due to sim code
  assert(mmiox1_core_has_log(i_test_mmio_gen_mmiox_info)==1); // due to sim code
  mmiox1_core_log_read(i_test_mmio_gen_mmiox_info, &acc_core_log);
  assert(acc_core_log==0xdddd); // due to sim code
  mmiox1_core_status_read(i_test_mmio_gen_mmiox_info, &acc_core_status);
  assert(acc_core_status==0x010a); // due to sim code
  mmiox1_input_push(i_test_mmio_gen_mmiox_info, acc_input_data, 3);
  assert(mmiox1_input_num_fifo(i_test_mmio_gen_mmiox_info)==(5-3));
  mmiox1_inst_push(i_test_mmio_gen_mmiox_info, inst, 2, 0);
  assert(mmiox1_inst_num_busy(i_test_mmio_gen_mmiox_info)==1); // due to sim code
  assert(mmiox1_output_num_fifo(i_test_mmio_gen_mmiox_info)==3); // due to sim code
  //mmiox1_output_pop(i_test_mmio_gen_mmiox_info, acc_output_data, 3); // if num_output is fixed
  int output_index = 0;
  while(1)
  {
    int num_output = mmiox1_output_num_fifo(i_test_mmio_gen_mmiox_info);
    if(num_output==0)
      break;
    mmiox1_output_pop(i_test_mmio_gen_mmiox_info, &(acc_output_data[output_index]), num_output);
    output_index += num_output;
  }
  
	for(int i=0; i<3; i++)
		assert(acc_output_data[i]==acc_input_data[i]);

	mmiox1_core_clear(i_test_mmio_gen_mmiox_info);
	return 0;
}
