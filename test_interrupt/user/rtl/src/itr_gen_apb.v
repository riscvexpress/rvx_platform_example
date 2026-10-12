`include "ervp_global.vh"

// Software-controlled interrupt generator for the "test_interrupt" platform.
// A starting point for adding an interrupt output to a user IP.
//
// offset 0x0  SET    write: 1 raises the matching line
//                    read : current lines
// offset 0x4  CLEAR  write: 1 lowers the matching line
//
// The PLIC gateway is level sensitive: the ISR must lower its line
// (write CLEAR) before it returns, or the interrupt fires again.

module ITR_GEN_APB
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

	interrupt
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 1;
parameter BW_DATA = 1;
parameter NUM_ITR = 4;

input wire clk, rstnn;
input wire rpsel;
input wire rpenable;
input wire [BW_ADDR-1:0] rpaddr;
input wire rpwrite;
input wire [BW_DATA-1:0] rpwdata;
output reg [BW_DATA-1:0] rprdata;
output wire rpready;
output reg rpslverr;

output reg [NUM_ITR-1:0] interrupt;

/////////////
/* signals */
/////////////

wire [8-1:0] addr_offset = rpaddr;
wire access = rpsel & rpenable;
reg is_valid_addr;
reg we_set, we_clear;

////////////
/* logics */
////////////

always@(*)
begin
	rprdata = 0;
	is_valid_addr = 0;
	we_set = 0;
	we_clear = 0;
	if(access)
		case(addr_offset)
			8'h 0:
			begin
				is_valid_addr = 1;
				rprdata = interrupt;
				we_set = rpwrite;
			end
			8'h 4:
			begin
				is_valid_addr = 1;
				we_clear = rpwrite;
			end
		endcase
end

assign rpready = 1;

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		interrupt <= 0;
	else if(we_set)
		interrupt <= interrupt | rpwdata[NUM_ITR-1:0];
	else if(we_clear)
		interrupt <= interrupt & (~rpwdata[NUM_ITR-1:0]);
end

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		rpslverr <= 0;
	else if(rpsel)
	begin
		if(rpenable==1'b 0)
			rpslverr <= 0;
		else
			rpslverr <= ~is_valid_addr;
	end
end

endmodule
