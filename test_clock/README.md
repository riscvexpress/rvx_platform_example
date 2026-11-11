# test_clock

## Overview

Example of the `<clock_info>` block in the platform XML: clocks are named,
given a frequency, and assigned to a clock domain by alias.

- Core `rvc_orca`, 128 kB SRAM
- `clk_100M` = 100 MHz, `clk_200M` = 200 MHz
- `clk_core` = `clk_100M`, `clk_noc` = `clk_200M`

## Clock settings

Add `<clock_info>` to `<platform>` in the platform XML. Define each clock
with `type="dec"` (frequency in Hz), and assign a clock domain to it with
`type="equal"`:

```
<clock_info>
  <define>
    <name>clk_core</name>
    <value type="equal">clk_100M</value>
  </define>
  <define>
    <name>clk_noc</name>
    <value type="equal">clk_200M</value>
  </define>
  <define>
    <name>clk_100M</name>
    <value type="dec">100000000</value>
  </define>
  <define>
    <name>clk_200M</name>
    <value type="dec">200000000</value>
  </define>
</clock_info>
```

After `make syn`, the clocks are separate ports of
`arch/rtl/src/test_clock_rtl.v`, and the core clock is `CORE_CLK_HZ` in
`arch/rtl/include/hw_info.vh`.

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
make hello.sim
```
