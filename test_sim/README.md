# test_sim

## Overview

Example of simulation options in the platform XML: the waveform is limited
to the platform RTL.

- Core `rvc_orca`, 128 kB SRAM

## Waveform settings

Add `<record_top>` to `<sim_define>` in the platform XML:

```
<sim_define>
  <record_top>"depth=20",i_platform.i_rtl</record_top>
</sim_define>
```

It becomes `RECORD_TOP` in `arch/rtl/include/sim_info.vh`. Without it, the
waveform is recorded from the testbench top, 20 levels deep.

The waveform is recorded only by `make #(app).debug`, to `sim_rtl/wave/`.

## Apps

| App | Description |
|---|---|
| `hello` | prints `Hello World!` |

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make hello.sim      # no waveform
make hello.debug    # records the waveform to wave/
```
