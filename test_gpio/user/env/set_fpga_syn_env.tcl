set xci_list [concat_file_list $xci_list ${PLATFORM_DIR}/user/fpga/common/xci/*/*.{xci,xcix}]
set xci_list [concat_file_list $xci_list ${PLATFORM_DIR}/user/fpga/${FPGA_NAME}/xci/*/*.{xci,xcix}]
