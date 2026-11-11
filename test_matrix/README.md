# test_matrix

## Overview

Template for checking a matrix accelerator against the software matrix
library. Each app maps matrix operations to your hardware, runs them, and
compares the result with the software reference (`matrix_*_sw`). Without a
mapping, the software functions are used, so every app passes on this
platform as is.

- Core `rvc_orca`, 128 kB SRAM, slow DRAM
- Matrices are placed in `BIG_DATA_BSS`, aligned to 4 kB

## Using your hardware

In `src/map_your_matrix_hw.c` of the app, set `matrix_hw_name` and assign
your functions in `map_your_matrix_function()`:

```
const char matrix_hw_name[] = "MY_HW";

void map_your_matrix_function(ervp_mop_mapping_t* mop_mapping)
{
  mop_mapping->matrix_mult = your_matrix_mult;
}
```

Operations left unassigned run in software. A mismatch prints the inputs,
the output and the reference, and fails.

Turn an operation off with its `VERIFY_*` define in `main.c`. The matrix
size and data types are also defines at the top of `main.c`.

## Apps

| App | Description |
|---|---|
| `verify_matrix_basic` | 8x8 int8 add, sub, ewmult, mult with a zero, one or identity matrix, so the expected result is the input (`make verify`) |
| `verify_gemm_opt` | 10x10 int8 add, sub, ewmult, mult and mult with accumulate, random matrices |
| `verify_blocked_gemm_opt` | the same without accumulate, 17x17, to test matrices larger than one hardware block |
| `verify_conv_opt` | 8x8 int32 input, 3x3 kernel convolution, with and without accumulate |

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_matrix_basic.sim
```
