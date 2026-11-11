# test_gpio

## Overview

Example of user GPIO: a GPIO added in the platform XML, read by polling or by
interrupt through the PLIC.

- Core `rvc_orca`, 64 kB SRAM, timer, PLIC
- Ready-made slide switch `switch_slide_readymade_0` and a 2-bit user GPIO
  `switch_slide1`
- Default FPGA: `arty-100t`

`test_interrupt` has the same two apps plus self-checking versions. Use it
for interrupt tests; use this platform as the smallest GPIO example.

## Adding a GPIO

1. Add `<gpio>` to `<platform>` in the platform XML. For GPIO interrupts,
   also turn on the PLIC:

   ```
   <spec>
     <define>
       <name>include_plic</name>
       <value type="bool">True</value>
     </define>
   </spec>
   <gpio>
     <name>switch_slide1</name>
     <width>2</width>
   </gpio>
   ```

2. `make syn`. The GPIO becomes a port of the platform and
   `GPIO_INDEX_FOR_SWITCH_SLIDE1` in software.
3. FPGA: map the pins in `user/fpga/#(fpga name)/xdc/` (here
   `user/fpga/arty-100t/xdc/switch_slide.xdc`).
4. RTL simulation: drive the port in `user/sim/include/sim_user_region.vh`.
   This platform steps the switches through a fixed sequence of values
   (000, 001, 010, 100, ...).

Ready-made switches and LEDs are added by count instead:
`num_switch_slide_readymade`, `num_led_checker`.

## Apps

| App | Description |
|---|---|
| `test_gpio_polling_loop` | prints the 3 switch bits every test period; never exits |
| `test_gpio_itr_loop` | prints `gpio 0` on each slide0 rise and `gpio 1: #(value)` while slide1 is 2 or higher; never exits |

Both apps are judged by a person.

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make test_gpio_itr_loop.sim
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
