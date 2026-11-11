# test_flash

## Overview

Platform for testing the **rvx flash** (SPI flash for app binaries and the file
system) and the flash file system (littlefs, `rvx_ssw/driver/flash_fs`).
Use it to bring up the flash on a new board.

- Core `rvc_orca`, 128 kB SRAM, system clock 100 MHz
- SPI controller: TRVP_SPI (OpenTitan `spi_host`)
- RTL simulation includes a flash model (MT25QL512ABA1E0)

Flash device, layout, SPI clock and QSPI come from the board's
`rvx_imp_class.xml`.

## Board settings

| Board | Flash | QSPI | SPI clock | File system (base, size) |
|---|---|---|---|---|
| `genesys2-eb` | S25FL256S 32 MB, shared with bitstream | yes | 25 MHz | 0xEF0000, 0x1110000 |
| `arty-plus` | S25FL256S 32 MB, separate | yes | 25 MHz | 0x400000, 0x1C00000 |
| `arty-100t` | S25FL128S 16 MB, shared with bitstream | no | 1 MHz | 0x7B0000, 0x850000 |
| `edge`, `edge-19p`, `edge-19pm` | S25FL512S 64 MB | yes | 25 MHz | 0x400000, 0x3C00000 |

To use QSPI on a board, add `<qspi/>` to `<flash>` in `rvx_imp_class.xml`
and map `spi_flash_sdq[2]`, `spi_flash_sdq[3]` with `PULLUP TRUE` in
`pinmap/spi_flash.xdc`. Set the SPI clock with `<spi_freq_hz>`
(default 1 MHz). QSPI currently supports Spansion/Infineon flash only.

## Reading the flash

In the FPGA directory. Each command clears and recreates its own `dump/` subdirectory.

| Command | Output |
|---|---|
| `make dump.flash_fs` | `dump/flash_fs/files/` — the files in the file system, and `dump/flash_fs/flash_fs.bin`, the image (only blocks in use are transferred, the rest is 0xFF) |
| `make dump.flash_fs_raw` | `dump/flash_fs_raw/flash_fs.bin` — the raw file system region; chunks that are all 0x00 or all 0xFF are not transferred |
| `make dump.unified_flash_image` | `dump/unified_flash_image/*.mcs` — the whole flash, for boards that share it with the bitstream |
| `make dump.fakefile` | `dump/fakefile/` — files written by an app (formerly `make dump`) |

- `make dump.flash_fs` needs the Python package `littlefs-python` (installed by `rvx_install`).
  To extract an image on another PC: `python rvx_devkit/util/extract_flash_fs.py -i flash_fs.bin -b #(sector size) -o files`.
- `make dump.flash_fs_raw DUMP_OFFSET=0x0 DUMP_SIZE=0x10000` reads part of the file system.
- `make dump.unified_flash_image` needs `make #(app).flash` first, and
  reconfigures the FPGA, so run `make program` after it.
- To copy the image to another board of the same type:

  ```
  cd dump/unified_flash_image
  vivado -mode batch -source program_flash_image_wo_rvx.#(date).tcl
  ```

## Adding a file to the flash

In the FPGA directory:

```
make flash_fs.write FILE=big.bin [FS_PATH=/data/big.bin]
```

Verified on `edge-19pm` and `edge`: a 4 MB file takes about 100 s (about 40 KB/s).

- `flash_server` writes the file through littlefs, so existing files stay as they are.
- `FS_PATH` defaults to `/#(file name)`. Missing parent directories are created.
- It fails if `FS_PATH` already exists or the file system has no room for the file.
- The file is written as `FS_PATH.tmp` and renamed at the end, so an interrupted write leaves no partial file under `FS_PATH`.

## Apps

| App | Description |
|---|---|
| `verify_flash_fs` | basic file write/read and remount (RTL sim and FPGA, `make verify`) |
| `verify_flash_fs_fpga` | 128 kB file, file API checks, boot counter |
| `test_flash_id` | read flash ID and status/config registers (read-only) |
| `test_flash_speed_fpga` | find the maximum SPI clock (**overwrites the last FS sector**) |

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_flash_fs.sim
```

FPGA:

```
make #(fpga name)
cd imp_#(fpga name)_#(date)
make imp
make program
make printf                      # in another shell
make verify_flash_fs_fpga.run    # expect "[LFS] PASS"
```

Run `verify_flash_fs_fpga` again: the boot count must go up by one.

## Known issues

- The first flash command after FPGA configuration returns wrong data.
  The driver works around it with one dummy read.
