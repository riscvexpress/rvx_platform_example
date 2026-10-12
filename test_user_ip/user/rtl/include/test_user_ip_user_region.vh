/*****************/
/* Custom Region */
/*****************/

// wire spi_common_sclk;
// wire spi_common_sdq0;
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
// wire global_rstnn;
// wire global_rstpp;
// wire [(6)-1:0] rstnn_seqeunce;
// wire [(6)-1:0] rstpp_seqeunce;
// wire rstnn_user;
// wire rstpp_user;
// wire i_test1_clk;
// wire i_test1_rstnn;
// wire i_test1_rpsel;
// wire i_test1_rpenable;
// wire i_test1_rpwrite;
// wire [(32)-1:0] i_test1_rpaddr;
// wire [(32)-1:0] i_test1_rpwdata;
// wire i_test1_rpready;
// wire [(32)-1:0] i_test1_rprdata;
// wire i_test1_rpslverr;
// wire i_test2_clk;
// wire i_test2_rstnn;
// wire i_test2_sxawready;
// wire i_test2_sxawvalid;
// wire [(32)-1:0] i_test2_sxawaddr;
// wire [(2)-1:0] i_test2_sxawid;
// wire [(8)-1:0] i_test2_sxawlen;
// wire [(3)-1:0] i_test2_sxawsize;
// wire [(2)-1:0] i_test2_sxawburst;
// wire i_test2_sxwready;
// wire i_test2_sxwvalid;
// wire [(2)-1:0] i_test2_sxwid;
// wire [(32)-1:0] i_test2_sxwdata;
// wire [(32/8)-1:0] i_test2_sxwstrb;
// wire i_test2_sxwlast;
// wire i_test2_sxbready;
// wire i_test2_sxbvalid;
// wire [(2)-1:0] i_test2_sxbid;
// wire [(2)-1:0] i_test2_sxbresp;
// wire i_test2_sxarready;
// wire i_test2_sxarvalid;
// wire [(32)-1:0] i_test2_sxaraddr;
// wire [(2)-1:0] i_test2_sxarid;
// wire [(8)-1:0] i_test2_sxarlen;
// wire [(3)-1:0] i_test2_sxarsize;
// wire [(2)-1:0] i_test2_sxarburst;
// wire i_test2_sxrready;
// wire i_test2_sxrvalid;
// wire [(2)-1:0] i_test2_sxrid;
// wire [(32)-1:0] i_test2_sxrdata;
// wire i_test2_sxrlast;
// wire [(2)-1:0] i_test2_sxrresp;

/* DO NOT MODIFY THE ABOVE */
/* MUST MODIFY THE BELOW   */


TEST1_APB
#(
	.BW_ADDR((32)),
	.BW_DATA((32))
)
i_test1
(
	.clk(i_test1_clk),
	.rstnn(i_test1_rstnn),
	.rpsel(i_test1_rpsel),
	.rpenable(i_test1_rpenable),
	.rpwrite(i_test1_rpwrite),
	.rpaddr(i_test1_rpaddr),
	.rpwdata(i_test1_rpwdata),
	.rpready(i_test1_rpready),
	.rprdata(i_test1_rprdata),
	.rpslverr(i_test1_rpslverr)
);

/*
USER_IP
#(
	.BW_ADDR(32),
	.BW_DATA(32),
	.BW_AXI_TID(2)
)
i_test2
(
	.clk(i_test2_clk),
	.rstnn(i_test2_rstnn),
	.sxawready(i_test2_sxawready),
	.sxawvalid(i_test2_sxawvalid),
	.sxawaddr(i_test2_sxawaddr),
	.sxawid(i_test2_sxawid),
	.sxawlen(i_test2_sxawlen),
	.sxawsize(i_test2_sxawsize),
	.sxawburst(i_test2_sxawburst),
	.sxwready(i_test2_sxwready),
	.sxwvalid(i_test2_sxwvalid),
	.sxwid(i_test2_sxwid),
	.sxwdata(i_test2_sxwdata),
	.sxwstrb(i_test2_sxwstrb),
	.sxwlast(i_test2_sxwlast),
	.sxbready(i_test2_sxbready),
	.sxbvalid(i_test2_sxbvalid),
	.sxbid(i_test2_sxbid),
	.sxbresp(i_test2_sxbresp),
	.sxarready(i_test2_sxarready),
	.sxarvalid(i_test2_sxarvalid),
	.sxaraddr(i_test2_sxaraddr),
	.sxarid(i_test2_sxarid),
	.sxarlen(i_test2_sxarlen),
	.sxarsize(i_test2_sxarsize),
	.sxarburst(i_test2_sxarburst),
	.sxrready(i_test2_sxrready),
	.sxrvalid(i_test2_sxrvalid),
	.sxrid(i_test2_sxrid),
	.sxrdata(i_test2_sxrdata),
	.sxrlast(i_test2_sxrlast),
	.sxrresp(i_test2_sxrresp)
);
*/
assign i_test2_clk = 0;
//assign `NOT_CONNECT = i_test2_rstnn;
//assign `NOT_CONNECT = i_test2_sxawready;
assign i_test2_sxawvalid = 0;
assign i_test2_sxawaddr = 0;
assign i_test2_sxawid = 0;
assign i_test2_sxawlen = 0;
assign i_test2_sxawsize = 0;
assign i_test2_sxawburst = 0;
//assign `NOT_CONNECT = i_test2_sxwready;
assign i_test2_sxwvalid = 0;
assign i_test2_sxwid = 0;
assign i_test2_sxwdata = 0;
assign i_test2_sxwstrb = 0;
assign i_test2_sxwlast = 0;
assign i_test2_sxbready = 0;
//assign `NOT_CONNECT = i_test2_sxbvalid;
//assign `NOT_CONNECT = i_test2_sxbid;
//assign `NOT_CONNECT = i_test2_sxbresp;
//assign `NOT_CONNECT = i_test2_sxarready;
assign i_test2_sxarvalid = 0;
assign i_test2_sxaraddr = 0;
assign i_test2_sxarid = 0;
assign i_test2_sxarlen = 0;
assign i_test2_sxarsize = 0;
assign i_test2_sxarburst = 0;
assign i_test2_sxrready = 0;
//assign `NOT_CONNECT = i_test2_sxrvalid;
//assign `NOT_CONNECT = i_test2_sxrid;
//assign `NOT_CONNECT = i_test2_sxrdata;
//assign `NOT_CONNECT = i_test2_sxrlast;
//assign `NOT_CONNECT = i_test2_sxrresp;
