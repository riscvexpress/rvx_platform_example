# test_user_ip

## Overview

Example of adding user IPs to a platform and accessing a user slave from
software.

- Core `rvc_orca`, 128 kB SRAM
- `i_test1` (`user_slaveif_apb_clkin`): APB slave, 4 kB memory map,
  `TEST1_APB` in `user/rtl/src/test1_apb.v`
- `i_test2` (`user_masterif_axi_clkout`): AXI master interface. No IP is
  connected; the instance in `user/rtl/include/test_user_ip_user_region.vh`
  is a commented template

## Adding a user IP

1. Add an `<ip_instance>` to the platform XML with a user interface library:

   ```
   <ip_instance>
     <name>i_test1</name>
     <library_name>user_slaveif_apb_clkin</library_name>
     <parameter><id>SIZE_OF_MEMORYMAP</id><value>0x1000</value></parameter>
     <parameter><id>BW_ADDR</id><value>32</value></parameter>
     <parameter><id>BW_DATA</id><value>32</value></parameter>
   </ip_instance>
   ```

   `user_slaveif_apb_clkin` is a slave the core accesses;
   `user_masterif_axi_clkout` is a master that accesses memory.

2. `make syn`. `user/template/` gets an instance template for each IP, and
   `user/rtl/include/#(platform name)_user_region.vh` is created.
3. Put the IP's Verilog in `user/rtl/src/` and instantiate it in the user
   region, starting from the template.
4. Software accesses a slave at `#(INSTANCE NAME)_SLAVE_BASEADDR`
   (here `I_TEST1_SLAVE_BASEADDR`) with `REG32()`.

## TEST1_APB registers

| Offset | Access | Function |
|---|---|---|
| 0x0 | R/W | `x` |
| 0x4 | R/W | `y` |
| 0x8 | R/W | `z` |
| 0xC | W | `z = x + y` |
| 0xC | R | `z = x - y` |
| 0x10 | W | `z = x * y + wdata` |

Any other offset returns `pslverr`.

## Apps

| App | Description |
|---|---|
| `test_mmio` | writes `x`=3, `y`=6, `z`=4, runs add, sub and mac and prints `z` after each (4, 9, -3, 21); finally writes to an invalid offset |

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make test_mmio.sim
```
