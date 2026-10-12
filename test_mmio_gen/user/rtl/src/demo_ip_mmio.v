// ****************************************************************************
// ****************************************************************************
// Copyright SoC Design Research Group, All rights reserved.    
// Electronics and Telecommunications Research Institute (ETRI)
//
// THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
// WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
// TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
// REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
// SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
// IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
// COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
// ****************************************************************************
// Kyuseung Han (han@etri.re.kr)
// ****************************************************************************
// ****************************************************************************

`include "ervp_global.vh"
`include "ervp_endian.vh"
`include "demo_ip_memorymap_offset.vh"

module DEMO_IP_MMIO
(
	clk,
	rstnn,

	rpsel,
	rpenable,
	rpaddr,
	rpwrite,
	rpwdata,
	rprdata,
	rpready,
	rpslverr,

	tick_gpio,
	ctrl_config,
	ctrl_wait_done_re,
	ctrl_wait_done_rdata,
	ctrl_wait_done_readyin,
	ctrl_start_write_cmd,
	ctrl_tick_read_cmd,
	ctrl_kick_onehot_cmd,
	ctrl_wide_in,
	ctrl_wide_out,
	ctrl_cmdq_clear,
	ctrl_cmdq_rready,
	ctrl_cmdq_rempty,
	ctrl_cmdq_rnum,
	ctrl_cmdq_rrequest,
	ctrl_cmdq_rdata,
	ctrl_cmdq_wready,
	ctrl_cmdq_wfull,
	ctrl_cmdq_wnum,
	ctrl_result_clear,
	ctrl_result_rready,
	ctrl_result_rempty,
	ctrl_result_rnum,
	ctrl_result_wready,
	ctrl_result_wfull,
	ctrl_result_wnum,
	ctrl_result_wrequest,
	ctrl_result_wdata,
	ctrl_ver_re,
	ctrl_ver_rdata,
	ctrl_cap_re,
	ctrl_cap_rdata,
	ctrl_mode_re,
	ctrl_mode_rdata,
	ctrl_mode_we,
	ctrl_mode_wdata,
	ctrl_data_re_list,
	ctrl_data_rdata_list,
	ctrl_data_we_list,
	ctrl_data_wdata_list
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 1;
parameter BW_DATA = 1;
parameter ENDIAN_TYPE = `LITTLE_ENDIAN;

`include "ervp_endian.vf"
`include "ervp_log_util.vf"

input wire clk, rstnn;
input wire rpsel;
input wire rpenable;
input wire [BW_ADDR-1:0] rpaddr;
input wire rpwrite;
input wire [BW_DATA-1:0] rpwdata;
output wire [BW_DATA-1:0] rprdata;
output reg rpready;
output reg rpslverr;


input wire tick_gpio;

output wire [`BW_CTRL_CONFIG-1:0] ctrl_config;

output wire ctrl_wait_done_re;
input wire [`BW_CTRL_WAIT_DONE-1:0] ctrl_wait_done_rdata;
input wire ctrl_wait_done_readyin;

output wire ctrl_start_write_cmd;

output wire ctrl_tick_read_cmd;

output wire [`BW_CTRL_KICK-1:0] ctrl_kick_onehot_cmd;

input wire [`BW_CTRL_WIDE_IN-1:0] ctrl_wide_in;

output wire [`BW_CTRL_WIDE_OUT-1:0] ctrl_wide_out;

input wire ctrl_cmdq_clear;
output wire ctrl_cmdq_wready;
output wire ctrl_cmdq_wfull;
output wire [BW_DATA-1:0] ctrl_cmdq_wnum;
output wire ctrl_cmdq_rready;
output wire ctrl_cmdq_rempty;
output wire [BW_DATA-1:0] ctrl_cmdq_rnum;
input wire ctrl_cmdq_rrequest;
output wire [`BW_CTRL_CMDQ-1:0] ctrl_cmdq_rdata;

input wire ctrl_result_clear;
output wire ctrl_result_rready;
output wire ctrl_result_rempty;
output wire [BW_DATA-1:0] ctrl_result_rnum;
output wire ctrl_result_wready;
output wire ctrl_result_wfull;
output wire [BW_DATA-1:0] ctrl_result_wnum;
input wire ctrl_result_wrequest;
input wire [`BW_CTRL_RESULT-1:0] ctrl_result_wdata;

output wire ctrl_ver_re;
input wire [`BW_CTRL_VER-1:0] ctrl_ver_rdata;

output wire ctrl_cap_re;
input wire [`BW_CTRL_CAP-1:0] ctrl_cap_rdata;

output wire ctrl_mode_re;
input wire [`BW_CTRL_MODE-1:0] ctrl_mode_rdata;
output wire ctrl_mode_we;
output wire [`BW_CTRL_MODE-1:0] ctrl_mode_wdata;


output wire [`NUM_CTRL_DATA-1:0] ctrl_data_re_list;
input wire [(`BW_CTRL_DATA)*(`NUM_CTRL_DATA)-1:0] ctrl_data_rdata_list;
output wire [`NUM_CTRL_DATA-1:0] ctrl_data_we_list;
output wire [(`BW_CTRL_DATA)*(`NUM_CTRL_DATA)-1:0] ctrl_data_wdata_list;

/////////////
/* signals */
/////////////

genvar i;

wire [BW_DATA-1:0] man_rpwdata;
reg [BW_DATA-1:0] man_rprdata;
wire read_request_from_bus;
wire write_request_from_bus;
wire is_aligned_access;

wire [`BW_MMAP_OFFSET_DEMO_IP-1:0] paddr_offset = rpaddr;
wire [`BW_MMAP_OFFSET_DEMO_IP-1:0] addr_offset;
wire [BW_ADDR-1:0] addr_aligned;
wire [`BW_UNUSED_DEMO_IP-1:0] addr_unaligned;
wire [`BW_UNUSED_DEMO_IP-1:0] addr_unused = 0;
reg signal_ctrl_config_re;
wire [BW_DATA-1:0] signal_ctrl_config_rdata;
reg signal_ctrl_config_we;
wire [BW_DATA-1:0] signal_ctrl_config_wdata;
wire signal_ctrl_config_readyin;
reg [8-1:0] reg_ctrl_config;
reg signal_ctrl_wait_done_re;
wire [BW_DATA-1:0] signal_ctrl_wait_done_rdata;
reg signal_ctrl_wait_done_we;
wire [BW_DATA-1:0] signal_ctrl_wait_done_wdata;
wire signal_ctrl_wait_done_readyin;
reg signal_ctrl_start_re;
wire [BW_DATA-1:0] signal_ctrl_start_rdata;
reg signal_ctrl_start_we;
wire [BW_DATA-1:0] signal_ctrl_start_wdata;
wire signal_ctrl_start_readyin;
reg signal_ctrl_tick_re;
wire [BW_DATA-1:0] signal_ctrl_tick_rdata;
reg signal_ctrl_tick_we;
wire [BW_DATA-1:0] signal_ctrl_tick_wdata;
wire signal_ctrl_tick_readyin;
reg signal_ctrl_kick_re;
wire [BW_DATA-1:0] signal_ctrl_kick_rdata;
reg signal_ctrl_kick_we;
wire [BW_DATA-1:0] signal_ctrl_kick_wdata;
wire signal_ctrl_kick_readyin;
reg signal_ctrl_data00_re;
wire [BW_DATA-1:0] signal_ctrl_data00_rdata;
reg signal_ctrl_data00_we;
wire [BW_DATA-1:0] signal_ctrl_data00_wdata;
wire signal_ctrl_data00_readyin;
reg signal_ctrl_data01_re;
wire [BW_DATA-1:0] signal_ctrl_data01_rdata;
reg signal_ctrl_data01_we;
wire [BW_DATA-1:0] signal_ctrl_data01_wdata;
wire signal_ctrl_data01_readyin;
reg signal_ctrl_wide_in_re;
wire [BW_DATA-1:0] signal_ctrl_wide_in_rdata;
reg signal_ctrl_wide_in_we;
wire [BW_DATA-1:0] signal_ctrl_wide_in_wdata;
wire signal_ctrl_wide_in_readyin;
wire [64-1:0] reg_ctrl_wide_in;
reg signal_ctrl_wide_out_re;
wire [BW_DATA-1:0] signal_ctrl_wide_out_rdata;
reg signal_ctrl_wide_out_we;
wire [BW_DATA-1:0] signal_ctrl_wide_out_wdata;
wire signal_ctrl_wide_out_readyin;
wire [48-1:0] reg_ctrl_wide_out;
reg signal_ctrl_cmdq_re;
wire [BW_DATA-1:0] signal_ctrl_cmdq_rdata;
reg signal_ctrl_cmdq_we;
wire [BW_DATA-1:0] signal_ctrl_cmdq_wdata;
wire signal_ctrl_cmdq_readyin;
wire fifo_ctrl_cmdq_clear;
wire fifo_ctrl_cmdq_wready;
wire fifo_ctrl_cmdq_wfull;
wire fifo_ctrl_cmdq_wrequest;
wire [32-1:0] fifo_ctrl_cmdq_wdata;
wire [BW_DATA-1:0] fifo_ctrl_cmdq_wnum;
wire fifo_ctrl_cmdq_rready;
wire fifo_ctrl_cmdq_rempty;
wire fifo_ctrl_cmdq_rrequest;
wire [32-1:0] fifo_ctrl_cmdq_rdata;
wire [BW_DATA-1:0] fifo_ctrl_cmdq_rnum;
reg signal_ctrl_result_re;
wire [BW_DATA-1:0] signal_ctrl_result_rdata;
reg signal_ctrl_result_we;
wire [BW_DATA-1:0] signal_ctrl_result_wdata;
wire signal_ctrl_result_readyin;
wire fifo_ctrl_result_clear;
wire fifo_ctrl_result_wready;
wire fifo_ctrl_result_wfull;
wire fifo_ctrl_result_wrequest;
wire [32-1:0] fifo_ctrl_result_wdata;
wire [BW_DATA-1:0] fifo_ctrl_result_wnum;
wire fifo_ctrl_result_rready;
wire fifo_ctrl_result_rempty;
wire fifo_ctrl_result_rrequest;
wire [32-1:0] fifo_ctrl_result_rdata;
wire [BW_DATA-1:0] fifo_ctrl_result_rnum;
reg signal_ctrl_hole_re;
wire [BW_DATA-1:0] signal_ctrl_hole_rdata;
reg signal_ctrl_hole_we;
wire [BW_DATA-1:0] signal_ctrl_hole_wdata;
wire signal_ctrl_hole_readyin;
reg signal_ctrl_ver_re;
wire [BW_DATA-1:0] signal_ctrl_ver_rdata;
reg signal_ctrl_ver_we;
wire [BW_DATA-1:0] signal_ctrl_ver_wdata;
wire signal_ctrl_ver_readyin;
reg signal_ctrl_cap_re;
wire [BW_DATA-1:0] signal_ctrl_cap_rdata;
reg signal_ctrl_cap_we;
wire [BW_DATA-1:0] signal_ctrl_cap_wdata;
wire signal_ctrl_cap_readyin;
reg signal_ctrl_mode_re;
wire [BW_DATA-1:0] signal_ctrl_mode_rdata;
reg signal_ctrl_mode_we;
wire [BW_DATA-1:0] signal_ctrl_mode_wdata;
wire signal_ctrl_mode_readyin;
reg signal_ctrl_result_valid_num_re;
wire [BW_DATA-1:0] signal_ctrl_result_valid_num_rdata;
reg signal_ctrl_result_valid_num_we;
wire [BW_DATA-1:0] signal_ctrl_result_valid_num_wdata;
wire signal_ctrl_result_valid_num_readyin;

////////////
/* logics */
////////////

assign man_rpwdata = CHANGE_ENDIAN_BUS2MAN(BW_DATA,ENDIAN_TYPE,rpwdata);
assign rprdata = CHANGE_ENDIAN_MAN2BUS(BW_DATA,ENDIAN_TYPE,man_rprdata);
assign {addr_aligned,addr_unaligned} = paddr_offset;
assign addr_offset = {addr_aligned,addr_unused};
assign is_aligned_access = (addr_unaligned==0);
assign read_request_from_bus = rpsel & rpenable & is_aligned_access & (~rpwrite);
assign write_request_from_bus = rpsel & rpenable & is_aligned_access & rpwrite;

assign signal_ctrl_config_wdata = $unsigned(rpwdata);
assign signal_ctrl_wait_done_wdata = $unsigned(rpwdata);
assign signal_ctrl_start_wdata = $unsigned(rpwdata);
assign signal_ctrl_tick_wdata = $unsigned(rpwdata);
assign signal_ctrl_kick_wdata = $unsigned(rpwdata);
assign signal_ctrl_data00_wdata = $unsigned(rpwdata);
assign signal_ctrl_data01_wdata = $unsigned(rpwdata);
assign signal_ctrl_wide_in_wdata = $unsigned(rpwdata);
assign signal_ctrl_wide_out_wdata = $unsigned(rpwdata);
assign signal_ctrl_cmdq_wdata = $unsigned(rpwdata);
assign signal_ctrl_result_wdata = $unsigned(rpwdata);
assign signal_ctrl_hole_wdata = $unsigned(rpwdata);
assign signal_ctrl_ver_wdata = $unsigned(rpwdata);
assign signal_ctrl_cap_wdata = $unsigned(rpwdata);
assign signal_ctrl_mode_wdata = $unsigned(rpwdata);
assign signal_ctrl_result_valid_num_wdata = $unsigned(rpwdata);

always@(*)
begin
	rpslverr = 0;
	man_rprdata = 0;
	rpready = 1;

	signal_ctrl_config_re = 0;
	signal_ctrl_config_we = 0;

	signal_ctrl_wait_done_re = 0;
	signal_ctrl_wait_done_we = 0;

	signal_ctrl_start_re = 0;
	signal_ctrl_start_we = 0;

	signal_ctrl_tick_re = 0;
	signal_ctrl_tick_we = 0;

	signal_ctrl_kick_re = 0;
	signal_ctrl_kick_we = 0;

	signal_ctrl_data00_re = 0;
	signal_ctrl_data00_we = 0;

	signal_ctrl_data01_re = 0;
	signal_ctrl_data01_we = 0;

	signal_ctrl_wide_in_re = 0;
	signal_ctrl_wide_in_we = 0;

	signal_ctrl_wide_out_re = 0;
	signal_ctrl_wide_out_we = 0;

	signal_ctrl_cmdq_re = 0;
	signal_ctrl_cmdq_we = 0;

	signal_ctrl_result_re = 0;
	signal_ctrl_result_we = 0;

	signal_ctrl_hole_re = 0;
	signal_ctrl_hole_we = 0;

	signal_ctrl_ver_re = 0;
	signal_ctrl_ver_we = 0;

	signal_ctrl_cap_re = 0;
	signal_ctrl_cap_we = 0;

	signal_ctrl_mode_re = 0;
	signal_ctrl_mode_we = 0;

	signal_ctrl_result_valid_num_re = 0;
	signal_ctrl_result_valid_num_we = 0;

	if(rpsel==1'b 1)
	begin
		case(addr_offset)
			`MMAP_OFFSET_CTRL_CONFIG:
			begin
				signal_ctrl_config_re = read_request_from_bus;
				signal_ctrl_config_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_config_rdata);
				rpready = signal_ctrl_config_readyin;
			end
			`MMAP_OFFSET_CTRL_WAIT_DONE:
			begin
				signal_ctrl_wait_done_re = read_request_from_bus;
				signal_ctrl_wait_done_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_wait_done_rdata);
				rpready = signal_ctrl_wait_done_readyin;
			end
			`MMAP_OFFSET_CTRL_START:
			begin
				signal_ctrl_start_re = read_request_from_bus;
				signal_ctrl_start_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_start_rdata);
				rpready = signal_ctrl_start_readyin;
			end
			`MMAP_OFFSET_CTRL_TICK:
			begin
				signal_ctrl_tick_re = read_request_from_bus;
				signal_ctrl_tick_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_tick_rdata);
				rpready = signal_ctrl_tick_readyin;
			end
			`MMAP_OFFSET_CTRL_KICK:
			begin
				signal_ctrl_kick_re = read_request_from_bus;
				signal_ctrl_kick_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_kick_rdata);
				rpready = signal_ctrl_kick_readyin;
			end
			`MMAP_OFFSET_CTRL_DATA00:
			begin
				signal_ctrl_data00_re = read_request_from_bus;
				signal_ctrl_data00_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_data00_rdata);
				rpready = signal_ctrl_data00_readyin;
			end
			`MMAP_OFFSET_CTRL_DATA01:
			begin
				signal_ctrl_data01_re = read_request_from_bus;
				signal_ctrl_data01_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_data01_rdata);
				rpready = signal_ctrl_data01_readyin;
			end
			`MMAP_OFFSET_CTRL_WIDE_IN:
			begin
				signal_ctrl_wide_in_re = read_request_from_bus;
				signal_ctrl_wide_in_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_wide_in_rdata);
				rpready = signal_ctrl_wide_in_readyin;
			end
			`MMAP_OFFSET_CTRL_WIDE_OUT:
			begin
				signal_ctrl_wide_out_re = read_request_from_bus;
				signal_ctrl_wide_out_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_wide_out_rdata);
				rpready = signal_ctrl_wide_out_readyin;
			end
			`MMAP_OFFSET_CTRL_CMDQ:
			begin
				signal_ctrl_cmdq_re = read_request_from_bus;
				signal_ctrl_cmdq_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_cmdq_rdata);
				rpready = signal_ctrl_cmdq_readyin;
			end
			`MMAP_OFFSET_CTRL_RESULT:
			begin
				signal_ctrl_result_re = read_request_from_bus;
				signal_ctrl_result_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_result_rdata);
				rpready = signal_ctrl_result_readyin;
			end
			`MMAP_OFFSET_CTRL_VER:
			begin
				signal_ctrl_ver_re = read_request_from_bus;
				signal_ctrl_ver_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_ver_rdata);
				rpready = signal_ctrl_ver_readyin;
			end
			`MMAP_OFFSET_CTRL_CAP:
			begin
				signal_ctrl_cap_re = read_request_from_bus;
				signal_ctrl_cap_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_cap_rdata);
				rpready = signal_ctrl_cap_readyin;
			end
			`MMAP_OFFSET_CTRL_MODE:
			begin
				signal_ctrl_mode_re = read_request_from_bus;
				signal_ctrl_mode_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_mode_rdata);
				rpready = signal_ctrl_mode_readyin;
			end
			`MMAP_OFFSET_CTRL_RESULT_VALID_NUM:
			begin
				signal_ctrl_result_valid_num_re = read_request_from_bus;
				signal_ctrl_result_valid_num_we = write_request_from_bus;
				man_rprdata = $unsigned(signal_ctrl_result_valid_num_rdata);
				rpready = signal_ctrl_result_valid_num_readyin;
			end
			default:
				rpslverr = 1;
		endcase
	end
end

// ctrl_config
always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		reg_ctrl_config <= `CTRL_CONFIG_DEFAULT_VALUE;
	else if (signal_ctrl_config_we==1'b 1)
		reg_ctrl_config <= signal_ctrl_config_wdata;
end
assign signal_ctrl_config_rdata = reg_ctrl_config;
// ctrl_wide_in
ERVP_MMIO_WIDE_READ
#(
	.BW_MMIO(BW_DATA),
	.BW_WIDE_DATA(64)
)
i_ctrl_wide_in
(
	.clk(clk),
	.rstnn(rstnn),
	.clear(1'b 0),
	.enable(1'b 1),
	.mmio_re(signal_ctrl_wide_in_re),
	.mmio_rdata(signal_ctrl_wide_in_rdata),
	.wide_data_in(reg_ctrl_wide_in)
);
// ctrl_wide_out
ERVP_MMIO_WIDE_REG
#(
	.BW_MMIO(BW_DATA),
	.BW_WIDE_DATA(48),
	.DEFAULT_VALUE(`CTRL_WIDE_OUT_DEFAULT_VALUE)
)
i_ctrl_wide_out
(
	.clk(clk),
	.rstnn(rstnn),
	.clear(1'b 0),
	.enable(1'b 1),
	.mmio_re(signal_ctrl_wide_out_re),
	.mmio_rdata(signal_ctrl_wide_out_rdata),
	.mmio_we(signal_ctrl_wide_out_we),
	.mmio_wdata(signal_ctrl_wide_out_wdata),
	.wide_data_out(reg_ctrl_wide_out)
);
// ctrl_cmdq
ERVP_FIFO
#(
	.BW_DATA(32),
	.DEPTH(4),
	.BW_NUM_DATA(BW_DATA)
)
i_fifo_cmdq
(
	.clk(clk),
	.rstnn(rstnn),
	.enable(1'b 1),
	.clear(fifo_ctrl_cmdq_clear),
	.wready(fifo_ctrl_cmdq_wready),
	.wfull(fifo_ctrl_cmdq_wfull),
	.wrequest(fifo_ctrl_cmdq_wrequest),
	.wdata(fifo_ctrl_cmdq_wdata),
	.wnum(fifo_ctrl_cmdq_wnum),
	.rready(fifo_ctrl_cmdq_rready),
	.rempty(fifo_ctrl_cmdq_rempty),
	.rrequest(fifo_ctrl_cmdq_rrequest),
	.rdata(fifo_ctrl_cmdq_rdata),
	.rnum(fifo_ctrl_cmdq_rnum)
);
assign fifo_ctrl_cmdq_clear = ctrl_cmdq_clear;
assign ctrl_cmdq_wready = fifo_ctrl_cmdq_wready;
assign ctrl_cmdq_wfull = fifo_ctrl_cmdq_wfull;
assign ctrl_cmdq_wnum = fifo_ctrl_cmdq_wnum;
assign ctrl_cmdq_rready = fifo_ctrl_cmdq_rready;
assign ctrl_cmdq_rempty = fifo_ctrl_cmdq_rempty;
assign ctrl_cmdq_rnum = fifo_ctrl_cmdq_rnum;
assign fifo_ctrl_cmdq_wrequest = signal_ctrl_cmdq_we;
assign fifo_ctrl_cmdq_wdata = signal_ctrl_cmdq_wdata;
assign signal_ctrl_cmdq_rdata = fifo_ctrl_cmdq_wnum;
assign fifo_ctrl_cmdq_rrequest = ctrl_cmdq_rrequest;
assign ctrl_cmdq_rdata = fifo_ctrl_cmdq_rdata;
// ctrl_result
ERVP_FIFO
#(
	.BW_DATA(32),
	.DEPTH(4),
	.BW_NUM_DATA(BW_DATA)
)
i_fifo_result
(
	.clk(clk),
	.rstnn(rstnn),
	.enable(1'b 1),
	.clear(fifo_ctrl_result_clear),
	.wready(fifo_ctrl_result_wready),
	.wfull(fifo_ctrl_result_wfull),
	.wrequest(fifo_ctrl_result_wrequest),
	.wdata(fifo_ctrl_result_wdata),
	.wnum(fifo_ctrl_result_wnum),
	.rready(fifo_ctrl_result_rready),
	.rempty(fifo_ctrl_result_rempty),
	.rrequest(fifo_ctrl_result_rrequest),
	.rdata(fifo_ctrl_result_rdata),
	.rnum(fifo_ctrl_result_rnum)
);
assign fifo_ctrl_result_clear = ctrl_result_clear;
assign ctrl_result_wready = fifo_ctrl_result_wready;
assign ctrl_result_wfull = fifo_ctrl_result_wfull;
assign ctrl_result_wnum = fifo_ctrl_result_wnum;
assign ctrl_result_rready = fifo_ctrl_result_rready;
assign ctrl_result_rempty = fifo_ctrl_result_rempty;
assign ctrl_result_rnum = fifo_ctrl_result_rnum;
assign fifo_ctrl_result_wrequest = ctrl_result_wrequest;
assign fifo_ctrl_result_wdata = ctrl_result_wdata;
assign fifo_ctrl_result_rrequest = signal_ctrl_result_re;
assign signal_ctrl_result_rdata = fifo_ctrl_result_rdata;
assign signal_ctrl_result_valid_num_rdata = fifo_ctrl_result_rnum;
// ctrl_config
assign ctrl_config = reg_ctrl_config;
assign signal_ctrl_config_readyin = 1;
// ctrl_wait_done
assign ctrl_wait_done_re = signal_ctrl_wait_done_re;
assign signal_ctrl_wait_done_rdata = ctrl_wait_done_rdata;
assign signal_ctrl_wait_done_readyin = ctrl_wait_done_readyin;
// ctrl_start
assign ctrl_start_write_cmd = signal_ctrl_start_we;
assign signal_ctrl_start_rdata = 0;
assign signal_ctrl_start_readyin = 1;
// ctrl_tick
assign ctrl_tick_read_cmd = signal_ctrl_tick_re;
assign signal_ctrl_tick_rdata = 0;
assign signal_ctrl_tick_readyin = 1;
// ctrl_kick
assign ctrl_kick_onehot_cmd = signal_ctrl_kick_we? signal_ctrl_kick_wdata : 0;
assign signal_ctrl_kick_rdata = 0;
assign signal_ctrl_kick_readyin = 1;
// ctrl_wide_in
assign reg_ctrl_wide_in = ctrl_wide_in;
assign signal_ctrl_wide_in_readyin = 1;
// ctrl_wide_out
assign ctrl_wide_out = reg_ctrl_wide_out;
assign signal_ctrl_wide_out_readyin = 1;
// ctrl_cmdq
assign signal_ctrl_cmdq_readyin = 1;
// ctrl_result
assign signal_ctrl_result_readyin = 1;
// ctrl_hole
assign signal_ctrl_hole_readyin = 1;
// ctrl_ver
assign ctrl_ver_re = signal_ctrl_ver_re;
assign signal_ctrl_ver_rdata = ctrl_ver_rdata;
assign signal_ctrl_ver_readyin = 1;
// ctrl_cap
assign ctrl_cap_re = signal_ctrl_cap_re;
assign signal_ctrl_cap_rdata = ctrl_cap_rdata;
assign signal_ctrl_cap_readyin = 1;
// ctrl_mode
assign ctrl_mode_re = signal_ctrl_mode_re;
assign signal_ctrl_mode_rdata = ctrl_mode_rdata;
assign ctrl_mode_we = signal_ctrl_mode_we;
assign ctrl_mode_wdata = signal_ctrl_mode_wdata;
assign signal_ctrl_mode_readyin = 1;
// ctrl_result_valid_num
assign signal_ctrl_result_valid_num_readyin = 1;

assign ctrl_data_re_list[00] = signal_ctrl_data00_re;
assign signal_ctrl_data00_rdata = ctrl_data_rdata_list[(32)*((00)+1)-1-:32];
assign ctrl_data_we_list[00] = signal_ctrl_data00_we;
assign ctrl_data_wdata_list[(32)*((00)+1)-1-:32] = signal_ctrl_data00_wdata;

assign ctrl_data_re_list[01] = signal_ctrl_data01_re;
assign signal_ctrl_data01_rdata = ctrl_data_rdata_list[(32)*((01)+1)-1-:32];
assign ctrl_data_we_list[01] = signal_ctrl_data01_we;
assign ctrl_data_wdata_list[(32)*((01)+1)-1-:32] = signal_ctrl_data01_wdata;

assign signal_ctrl_data00_readyin = 1;

assign signal_ctrl_data01_readyin = 1;

endmodule
