`include "ervp_global.vh"

module TEST1_APB
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
	rpslverr
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 1;
parameter BW_DATA = 1;

input wire clk, rstnn;
input wire rpsel;
input wire rpenable;
input wire [BW_ADDR-1:0] rpaddr;
input wire rpwrite;
input wire [BW_DATA-1:0] rpwdata;
output reg [BW_DATA-1:0] rprdata;
output wire rpready;
output reg rpslverr;

/////////////
/* signals */
/////////////

genvar i;

wire [8-1:0] addr_offset = rpaddr;
reg is_valid_addr;

reg [32-1:0] var_x, var_y, var_z;
reg we_x, we_y, we_z;

reg request_add;
reg request_sub;
reg request_mac;

////////////
/* logics */
////////////

always@(*)
begin
	rprdata = 0;
	is_valid_addr = 0;
	we_x = 0;
	we_y = 0;
	we_z = 0;
	request_add = 0;
	request_sub = 0;
	request_mac = 0;

	if(rpsel==1'b 1)
	begin
		if(rpenable==1'b 1)
		begin
			case(addr_offset)
				8'h 0:
				begin
					is_valid_addr = 1;
					rprdata = var_x;
					we_x = rpwrite;
				end
				8'h 4:
				begin
					is_valid_addr = 1;
					rprdata = var_y;
					we_y = rpwrite;
				end
				8'h 8:
				begin
					is_valid_addr = 1;
					rprdata = var_z;
					we_z = rpwrite;
				end
				8'h C:
				begin
					is_valid_addr = 1;
					if(rpwrite==1'b 1)
						request_add = 1;
					else
						request_sub = 1;
				end
				8'h 10:
				begin
					is_valid_addr = 1;
					if(rpwrite==1'b 1)
						request_mac = 1;
				end
				default:
					is_valid_addr = 0;
			endcase
		end
	end
end

assign rpready = 1;

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		var_x <= 0;
	else if(we_x==1'b 1)
		var_x <= rpwdata;
end

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		var_y <= 0;
	else if(we_y==1'b 1)
		var_y <= rpwdata;
end

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		var_z <= 0;
	else if(we_z==1'b 1)
		var_z <= rpwdata;
	else if(request_add)
		var_z <= var_x + var_y;
	else if(request_sub)
		var_z <= var_x - var_y;
	else if(request_mac)
		var_z <= (var_x * var_y) + rpwdata;
end

always@(posedge clk, negedge rstnn)
begin
	if(rstnn==0)
		rpslverr <= 0;
	else if(rpsel==1'b 1)
	begin
		if(rpenable==1'b 0)
			rpslverr <= 0;
		else
			rpslverr <= ~is_valid_addr;
	end
end

endmodule
