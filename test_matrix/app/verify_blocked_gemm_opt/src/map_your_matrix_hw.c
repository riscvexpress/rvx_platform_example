#include "platform_info.h"
#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_blocked_matrix_op.h"

#include "map_your_matrix_hw.h"

const char matrix_hw_name[] = "BLOCKED_SW";
const int BLOCK_SIZE = 4;

static ervp_blocked_matrix_info_t *blocked_info;

ervp_hwtask_busy_fx_t i_blocked_matrix_add(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  // flush_cache();
  return blocked_matrix_op(blocked_info->block_size, mop_mapping->matrix_add, blocked_info->subop_mapping, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t i_blocked_matrix_sub(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  // flush_cache();
  return blocked_matrix_op(blocked_info->block_size, mop_mapping->matrix_sub, blocked_info->subop_mapping, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t i_blocked_matrix_ewmult(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  // flush_cache();
  return blocked_matrix_op(blocked_info->block_size, mop_mapping->matrix_ewmult, blocked_info->subop_mapping, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t i_blocked_matrix_mult(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  // flush_cache();
  return blocked_matrix_mult(blocked_info, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t i_blocked_matrix_conv(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int conv_options)
{
  // flush_cache();
  return blocked_matrix_conv(blocked_info, ma_info, mb_info, mc_info, conv_options);
}

void map_your_matrix_function(ervp_mop_mapping_t *mop_mapping)
{
  /* map your own functions */
  blocked_info = blocked_matrix_info_alloc();
  blocked_info->block_size = BLOCK_SIZE;
  blocked_info->subop_mapping = matrix_op_mapping_alloc();

  mop_mapping->matrix_add = i_blocked_matrix_add;
  mop_mapping->matrix_sub = i_blocked_matrix_sub;
  mop_mapping->matrix_ewmult = i_blocked_matrix_ewmult;
  mop_mapping->matrix_mult = i_blocked_matrix_mult;
  mop_mapping->matrix_conv = i_blocked_matrix_conv;
}
