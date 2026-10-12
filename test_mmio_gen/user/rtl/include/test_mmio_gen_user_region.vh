/*****************/
/* Custom Region */
/*****************/

// wire clk_system;
// wire clk_core;
// wire clk_system_external;
// wire clk_system_debug;
// wire clk_local_access;
// wire clk_process_000;
// wire clk_noc;
// wire gclk_system;
// wire gclk_core;
// wire gclk_system_external;
// wire gclk_system_debug;
// wire gclk_local_access;
// wire gclk_process_000;
// wire gclk_noc;
// wire tick_1us;
// wire tick_62d5ms;
// wire tick_gpio;
// wire spi_common_sclk;
// wire spi_common_sdq0;
// wire global_rstnn;
// wire global_rstpp;
// wire [(6)-1:0] rstnn_seqeunce;
// wire [(6)-1:0] rstpp_seqeunce;
// wire rstnn_user;
// wire rstpp_user;
// wire i_test_mmio_gen_clk;
// wire i_test_mmio_gen_rstnn;
// wire [(64)-1:0] i_test_mmio_gen_rmx_core_config;
// wire [(32)-1:0] i_test_mmio_gen_rmx_core_status;
// wire i_test_mmio_gen_rmx_clear_request;
// wire i_test_mmio_gen_rmx_clear_finish;
// wire i_test_mmio_gen_rmx_log_fifo_wready;
// wire i_test_mmio_gen_rmx_log_fifo_wrequest;
// wire [(32)-1:0] i_test_mmio_gen_rmx_log_fifo_wdata;
// wire i_test_mmio_gen_rmx_inst_fifo_rready;
// wire [(62)-1:0] i_test_mmio_gen_rmx_inst_fifo_rdata;
// wire i_test_mmio_gen_rmx_inst_fifo_rrequest;
// wire i_test_mmio_gen_rmx_operation_finish;
// wire i_test_mmio_gen_rmx_input_fifo_rready;
// wire [(64)-1:0] i_test_mmio_gen_rmx_input_fifo_rdata;
// wire i_test_mmio_gen_rmx_input_fifo_rrequest;
// wire i_test_mmio_gen_rmx_output_fifo_wready;
// wire i_test_mmio_gen_rmx_output_fifo_wrequest;
// wire [(64)-1:0] i_test_mmio_gen_rmx_output_fifo_wdata;

/* DO NOT MODIFY THE ABOVE */
/* MUST MODIFY THE BELOW   */

reg inst_rrequest;
wire [32-1:0] waddr;
wire [08-1:0] wnum;
wire [08-1:0] opcode;
reg allows_transfer;

/*
USER_IP
#(
	.BW_INST(62),
	.BW_CONFIG(64),
	.BW_STATUS(32),
	.BW_INPUT(64),
	.BW_OUTPUT(64)
)
i_test_mmio_gen
(
	.clk(i_test_mmio_gen_clk),
	.rstnn(i_test_mmio_gen_rstnn),
	.rmx_core_config(i_test_mmio_gen_rmx_core_config),
	.rmx_core_status(i_test_mmio_gen_rmx_core_status),
	.rmx_clear_request(i_test_mmio_gen_rmx_clear_request),
	.rmx_clear_finish(i_test_mmio_gen_rmx_clear_finish),
	.rmx_log_fifo_wready(i_test_mmio_gen_rmx_log_fifo_wready),
	.rmx_log_fifo_wrequest(i_test_mmio_gen_rmx_log_fifo_wrequest),
	.rmx_log_fifo_wdata(i_test_mmio_gen_rmx_log_fifo_wdata),
	.rmx_inst_fifo_rready(i_test_mmio_gen_rmx_inst_fifo_rready),
	.rmx_inst_fifo_rdata(i_test_mmio_gen_rmx_inst_fifo_rdata),
	.rmx_inst_fifo_rrequest(i_test_mmio_gen_rmx_inst_fifo_rrequest),
	.rmx_operation_finish(i_test_mmio_gen_rmx_operation_finish),
	.rmx_input_fifo_rready(i_test_mmio_gen_rmx_input_fifo_rready),
	.rmx_input_fifo_rdata(i_test_mmio_gen_rmx_input_fifo_rdata),
	.rmx_input_fifo_rrequest(i_test_mmio_gen_rmx_input_fifo_rrequest),
	.rmx_output_fifo_wready(i_test_mmio_gen_rmx_output_fifo_wready),
	.rmx_output_fifo_wrequest(i_test_mmio_gen_rmx_output_fifo_wrequest),
	.rmx_output_fifo_wdata(i_test_mmio_gen_rmx_output_fifo_wdata)
);
*/
assign i_test_mmio_gen_clk = gclk_core;
//assign `NOT_CONNECT = i_test_mmio_gen_rstnn;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_core_config;
assign i_test_mmio_gen_rmx_core_status = 32'h 010a;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_log_fifo_wready;
assign i_test_mmio_gen_rmx_clear_finish = i_test_mmio_gen_rmx_clear_request;
assign i_test_mmio_gen_rmx_log_fifo_wrequest = inst_rrequest;
assign i_test_mmio_gen_rmx_log_fifo_wdata = 32'h dddd;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_inst_fifo_rready;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_inst_fifo_rdata;
assign i_test_mmio_gen_rmx_inst_fifo_rrequest = inst_rrequest;
assign i_test_mmio_gen_rmx_operation_finish = inst_rrequest;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_input_fifo_rready;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_input_fifo_rdata;
assign i_test_mmio_gen_rmx_input_fifo_rrequest = allows_transfer & i_test_mmio_gen_rmx_input_fifo_rready;
//assign `NOT_CONNECT = i_test_mmio_gen_rmx_output_fifo_wready;
assign i_test_mmio_gen_rmx_output_fifo_wrequest = allows_transfer & i_test_mmio_gen_rmx_input_fifo_rready;
assign i_test_mmio_gen_rmx_output_fifo_wdata = i_test_mmio_gen_rmx_input_fifo_rdata;

assign {opcode, wnum, waddr} = i_test_mmio_gen_rmx_inst_fifo_rdata;

// testbench
initial
begin
  inst_rrequest = 0;
  allows_transfer = 0;

  //
  wait(i_test_mmio_gen_rmx_inst_fifo_rready==1);
  wait(i_test_mmio_gen_clk==1);
  #1
  $display("0x%x %d %d", waddr, wnum, opcode);
  inst_rrequest = 1;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);
  #1
  inst_rrequest = 0;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);

  //
  wait(i_test_mmio_gen_rmx_inst_fifo_rready==1);
  wait(i_test_mmio_gen_clk==1);
  #1
  $display("0x%x %d %d", waddr, wnum, opcode);
  inst_rrequest = 1;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);
  #1
  inst_rrequest = 0;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);

  //
  wait(i_test_mmio_gen_rmx_inst_fifo_rready==1);
  wait(i_test_mmio_gen_clk==1);
  #1
  $display("0x%x %d %d", waddr, wnum, opcode);
  inst_rrequest = 1;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);
  #1
  inst_rrequest = 0;
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);

  //
  #1
  allows_transfer = 1;
end

/*
initial
begin
  wait(inst_rrequest==1);
  wait(i_test_mmio_gen_clk==0);
  wait(i_test_mmio_gen_clk==1);
  wait(i_platform.i_rtl.i_test_mmio_gen_no_name_apb2mmiox_interrupt_list!=0);
  $display("Interrupt Occurs");
  wait(i_platform.i_rtl.i_test_mmio_gen_no_name_apb2mmiox_interrupt_list==0);
  $display("Interrupt Clears");
end
*/

/////////////////////////////////////////////
// i_demo: MMIO generated by util/mmio (demo_ip)
/////////////////////////////////////////////

wire [8-1:0] demo_config;
wire demo_wait_done_re;
wire demo_wait_done_readyin;
wire demo_start_write_cmd;
wire demo_tick_read_cmd;
wire [4-1:0] demo_kick_onehot_cmd;
wire [2-1:0] demo_data_re_list;
wire [2*32-1:0] demo_data_rdata_list;
wire [2-1:0] demo_data_we_list;
wire [2*32-1:0] demo_data_wdata_list;
wire [48-1:0] demo_wide_out;
wire demo_cmdq_rready;
wire demo_cmdq_rrequest;
wire [32-1:0] demo_cmdq_rdata;
wire demo_result_wready;
wire demo_result_wrequest;
wire [32-1:0] demo_result_wdata;
wire demo_mode_we;
wire [2-1:0] demo_mode_wdata;

reg [4-1:0] demo_stall_count;
reg [4-1:0] demo_wait_count;
reg [4-1:0] demo_start_count;
reg [4-1:0] demo_tick_count;
reg [4-1:0] demo_kick_acc;
reg [32-1:0] demo_data_reg [0:1];
reg [2-1:0] demo_mode_reg;

DEMO_IP_MMIO
#(
	.BW_ADDR(32),
	.BW_DATA(32)
)
i_demo_mmio
(
	.clk(i_demo_clk),
	.rstnn(i_demo_rstnn),
	.rpsel(i_demo_rpsel),
	.rpenable(i_demo_rpenable),
	.rpaddr(i_demo_rpaddr),
	.rpwrite(i_demo_rpwrite),
	.rpwdata(i_demo_rpwdata),
	.rprdata(i_demo_rprdata),
	.rpready(i_demo_rpready),
	.rpslverr(i_demo_rpslverr),
	.tick_gpio(tick_gpio),
	.ctrl_config(demo_config),
	.ctrl_wait_done_re(demo_wait_done_re),
	.ctrl_wait_done_rdata(1'b 1),
	.ctrl_wait_done_readyin(demo_wait_done_readyin),
	.ctrl_start_write_cmd(demo_start_write_cmd),
	.ctrl_tick_read_cmd(demo_tick_read_cmd),
	.ctrl_kick_onehot_cmd(demo_kick_onehot_cmd),
	.ctrl_wide_in(64'h 1122334455667788),
	.ctrl_wide_out(demo_wide_out),
	.ctrl_cmdq_clear(1'b 0),
	.ctrl_cmdq_rready(demo_cmdq_rready),
	.ctrl_cmdq_rempty(),
	.ctrl_cmdq_rnum(),
	.ctrl_cmdq_rrequest(demo_cmdq_rrequest),
	.ctrl_cmdq_rdata(demo_cmdq_rdata),
	.ctrl_cmdq_wready(),
	.ctrl_cmdq_wfull(),
	.ctrl_cmdq_wnum(),
	.ctrl_result_clear(1'b 0),
	.ctrl_result_rready(),
	.ctrl_result_rempty(),
	.ctrl_result_rnum(),
	.ctrl_result_wready(demo_result_wready),
	.ctrl_result_wfull(),
	.ctrl_result_wnum(),
	.ctrl_result_wrequest(demo_result_wrequest),
	.ctrl_result_wdata(demo_result_wdata),
	.ctrl_ver_re(),
	.ctrl_ver_rdata(demo_wide_out[31:0]),
	.ctrl_cap_re(),
	.ctrl_cap_rdata({demo_wide_out[47:32],demo_stall_count,demo_start_count,demo_tick_count,demo_kick_acc}),
	.ctrl_mode_re(),
	.ctrl_mode_rdata(demo_mode_reg),
	.ctrl_mode_we(demo_mode_we),
	.ctrl_mode_wdata(demo_mode_wdata),
	.ctrl_data_re_list(demo_data_re_list),
	.ctrl_data_rdata_list({~demo_data_reg[1],~demo_data_reg[0]}),
	.ctrl_data_we_list(demo_data_we_list),
	.ctrl_data_wdata_list(demo_data_wdata_list)
);

// wait_done: stalls every read for 10 cycles
assign demo_wait_done_readyin = (demo_wait_count==10);

always@(posedge i_demo_clk, negedge i_demo_rstnn)
begin
	if(i_demo_rstnn==0)
	begin
		demo_wait_count <= 0;
		demo_stall_count <= 0;
	end
	else if(demo_wait_done_re)
	begin
		if(demo_wait_done_readyin)
			demo_wait_count <= 0;
		else
		begin
			demo_wait_count <= demo_wait_count + 1;
			demo_stall_count <= demo_stall_count + 1;
		end
	end
end

// commands
always@(posedge i_demo_clk, negedge i_demo_rstnn)
begin
	if(i_demo_rstnn==0)
	begin
		demo_start_count <= 0;
		demo_tick_count <= 0;
		demo_kick_acc <= 0;
	end
	else
	begin
		if(demo_start_write_cmd)
			demo_start_count <= demo_start_count + 1;
		if(demo_tick_read_cmd)
			demo_tick_count <= demo_tick_count + 1;
		demo_kick_acc <= demo_kick_acc | demo_kick_onehot_cmd;
	end
end

// general: data (array) and mode
always@(posedge i_demo_clk, negedge i_demo_rstnn)
begin
	if(i_demo_rstnn==0)
	begin
		demo_data_reg[0] <= 0;
		demo_data_reg[1] <= 0;
		demo_mode_reg <= 0;
	end
	else
	begin
		if(demo_data_we_list[0])
			demo_data_reg[0] <= demo_data_wdata_list[32*1-1-:32];
		if(demo_data_we_list[1])
			demo_data_reg[1] <= demo_data_wdata_list[32*2-1-:32];
		if(demo_mode_we)
			demo_mode_reg <= demo_mode_wdata;
	end
end

// cmdq (wfifo) -> +1 -> result (rfifo)
assign demo_cmdq_rrequest = demo_cmdq_rready & demo_result_wready;
assign demo_result_wrequest = demo_cmdq_rrequest;
assign demo_result_wdata = demo_cmdq_rdata + 1;
