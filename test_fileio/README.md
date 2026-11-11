# test_fileio

## Overview

Example of the fakefile API (`ervp_fakefile.h`): apps open, read and write
files by name. Input files are built into the binary; files written by an app
are dumped to the host.

- Core `rvc_orca`, 64 kB SRAM, slow DRAM
- Input files: `util/fakefile/target/` (`tiger_small.jpg`, `mat_int.csv`, `mat_float.csv`)

## Input files

Put the files in `util/fakefile/target/` and run `make all` in `util/fakefile`.
They are converted into `util/generated/ssw/fakefile` and built into the
apps. An app opens them by file name:

```
FAKEFILE* fp = ffopen("tiger_small.jpg", "r");
```

To add `util/fakefile` to a new platform, run `make fakefile` in `util`.

## Output files

On the FPGA, after `make #(app).run`, run `make dump.fakefile` in the FPGA
directory. The files written by the app are copied to `dump/fakefile/`.

## Apps

| App | Description |
|---|---|
| `verify_fileio` | writes `test.txt`, appends to it and reads it back; copies `mat_int.csv` and compares the copy; prints `[verify_fileio] PASS` (`make verify`) |
| `test_fileio_sim` | writes `test.txt` with `rv`, then appends `x` |
| `test_fileio` | copies `tiger_small.jpg` twice, then converts it to `tiger_small_4.bmp` |
| `test_matrix_file` | reads a matrix from CSV (int8 and float32), changes one element, writes it to `*_modified.csv` |

## Run

RTL simulation:

```
make syn
make sim_rtl
cd sim_rtl
make verify_fileio.sim
```

FPGA:

```
make syn
make #(fpga name)
cd imp_#(fpga name)_#(date)
make imp
make program
make printf                      # in another shell
make test_fileio.run
make dump.fakefile
```
