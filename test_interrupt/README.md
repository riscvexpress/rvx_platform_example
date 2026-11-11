# test_interrupt

## Overview

Reference platform for every interrupt source in RVX: software, timer, and
external interrupts through the PLIC (GPIO, user IP). Each app shows one
source in a small, self-checking example, so others can copy the part they
need and extend it.

- Core `rvc_orca_plus`, 2 user cores, 64 kB SRAM, slow DRAM, timer, PLIC
- Ready-made slide switch `switch_slide_readymade_0` and a 2-bit user GPIO
  `switch_slide1`
- User IP `i_itr_gen` (`user/rtl/src/itr_gen_apb.v`): raises and lowers 4
  user interrupt lines from software
- Default FPGA: `arty-100t`

The switches are driven by `user/sim/include/sim_user_region.vh` in RTL
simulation.

## Interrupt sources

Each source needs these settings in `<spec>` of the platform XML:

| Source | XML |
|---|---|
| Timer | `include_timer` = True |
| GPIO | `include_plic` = True, and `<gpio>` or `num_switch_slide_readymade` |
| User IP | `include_plic` = True, `include_user_interrupts` = True |
| Software (between cores) | core `rvc_orca_plus` or `rvc_rocket_opt`, `num_core_user` = 2 |

For example:

```
<define>
  <name>include_plic</name>
  <value type="bool">True</value>
</define>
<define>
  <name>include_user_interrupts</name>
  <value type="bool">True</value>
</define>
```

A user IP drives the PLIC through `plic_user_interrupts` (32 bits) in
`user/rtl/include/#(platform name)_user_region.vh`:

```
assign plic_user_interrupts = {{(32-4){1'b 0}}, itr_gen_interrupt};
```

In software, register the ISR, then allow the PLIC and enable the trap.
Never set `mstatus.MIE` directly; use `enable_interrupt_trap()`.

```
register_isr_user(index, isr, priority);    // or register_isr_gpio()
register_plic_grant();
allow_interrupt_plic();
enable_interrupt_trap();
```

## Apps

### verify_interrupt_all

- **Purpose:** check every interrupt function in one run. `make verify` runs it.
- **Method:** core 0 runs the checks of the apps below in order (GPIO,
  GPIO debouncing, timer, wfi, user IP and PLIC priority, PLIC masking,
  software interrupt) and prints `PASS` for each. Turn a step off with its
  `CHECK_*` define in `main.c`. PLIC masking uses a user interrupt line,
  because the switch sequence runs only once per simulation.

### verify_timer_itr

- **Purpose:** timer interrupt.
- **Method:** periodic mode must interrupt every 200 us; after `stop_timer()`
  no interrupt may come; one-shot mode must interrupt exactly once.

### verify_gpio_itr

- **Purpose:** GPIO interrupt through the PLIC, with edge and level conditions.
- **Method:** the switch sequence raises slide0 twice and steps slide1 through
  1, 2, 3, 0. RISE on slide0 must fire once per rising edge. EQ on slide1 is a
  level condition; the ISR re-arms it with the next value, and each value must
  be seen once.

### verify_gpio_debounce

- **Purpose:** GPIO debouncing.
- **Method:** enabling debouncing on `switch_slide1` starts the testbench's
  debounce stimulus: 3 presses with contact bounce. RISE on slide0 must fire
  exactly 3 times.

### verify_plic_mask_priority

- **Purpose:** PLIC masking and priority.
- **Method:** slide0 rises while masked at the PLIC: no interrupt, and the
  held interrupt is delivered on unmask. Then slide0 rises and slide1 falls
  together while interrupts are disabled; on enable, the higher-priority
  source (slide1) must be served first.

### verify_user_itr

- **Purpose:** interrupts from a user IP.
- **Method:** software raises each of the 4 lines through `i_itr_gen`; each
  must call its own ISR, which lowers the line. Then all 4 are raised at once;
  they must be served in priority order.

### verify_wfi

- **Purpose:** `wfi` stops the core until an interrupt is pending.
- **Method:** a one-shot timer fires during `wfi`. With `mstatus.MIE` on, the
  ISR must have run when `wfi` returns. With it off, `wfi` must still wake up
  (MTIP pending) without running the ISR, and the ISR runs once interrupts
  are enabled. Fails on a core whose `wfi` is a no-op.

### verify_sw_interrupt

- **Purpose:** software interrupt between cores (`rvc_orca_plus` or
  `rvc_rocket_opt`, exactly 2 cores).
- **Method:** core 0 and core 1 send each other a software interrupt 4 times,
  each waiting with `wfi`. Both must receive 4.

### test_gpio_itr_loop

- **Purpose:** GPIO interrupt, judged by a person (FPGA or RTL simulation).
- **Method:** prints `gpio 0` on each slide0 rise and `gpio 1` while slide1
  is 2 or higher; never exits.

### test_gpio_polling_loop

- **Purpose:** GPIO input without interrupts, judged by a person.
- **Method:** prints the switch values every test period; never exits.

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_interrupt_all.sim
```

FPGA:

```
make arty-100t
cd imp_arty-100t_#(date)
make imp
make program
make printf                      # in another shell
make test_gpio_itr_loop.run
```
