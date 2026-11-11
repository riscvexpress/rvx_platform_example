# test_profiling

## Overview

Checks the timing functions: `delay_*`, the real clock and the profiling
API (`ervp_profiling.h`).

- Core `rvc_orca_plus`, 2 user cores, 128 kB SRAM, timer, 2 locks and barriers
- Default FPGA: `arty-100t`

## Profiling settings

Turn profiling on in `<sw_define>` of the platform XML.
`silent_for_profiling` keeps printf from disturbing the measurement:

```
<sw_define>
  <use_profiling type="bool">True</use_profiling>
  <silent_for_profiling type="bool">True</silent_for_profiling>
</sw_define>
```

Mark a section in an app by name:

```
profiling_start_by_name("my_section");
...
profiling_end("my_section");
```

Profiling counts `mcycle` when the core has it (`INCLUDE_MCYCLE`), the real
clock (`TICK_HZ`) otherwise.

## Apps

### verify_delay

- **Purpose:** `delay_us`, `delay_ns`, `delay_cycle` and the real clock.
- **Method:** measures `delay_us` of 1 us to 5 ms with the real clock; it must
  be at least the requested time and at most 10 us + 1 % longer. With
  `mcycle`, also checks `delay_cycle` and `delay_ns` within 100 cycles.
  Prints `[verify_delay] PASS`.

### verify_profiling

- **Purpose:** profiling sections. `make verify` runs it.
- **Method:** repeats sections of 100 us, 1 ms and 5 ms; each must have the
  right call count and an average within 20 us + 1 %. A nested section must
  include the inner sections and the profiling overhead. Prints
  `[verify_profiling] PASS`.

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_delay.sim
make verify_profiling.sim
```
