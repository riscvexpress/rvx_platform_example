
reg [32-1:0] switch_slide0;
reg [32-1:0] switch_slide1;

assign i_platform.switch_slide_readymade_0 = switch_slide0;
assign i_platform.switch_slide1 = switch_slide1;

initial
begin
  // (switch_slide1 << 1) | switch_slide0
  // 000
	switch_slide0 = 0;
	switch_slide1 = 0;
	#4000000
  // 001
	switch_slide0 = 1;
	switch_slide1 = 0;
	#4000000
  // 010
	switch_slide0 = 0;
	switch_slide1 = 1;
	#4000000
  // 100
	switch_slide0 = 0;
	switch_slide1 = 2;
	#4000000
  // 110
	switch_slide0 = 0;
	switch_slide1 = 3;
	#4000000
  // 001
	switch_slide0 = 1;
	switch_slide1 = 0;
	#4000000
  // 000
	switch_slide1 = 0;
	switch_slide0 = 0;
end
