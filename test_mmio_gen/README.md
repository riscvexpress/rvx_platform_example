# test_mmio_gen

## Overview

Example of the two ways to give a user IP its registers: the ready-made
`user_mmiox1` block, and registers generated from an XML description.

- Core `rvc_orca`, 128 kB SRAM
- `i_test_mmio_gen` (`user_mmiox1`): 62-bit instruction, 64-bit config,
  32-bit status, 64-bit input and output
- `i_demo` (APB slave): registers generated from `util/mmio/demo_ip.xml`,
  logic in `user/rtl/include/test_mmio_gen_user_region.vh`

## Adding user_mmiox1

`user_mmiox1` has config/status registers, an instruction FIFO,
input/output FIFOs and a log FIFO. Add it to the platform XML and set the
widths and FIFO depths:

```
<ip_instance>
  <name>i_test_mmio_gen</name>
  <library_name>user_mmiox1</library_name>
  <parameter><id>BW_INST</id><value>62</value></parameter>
  <parameter><id>INST_FIFO_DEPTH</id><value>8</value></parameter>
  <parameter><id>LOG_FIFO_DEPTH</id><value>2</value></parameter>
  <parameter><id>INPUT_FIFO_DEPTH</id><value>5</value></parameter>
  <parameter><id>BW_CONFIG</id><value>64</value></parameter>
  <parameter><id>BW_STATUS</id><value>32</value></parameter>
  ...
</ip_instance>
```

After `make syn`, connect your logic in
`user/rtl/include/#(platform name)_user_region.vh` (the instance template is
in `user/template/`). Software uses `ervp_mmiox1.h` with
`#(instance name)_mmiox_info`.

## MMIO generation

1. Add an APB slave to the platform XML:

   ```
   <ip_instance>
     <name>i_demo</name>
     <library_name>user_slaveif_apb_clkin</library_name>
     <parameter><id>SIZE_OF_MEMORYMAP</id><value>0x1000</value></parameter>
     <parameter><id>BW_ADDR</id><value>32</value></parameter>
     <parameter><id>BW_DATA</id><value>32</value></parameter>
   </ip_instance>
   ```

2. Describe the registers in `util/mmio/#(ip name).xml`, and set the IP name
   and base address in `util/mmio/each_mmio.mh`:

   ```
   MMIO_NAME=demo_ip
   BASEADDR=I_DEMO_SLAVE_BASEADDR
   ```

   `demo_ip.xml` uses every register type: `simple`, `read` (with
   `readyin`), `write_cmd`, `read_cmd`, `onehot_cmd`, `general` (array and
   `regvalue`), `read_reg` and `write_reg` wider than the bus, `wfifo`,
   `rfifo` and `fake`.

3. Run `make` in `util/mmio`. It writes:

   | File | To |
   |---|---|
   | `demo_ip_mmio.v` | `user/rtl/src/` |
   | `demo_ip_memorymap_offset.vh` | `user/rtl/include/` |
   | `demo_ip_memorymap*.h` | `user/api/` |

4. Instantiate `DEMO_IP_MMIO` in the user region and connect your logic.
   Software accesses the registers with `mmio_read_data()` and
   `mmio_write_data()` on the `MMAP_*` addresses.

To add `util/mmio` to a new platform, run `make mmio` in `util`.

## Apps

### verify_util_mmio

- **Purpose:** every generated register type (`make verify`).
- **Method:** reads the reset value and writes `simple`; checks that a
  `readyin` read stalls 10 cycles; counts `write_cmd`, `read_cmd` and
  `onehot_cmd` accesses; the user RTL returns the inverted value of the
  `general` array, the lower word first for wide registers, and the `wfifo`
  data plus 1 through the `rfifo`. Prints `verify_util_mmio passed`.

### verify_mmiox1

- **Purpose:** the `mmiox1` API.
- **Method:** config write and read-back, instruction push, status and log
  read, input push and output pop. The simulation model in
  `user/sim/include/sim_user_region.vh` returns fixed status and log values
  and loops the input back to the output.

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_util_mmio.sim
```
