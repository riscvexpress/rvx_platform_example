
reg [32-1:0] switch_slide0;
reg [32-1:0] switch_slide1;

assign i_platform.switch_slide_readymade_0 = switch_slide0;
assign i_platform.switch_slide1 = switch_slide1;

// Switch stimulus shared by all apps (time unit: ns)
// The first step is held for 8 ms so that main() is already running
// (boot takes about 4 ms) before the first switch changes.
//
//  time(ms)  slide1 slide0
//   0 -  8     0      0
//   8 - 12     0      1    slide0 rise
//  12 - 16     1      0    slide0 fall, slide1 0->1
//  16 - 20     2      0    slide1 1->2
//  20 - 24     3      0    slide1 2->3
//  24 - 28     0      1    slide0 rise and slide1 3->0 at the same time
//  28 -        0      0    slide0 fall
// app/verify_gpio_debounce replaces this sequence (see below).
initial
begin : legacy_stimulus
  // (switch_slide1 << 1) | switch_slide0
  // 000
	switch_slide0 = 0;
	switch_slide1 = 0;
	#8000000
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

////////////////////////////////////////////////////////////
// debounce stimulus for app/verify_gpio_debounce
// starts when software enables debouncing on switch_slide1,
// then replaces the sequence above
// (GPIO_TICK_PERIOD_US is 20 us in simulation, 4 consecutive ticks = 80 us)
////////////////////////////////////////////////////////////

localparam DEBOUNCE_NUM_PRESS = 3;
localparam DEBOUNCE_NUM_BOUNCE = 16;
localparam US = 1000;

integer debounce_seed = 7;
integer debounce_press;
integer debounce_k;

// contact bounce settling to final_value: pulses of the final value are longer than the others
task debounce_bounce(input final_value);
begin
	for (debounce_k = 0; debounce_k < DEBOUNCE_NUM_BOUNCE; debounce_k = debounce_k + 1)
	begin
		switch_slide0 = final_value;
		#((5 + ({$random(debounce_seed)} % 36)) * US);
		switch_slide0 = ~final_value;
		#((2 + ({$random(debounce_seed)} % 14)) * US);
	end
	switch_slide0 = final_value;
end
endtask

initial
begin
	wait (i_platform.i_rtl.i_gpio_switch_slide1.gpio_config[`GPIO_CONFIG_INDEX_DEBOUNCE_ENABLE] == 1);
	disable legacy_stimulus;
	$display("[TB] debounce stimulus: %0d bouncy presses", DEBOUNCE_NUM_PRESS);
	switch_slide0 = 0;
	switch_slide1 = 0;
	#(400 * US);
	for (debounce_press = 0; debounce_press < DEBOUNCE_NUM_PRESS; debounce_press = debounce_press + 1)
	begin
		debounce_bounce(1);
		#(400 * US);
		debounce_bounce(0);
		#(400 * US);
	end
	switch_slide1 = 3;
	$display("[TB] debounce stimulus done");
end
