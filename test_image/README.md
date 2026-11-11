# test_image

## Overview

Platform for the image peripherals: the ArduCAM camera and the RGB and
black-and-white OLED displays. Images can be built into the binary and
shown on the OLED, or captured by the camera and dumped to the host.

- Core `rvc_orca`, 128 kB SRAM, slow DRAM, timer, SPI flash
- ArduCAM (SPI camera), OLED RGB (Pmod OLEDrgb), OLED BW
- Default FPGA: `arty-100t`

## Image peripherals

Each peripheral is a define in `<spec>` of the platform XML:

| Peripheral | XML | Software |
|---|---|---|
| ArduCAM | `include_arducam` = True | `arducam.h` |
| OLED RGB | `include_oled_rgb` = True | `oled_rgb.h` |
| OLED BW | `include_oled_bw` = True | `PmodOLED.h`, `ervp_oledbw_printf.h` |
| SPI flash | `include_spi_flash` = True | `trvp_spi_flash.h` (used by `flash_server`) |

For example:

```
<define>
  <name>include_arducam</name>
  <value type="bool">True</value>
</define>
```

Before each use of the camera or the OLED RGB, call
`arducam_config_spi()` or `oled_rgb_config_spi()` to set up the SPI for it.

## Pin maps

Only `arty-100t` has pin maps for the camera and the OLED, in its imp class:
`imp_class/arty-100t/pinmap/arducam.xdc` and `pinmap/oled.xdc` (Pmod).

For any other board, write the pin map yourself in
`user/fpga/#(fpga name)/xdc/` of the platform, and read it from
`user/env/set_fpga_imp_env.tcl`:

```
set ucf_list [concat_file_list $ucf_list ${PLATFORM_DIR}/user/fpga/${FPGA_NAME}/xdc/*.xdc]
```

Ports to map (see the `arty-100t` files for the I/O standard):

| Peripheral | Ports |
|---|---|
| ArduCAM | `arducam_spi_sclk`, `arducam_spi_scs`, `arducam_spi_sdq0`, `arducam_spi_sdq1`, `arducam_i2c_sclk`, `arducam_i2c_sdata` |
| OLED RGB | `oled_sclk`, `oled_scs`, `oled_sdq0`, `oled_sdq1`, `oled_sdcsel`, `oled_srstnn`, `oled_svbat`, `oled_svdd`, `oled_sspi_dq2`, `oled_sspi_dq3` |

## Images in the binary

Put the images in `util/image/target/` (`fruit.jpg`, `tiger.jpg`,
`yunakim.jpg`) and set the size in `util/image/each_image.mh`
(96x64, the OLED RGB size). Then run `make` in `util/image`:

- each image is converted to RGB565 C source, `#(name)_image.c` and `.h`
- the files are copied to `user/api/`

An app includes `#(name)_image.h` and uses `#(name)_image` as an
`ErvpImage*`, for example `oled_rgb_draw_rvx_image(tiger_image)`.

## Image dump

`test_image_dump` and `test_small_image_dump` convert the captured image to
BMP and call `image_prepare_dump()` (BMP or JPEG only). After the app ends,
run `make dump.fakefile` in the FPGA directory: the image is copied to
`dump/fakefile/dumped_image.jpg` (the name is `.jpg` even for a BMP).

## Apps

| App | Description |
|---|---|
| `test_oled_rgb` | draws rectangles, lines, pixels and bitmaps and scrolls on the OLED RGB; never exits |
| `display_oled` | shows the three images in `user/api/` on the OLED RGB, one per second; never exits |
| `test_arducam` | captures QVGA RGB565 frames and shows them on the OLED RGB until space is pressed |
| `test_image_dump` | captures one QVGA frame, shows it on the OLED RGB, converts it to BMP and prepares the dump |
| `test_small_image_dump` | the same with a 96x64 buffer in SRAM |
| `flash_server` | serves the host's rvx flash commands (`make #(app).flash`) |
| `hello` | prints `Hello World!` |

Apps with the camera or the display are judged by a person on the FPGA.

## Run

FPGA:

```
make syn
make arty-100t
cd imp_arty-100t_#(date)
make imp
make program
make printf                      # in another shell
make test_oled_rgb.run
```
