# test_ila

## Overview

Example of adding a Xilinx ILA (Integrated Logic Analyzer) to the FPGA
implementation. You need to know how to use the Vivado ILA.

- Core `rvc_orca`, 128 kB SRAM
- ILA description: `user/ila/common/core.xml` probes `rstnn_seqeunce`
  (5 bits) on `i_rtl.clk_system`

## ILA description

Write one XML per ILA in `user/ila/common/` (all boards) or
`user/ila/#(fpga name)/` (one board):

```
<rvx>
  <ila>
    <clock>i_rtl.clk_system</clock>
    <signal>
      <name>rstnn_seqeunce</name>
      <width>5</width>
      <value>rstnn_seqeunce</value>
    </signal>
  </ila>
</rvx>
```

- Write the clock and signal values relative to `#(platform name).v`.
- Hierarchical names are allowed, because the ILA is used only on the FPGA.

In the FPGA directory, run `make ila`, then set `IMP_MODE` from `release`
to `debug` in `set_design.tcl`. Run `make ila` again whenever an ILA XML
changes.

## Apps

| App | Description |
|---|---|
| `hello` | prints `Hello World!` |

## Run

FPGA:

```
make syn
make #(fpga name)
cd imp_#(fpga name)_#(date)
make ila
                                 # set IMP_MODE to debug in set_design.tcl
make imp
make program
```
