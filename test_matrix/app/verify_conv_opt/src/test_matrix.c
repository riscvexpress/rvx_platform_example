#include "test_matrix.h"

#include "core_dependent.h"
#include "ervp_variable_allocation.h"
#include "ervp_float.h"
#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_matrix_element.h"

#define NUM_SEED 5

// Deterministic per-element hash so that a given (seed, row, col) always
// produces the same value, but the values within a matrix are well mixed
// instead of a monotonic linear ramp.
static unsigned int test_matrix_hash(int seed, int row, int col)
{
  unsigned int h = 0x811c9dc5u;
  h = (h ^ (unsigned int)seed) * 0x01000193u;
  h = (h ^ (unsigned int)row) * 0x01000193u;
  h = (h ^ (unsigned int)col) * 0x01000193u;
  // Final avalanche mix (splitmix-style) so nearby indices diverge strongly.
  h ^= h >> 16;
  h *= 0x7feb352du;
  h ^= h >> 15;
  h *= 0x846ca68bu;
  h ^= h >> 16;
  return h;
}

// Signed pseudo-random fraction in the range [-1.0, 1.0).
static float test_matrix_unit(unsigned int h)
{
  return (float)((int)(h & 0xffffff) - 0x800000) / (float)0x800000;
}

// Signed pseudo-random integer in [-range, range].
static int test_matrix_int(unsigned int h, int range)
{
  int span = 2 * range + 1;
  return (int)(h % (unsigned int)span) - range;
}

void generate_test_matrix(ErvpMatrixInfo *matrix_info, int seed)
{
  int i, j;
  int seed2 = seed % NUM_SEED;

  assert(matrix_info);
  assert(matrix_info->addr);

  if (matrix_info->datatype == MATRIX_DATATYPE_FLOAT32)
  {
    for (j = 0; j < matrix_info->num_row; j++)
      for (i = 0; i < matrix_info->num_col; i++)
      {
        unsigned int h = test_matrix_hash(seed, j, i);
        float unit = test_matrix_unit(h); // [-1,1)
        float value;
        switch (seed2)
        {
        case 0:
          // Large-magnitude values, with occasional extra-large outliers.
          value = unit * (((h >> 28) == 0) ? 65536.0f : 4355.0f);
          break;
        case 1:
          // Very small / near-denormal magnitudes.
          value = unit / (7.0f * 1024.0f * 1024.0f * 256.0f);
          break;
        case 2:
          // Moderate fractional values.
          value = unit * 7.0f;
          break;
        case 3:
          // Mixed small integers embedded in floats.
          value = (float)test_matrix_int(h, 8);
          break;
        case 4:
          // Tight range of small integers, including zeros.
          value = (float)test_matrix_int(h, 4);
          break;
        default:
          value = 0;
          assert(0);
        }
        matrix_write_float_element(matrix_info, j, i, value);
      }
  }
  else
  {
    for (j = 0; j < matrix_info->num_row; j++)
      for (i = 0; i < matrix_info->num_col; i++)
      {
        unsigned int h = test_matrix_hash(seed, j, i);
        int value;
        switch (seed2)
        {
        case 0:
          value = test_matrix_int(h, 127);
          break;
        case 1:
          value = test_matrix_int(h, 32);
          break;
        case 2:
          value = test_matrix_int(h, 100);
          break;
        case 3:
          value = test_matrix_int(h, 8);
          break;
        case 4:
          value = test_matrix_int(h, 4);
          break;
        default:
          value = 0;
          assert(0);
        }
        matrix_write_fixed_element(matrix_info, j, i, value);
      }
  }
}