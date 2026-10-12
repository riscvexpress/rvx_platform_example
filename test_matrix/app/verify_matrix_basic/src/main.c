#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_printf_section.h"
#include "ervp_variable_allocation.h"
#include "ervp_special_matrix_op.h"
#include "ervp_matrix.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_core_id.h"
#include "ervp_assert.h"

#include "test_matrix.h"
#include "map_your_matrix_hw.h"

///////////////////////////////////////////////////////////////

#define NUM_MATRIX 1
#define TEST_MATRIX_SIZE 8

#define VERIFY_ADD 1
#define VERIFY_SUB 1
#define VERIFY_EWMULT 1
#define VERIFY_MULT 1

#define ML_DATATYPE MATRIX_DATATYPE_SINT08
#define MR_DATATYPE MATRIX_DATATYPE_SINT08
#define MO_DATATYPE MATRIX_DATATYPE_SINT08

#if (GET_NUM_BITS(ML_DATATYPE)==32)
typedef uint32_t ml_type_t;
#elif (GET_NUM_BITS(ML_DATATYPE)==16)
typedef uint16_t ml_type_t;
#else
typedef uint8_t ml_type_t;
#endif

#if (GET_NUM_BITS(MR_DATATYPE)==32)
typedef uint32_t mr_type_t;
#elif (GET_NUM_BITS(MR_DATATYPE)==16)
typedef uint16_t mr_type_t;
#else
typedef uint8_t mr_type_t;
#endif

#if (GET_NUM_BITS(MO_DATATYPE)==32)
typedef uint32_t mo_type_t;
#elif (GET_NUM_BITS(MO_DATATYPE)==16)
typedef uint16_t mo_type_t;
#else
typedef uint8_t mo_type_t;
#endif

#define SKIP_SIM 1

#define ALIGNED_MATRIX __attribute__ ((aligned(0x1000)))

ml_type_t input_left_array[NUM_MATRIX][TEST_MATRIX_SIZE][TEST_MATRIX_SIZE] BIG_DATA_BSS ALIGNED_MATRIX;
mr_type_t input_right_array[NUM_MATRIX][TEST_MATRIX_SIZE][TEST_MATRIX_SIZE] BIG_DATA_BSS ALIGNED_MATRIX;
mo_type_t output_array[NUM_MATRIX][TEST_MATRIX_SIZE][TEST_MATRIX_SIZE] BIG_DATA_BSS ALIGNED_MATRIX;
mo_type_t ref_array[NUM_MATRIX][TEST_MATRIX_SIZE][TEST_MATRIX_SIZE] BIG_DATA_BSS ALIGNED_MATRIX;

ErvpMatrixInfo* input_left_info = NULL;
ErvpMatrixInfo* input_right_info = NULL;
ErvpMatrixInfo* output_info = NULL;
ErvpMatrixInfo* ref_info = NULL;

void matrix_info_init()
{
  input_left_info = matrix_generate_info(ML_DATATYPE,TEST_MATRIX_SIZE,TEST_MATRIX_SIZE,NULL,NULL);
  input_right_info = matrix_generate_info(MR_DATATYPE,TEST_MATRIX_SIZE,TEST_MATRIX_SIZE,NULL,NULL);
  output_info = matrix_generate_info(MO_DATATYPE,TEST_MATRIX_SIZE,TEST_MATRIX_SIZE,NULL,NULL);
  //ref_info = matrix_generate_info(MO_DATATYPE,TEST_MATRIX_SIZE,TEST_MATRIX_SIZE,NULL,NULL);
}

void matrix_info_setup(int index)
{
  input_left_info->addr = (void*)(input_left_array[index]);
  input_right_info->addr = (void*)(input_right_array[index]);
  output_info->addr = (void*)(output_array[index]);
  //ref_info->addr = (void*)(ref_array[index]);
}

int main()
{
  if(EXCLUSIVE_ID==0)
  {
    ervp_hwtask_busy_fx_t hwtask_busy_fx;
    ervp_mop_mapping_t* mop_mapping = matrix_op_mapping_alloc();
    map_your_matrix_function(mop_mapping);
    matrix_info_init();

    // init matrices
    for(int i=0; i<TEST_MATRIX_SIZE; i=i+1)
      for(int j=0; j<TEST_MATRIX_SIZE; j=j+1)
        input_left_array[0][i][j] = i + j;
    for(int i=1; i<NUM_MATRIX; i=i+1)
    {
      matrix_info_setup(i);
      generate_test_matrix(input_left_info, i);
      //generate_test_matrix(input_right_info, i+1);
    }
    flush_cache();

    //
    if(VERIFY_ADD)
    {
      printf_section(SKIP_SIM, "%s_ADD", matrix_hw_name);
      for(int i=0; i<NUM_MATRIX; i=i+1)
      {
        flush_cache();
        matrix_info_setup(i);
        matrix_zero_sw(input_right_info);
        matrix_zero_sw(output_info);
        ref_info = input_left_info;
        hwtask_busy_fx = mop_mapping->matrix_add(mop_mapping, input_left_info, input_right_info, output_info, 0);
        hwtask_wait_complete(hwtask_busy_fx);
        int all_are_equal = matrix_compare(output_info, ref_info, 1);
        if(!all_are_equal)
        {
          matrix_print(input_left_info);
          matrix_print(input_right_info);
          matrix_print(output_info);
          matrix_print(ref_info);
          assert(0);
          break;
        }
      }
    }
    
    //
    if(VERIFY_SUB)
    {
      printf_section(SKIP_SIM, "%s_SUB", matrix_hw_name);
      for(int i=0; i<NUM_MATRIX; i=i+1)
      {
        flush_cache();
        matrix_info_setup(i);
        matrix_zero_sw(input_right_info);
        matrix_zero_sw(output_info);
        ref_info = input_left_info;
        hwtask_busy_fx = mop_mapping->matrix_sub(mop_mapping, input_left_info, input_right_info, output_info, 0);
        hwtask_wait_complete(hwtask_busy_fx);
        int all_are_equal = matrix_compare(output_info, ref_info, 1);
        if(!all_are_equal)
        {
          matrix_print(input_left_info);
          matrix_print(input_right_info);
          matrix_print(output_info);
          matrix_print(ref_info);
          assert(0);
          break;
        }
      }
    }
    
    //
    if(VERIFY_EWMULT)
    {
      printf_section(SKIP_SIM, "%s_EWMULT", matrix_hw_name);
      for(int i=0; i<NUM_MATRIX; i=i+1)
      {
        flush_cache();
        matrix_info_setup(i);
        matrix_one_sw(input_right_info);
        matrix_zero_sw(output_info);
        ref_info = input_left_info;
        hwtask_busy_fx = mop_mapping->matrix_ewmult(mop_mapping, input_left_info, input_right_info, output_info, 0);
        hwtask_wait_complete(hwtask_busy_fx);
        int all_are_equal = matrix_compare(output_info, ref_info, 1);
        if(!all_are_equal)
        {
          matrix_print(input_left_info);
          matrix_print(input_right_info);
          matrix_print(output_info);
          matrix_print(ref_info);
          assert(0);
          break;
        }
      }
    }

    //
    if(VERIFY_MULT)
    {
      printf_section(SKIP_SIM, "%s_MULT", matrix_hw_name);
      for(int i=0; i<NUM_MATRIX; i=i+1)
      {
        flush_cache();
        matrix_info_setup(i);
        matrix_identity_sw(input_right_info);
        matrix_zero_sw(output_info);
        ref_info = input_left_info;
        hwtask_busy_fx = mop_mapping->matrix_mult(mop_mapping, input_left_info, input_right_info, output_info, 0);
        hwtask_wait_complete(hwtask_busy_fx);
        int all_are_equal = matrix_compare(output_info, ref_info, 1);
        if(!all_are_equal)
        {
          matrix_print(input_left_info);
          matrix_print(input_right_info);
          matrix_print(output_info);
          matrix_print(ref_info);
          assert(0);
          break;
        }
      }
    }

    flush_cache();
  }
  return 0;
}
