/*****************/
/* Custom Region */
/*****************/

// wire [(32)-1:0] plic_user_interrupts;
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
// wire [(7)-1:0] rstnn_seqeunce;
// wire [(7)-1:0] rstpp_seqeunce;
// wire rstnn_user;
// wire rstpp_user;
// wire i_itr_gen_clk;
// wire i_itr_gen_rstnn;
// wire i_itr_gen_rpsel;
// wire i_itr_gen_rpenable;
// wire i_itr_gen_rpwrite;
// wire [(32)-1:0] i_itr_gen_rpaddr;
// wire [(32)-1:0] i_itr_gen_rpwdata;
// wire i_itr_gen_rpready;
// wire [(32)-1:0] i_itr_gen_rprdata;
// wire i_itr_gen_rpslverr;

/* DO NOT MODIFY THE ABOVE */
/* MUST MODIFY THE BELOW   */

// software-controlled interrupt generator (user/rtl/src/itr_gen_apb.v)
// drives PLIC user interrupts 0-3; replace it with your own IP

wire [4-1:0] itr_gen_interrupt;

ITR_GEN_APB
#(
	.BW_ADDR(32),
	.BW_DATA(32),
	.NUM_ITR(4)
)
i_itr_gen
(
	.clk(i_itr_gen_clk),
	.rstnn(i_itr_gen_rstnn),
	.rpsel(i_itr_gen_rpsel),
	.rpenable(i_itr_gen_rpenable),
	.rpwrite(i_itr_gen_rpwrite),
	.rpaddr(i_itr_gen_rpaddr),
	.rpwdata(i_itr_gen_rpwdata),
	.rpready(i_itr_gen_rpready),
	.rprdata(i_itr_gen_rprdata),
	.rpslverr(i_itr_gen_rpslverr),
	.interrupt(itr_gen_interrupt)
);

// only bits 0-3 are used by the PLIC (NUM_PLIC_SOURCE_USER)
assign plic_user_interrupts = {{(32-4){1'b 0}}, itr_gen_interrupt};
