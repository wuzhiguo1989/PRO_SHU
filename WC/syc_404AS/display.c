#include "shared.h"
#include "display.h"
#include "sync.h"
#include "def.h"
#include "intrins.h"
// ------------------------- display config declaring -------------------------


// 1
static code u16 const 
cycle_for_0001[12]= {476, 500, 200, 70, 70, 70, 70, 70, 70, 70, 490, 10};
// 2
static code u16 const 
// cycle_for_0010[3] = {1, 2, 2};
cycle_for_0010[2] = {2, 2};
// 3
static code u16 const 
cycle_for_0011[15] = {41, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 55, 10};
// 4
static code u16 const 
cycle_for_0100[3] = {476, 490, 10};
// 5
static code u16 const 
cycle_for_0101[13] = {11, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 90, 10};
// 6
static code u16 const 
cycle_for_0110[3] = {176, 190, 10};
// 7
static code u16 const 
cycle_for_0111[5] = {76, 100, 100, 290, 10};
// 8
static code u16 const 
cycle_for_1000[24] = {41, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 260, 65, 65, 65, 65, 65, 65, 65, 190, 10};
// 9
static code u16 const 
cycle_for_1001[26] = {476, 500, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 200, 65, 65, 65, 65, 65, 65, 65, 190, 10};
// 10
static code u16 const 
cycle_for_1010[13] = {16, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 510, 10};
// 11
static code u16 const 
cycle_for_1011[3] ={296, 310, 10};
// 12
static code u16 const
cycle_for_1100[5] ={36, 60, 60, 150, 10};

cycle_reached_pt cycle_reached;
static const u16 code * data cycle_params;
static u8 current_interval;
static u8 cycle_counter_for_0010 = 4;

// ------------------------- display_mode_init declaring -------------------------
static void combination_driver_init();
static void driver_0001_init();
static void driver_0010_init();
static void driver_0011_init();
static void driver_0100_init();
static void driver_0101_init();
static void driver_0110_init();
static void driver_0111_init();
static void driver_1000_init();
static void driver_1001_init();
static void driver_1010_init();
static void driver_1011_init();
static void driver_1100_init();
static void driver_1101_init();

// ------------------------- cycle_reached declaring ---------------------------
static void cycle_reached_0001(u16);
static void cycle_reached_0010(u16);
static void cycle_reached_0011(u16);
static void cycle_reached_0100(u16);
static void cycle_reached_0101(u16);
static void cycle_reached_0110(u16);
static void cycle_reached_0111(u16);
static void cycle_reached_1000(u16);
static void cycle_reached_1001(u16);
static void cycle_reached_1010(u16);
static void cycle_reached_1011(u16);
static void cycle_reached_1100(u16);

void
toggle_once() {
	// todo
	if(cycle_output[0] == ON) P36 = 1;
	else P36 = 0;
	if(cycle_output[1] == ON) P37 = 1;
	else  P37 = 0;
}
void 
cycle_based_adjust(const u16 cycle) {
		cycle_reached(cycle);
}

void 
display_mode_set_changed() {
	// DON't trig display_mode_logic_changed()
	// -> sync.h
//	cycle_ctrl_disable();
	//display_mode_logic = display_mode_set;
	// 1. driver init
	// 2. display_driver -> detailed
	// 3. cycle_params -> detailed
	switch (display_mode_set) {
		case combination:
			combination_driver_init();
			// display_mode_logic will never be combination
			// don't set display driver -> combination driver
			// don't set cycle_params -> any
			break;
		case flash1_directional: 
			driver_0001_init();
			cycle_params = cycle_for_0001;
			cycle_reached = cycle_reached_0001;
			break;
		case flash2_directional: 
			driver_0010_init();
			cycle_params = cycle_for_0010;
			cycle_reached = cycle_reached_0010;
			break;
		case random_fade: 
			driver_0011_init();
			cycle_params = cycle_for_0011;
			cycle_reached = cycle_reached_0011;
			break;
		case stacking_flash: 
			driver_0100_init();
			cycle_params = cycle_for_0100;
			cycle_reached = cycle_reached_0100;
			break;
		case flash_and_chase: 
			driver_0101_init();
			cycle_params = cycle_for_0101;
			cycle_reached = cycle_reached_0101;
			break;
		case random_flashing: 
			driver_0110_init();
			cycle_params = cycle_for_0110;
			cycle_reached = cycle_reached_0110;
			break;
		case bi_directional_pulsing: 
			driver_0111_init();
			cycle_params = cycle_for_0111;
			cycle_reached = cycle_reached_0111;
			break;
		case bi_directional_chasing: 
			driver_1000_init();
			cycle_params = cycle_for_1000;
			cycle_reached = cycle_reached_1000;
			break;
		case pulsing: 
			driver_1001_init();
			cycle_params = cycle_for_1001;
			cycle_reached = cycle_reached_1001;
			break;
		case steady_flash: 
			driver_1010_init();
			cycle_params = cycle_for_1010;
			cycle_reached = cycle_reached_1010;
			break;
		case sequential_flashing: 
			driver_1011_init();
			cycle_params = cycle_for_1011;
			cycle_reached = cycle_reached_1011;
			break;
		case progressive_strobing: 
			driver_1100_init();
			cycle_params = cycle_for_1100;
			cycle_reached = cycle_reached_1100;
			break;
		case bi_directional_storbing:
			driver_1101_init();
		default:
			break; 
	}
	reset_all_counters();
	// -> sync.h
 //	cycle_ctrl_enable();
}

// called by pca_isr (combination rotation)
// called by com_isr (combination init)
void 
display_mode_logic_changed() {
	// 1. driver init
	// 2. display_driver -> detailed
	// 3. cycle_params -> detailed
	// -> sync.h
	//cycle_ctrl_disable();
	switch (display_mode_logic) {
		case combination:
			break;
		case flash1_directional: 
			driver_0001_init();
			cycle_params = cycle_for_0001;
			cycle_reached = cycle_reached_0001;
			break;
		case flash2_directional: 
			driver_0010_init();
			cycle_params = cycle_for_0010;
			cycle_reached = cycle_reached_0010;
			break;
		case random_fade: 
			driver_0011_init();
			cycle_params = cycle_for_0011;
			cycle_reached = cycle_reached_0011;
			break;
		case stacking_flash: 
			driver_0100_init();
			cycle_params = cycle_for_0100;
			cycle_reached = cycle_reached_0100;
			break;
		case flash_and_chase: 
			driver_0101_init();
			cycle_params = cycle_for_0101;
			cycle_reached = cycle_reached_0101;
			break;
		case random_flashing: 
			driver_0110_init();
			cycle_params = cycle_for_0110;
			cycle_reached = cycle_reached_0110;
			break;
		case bi_directional_pulsing: 
			driver_0111_init();
			cycle_params = cycle_for_0111;
			cycle_reached = cycle_reached_0111;
			break;
		case bi_directional_chasing: 
			driver_1000_init();
			cycle_params = cycle_for_1000;
			cycle_reached = cycle_reached_1000;
			break;
		case pulsing: 
			driver_1001_init();
			cycle_params = cycle_for_1001;
			cycle_reached = cycle_reached_1001;
			break;
		case steady_flash: 
			driver_1010_init();
			cycle_params = cycle_for_1010;
			cycle_reached = cycle_reached_1010;
			break;
		case sequential_flashing: 
			driver_1011_init();
			cycle_params = cycle_for_1011;
			cycle_reached = cycle_reached_1011;
			break;
		case progressive_strobing: 
			driver_1100_init();
			cycle_params = cycle_for_1100;
			cycle_reached = cycle_reached_1100;
			break;
		case bi_directional_storbing:
		//	driver_1101_init();
		default:
			break; 
	}
	reset_counter_for_cycle();
	// override flag -> YES:	(slow)fade-in-out
	// override flag -> NO: 	else
}

// 0
static void
combination_driver_init() {
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
	cycle_finished = NO;
	P33 = 0;
	current_interval = 0;
	display_mode_logic_changed();
}
// 13
static void
driver_1101_init()
{
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
	cycle_finished = NO;
	P33 = 0;
	current_interval = 0;
	//display_mode_logic = 2;
	display_mode_logic_changed();
}

// 1
static void
driver_0001_init() {
	current_interval = 0;
	P33 = 0;
//	cycle_output[0] = ON;
//	cycle_output[1] = OFF;
	P36 = 1;
	P37 = 0;
}


static void 
cycle_reached_0001(u16 cycle) 
{
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 1){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 2){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 3){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 4){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 5){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 6){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 7){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 8){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 9){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 10){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 11){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		 	P33 = 0;
		}
		// else if(current_interval == 10){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 0;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		// if(current_interval == 11)
		// {
		// 	current_interval = 0;
		// 	cycle_finished = YES;
		// }
		if(current_interval == 12)
		{
			current_interval = 0;
		}
	}
}

// 2
static void
driver_0010_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	cycle_counter_for_0010 = 4;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}

static void 
cycle_reached_0010(u16 cycle) 
{
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		// else if(current_interval == 1){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P33 = 1;
		// }
		// else if(current_interval == 2){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 1;
		//  P33 = 0;
		// }
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}

		reset_counter_for_cycle();
		++ current_interval;
//		if(current_interval == 3)
		 if(current_interval == 2)
		{
			current_interval = 0;
			cycle_finished = NO;
			--cycle_counter_for_0010;
			if(!cycle_counter_for_0010)
			{
				cycle_counter_for_0010 = 4;
				if(display_mode_set == combination)
				{
					display_mode_logic = 3;
					display_mode_logic_changed();
				}

			}
		}
	}
}
// 3
static void
driver_0011_init() {
	current_interval = 0;
	cycle_finished = NO;
	// cycle_output[0] = ON;
	// cycle_output[1] = OFF;
	P36 = 1;
	P37 = 0;
}


static void 
cycle_reached_0011(u16 cycle) {
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 4){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 5){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 6){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 7){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 8){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 9){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 10){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 11){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 12){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 13){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 14){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		 	P33 = 0;
		}
		// else if(current_interval == 13){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 0;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 15)
		// if(current_interval == 14)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}

// 4
static void
driver_0100_init() {
	// cycle_output[0] = ON;
	// cycle_output[1] = OFF;
	P36 = 1;
	P37 = 0;
	cycle_finished = NO;
	P33 = 0;
	current_interval = 0;
}


static void 
cycle_reached_0100(u16 cycle) {

	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		 	P33 = 0;
		}
		// else if(current_interval == 1){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 0;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 3)
		// if(current_interval == 2)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}


// 5
static void
driver_0101_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}


static void 
cycle_reached_0101(u16 cycle) {

	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 4){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 5){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 6){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 7){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 8){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 9){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 10){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 11){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 12){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 1;
		 P33 = 0;
		}
		// else if(current_interval == 11){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 13)
		// if(current_interval == 12)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}

// 6
static void
driver_0110_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}


static void 
cycle_reached_0110(u16 cycle) {

	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 1;
		 P33 = 0;
		}
		// else if(current_interval == 1){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 3)
		// if(current_interval == 2)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}


// 7
static void 
driver_0111_init()
{
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}

static void
cycle_reached_0111(u16 cycle)
{
	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 4){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 1;
		 	P33 = 0;
		}
		// else if(current_interval == 3){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 5)
		// if(current_interval == 4)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}

	}
}


// 8
static void
driver_1000_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = OFF;
	P36 = 1;
	P37 = 0;
}


static void 
cycle_reached_1000(u16 cycle) {

	if(cycle >= cycle_params[current_interval])
	{
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 4){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 5){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;		
		}
		else if(current_interval == 6){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 7){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 8){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 9){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 10){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 11){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 12){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 13){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 14){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 15){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 16){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 17){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 18){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 19){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 20){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 21){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 22){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 23){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		 P33 = 0;
		}
		// else if(current_interval == 22){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 0;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 24)
		// if(current_interval == 23)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}


// 9
static void
driver_1001_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = OFF;
	P36 = 1;
	P37 = 0;
}

static void 
cycle_reached_1001(u16 cycle) {
	if(cycle >= cycle_params[current_interval]){
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 4){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 5){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 6){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 7){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		}
		else if(current_interval == 8){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 9){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 10){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 11){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 12){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 13){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 14){
			// cycle_output[0] = OFF;
			// cycle_output[1] = ON;
			P36 = 0;
			P37 = 1;
		}
		else if(current_interval == 15){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 16){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 17){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 18){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 19){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 20){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 21){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 22){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 23){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 24){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 25){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 0;
		 P33 = 0;
		}
		// else if(current_interval == 24){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = OFF;
		// 	P36 = 1;
		// 	P37 = 0;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 26)
		// if(current_interval == 25)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}


// 10
static void
driver_1010_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
}

static void 
cycle_reached_1010(u16 cycle) {
	if(cycle >= cycle_params[current_interval]){
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 4){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 5){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 6){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 7){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 8){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 9){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 10){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 11){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 12){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		 P33 = 0;
		}
		// else if(current_interval == 11){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 13)
		// if(current_interval == 12)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}

// 11
static void
driver_1011_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}

static void 
cycle_reached_1011(u16 cycle)
{
	if(cycle >= cycle_params[current_interval]){	
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 1;
		 P33 = 0;
		}
		// else if(current_interval == 1){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 3)
		// if(current_interval == 2)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	
	}
}

// 12
static void
driver_1100_init() {
	current_interval = 0;
	cycle_finished = NO;
	P33 = 0;
	// cycle_output[0] = ON;
	// cycle_output[1] = ON;
	P36 = 1;
	P37 = 1;
}

static void 
cycle_reached_1100(u16 cycle) {
	if(cycle >= cycle_params[current_interval]){
		if(current_interval == 0){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 1){
			// cycle_output[0] = ON;
			// cycle_output[1] = ON;
			P36 = 1;
			P37 = 1;
		}
		else if(current_interval == 2){
			// cycle_output[0] = OFF;
			// cycle_output[1] = OFF;
			P36 = 0;
			P37 = 0;
		}
		else if(current_interval == 3){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P33 = 1;
		}
		else if(current_interval == 4){
			// cycle_output[0] = ON;
			// cycle_output[1] = OFF;
			P36 = 1;
			P37 = 1;
		 P33 = 0;
		}
		// else if(current_interval == 3){
		// 	// cycle_output[0] = ON;
		// 	// cycle_output[1] = ON;
		// 	P36 = 1;
		// 	P37 = 1;
		// }
		reset_counter_for_cycle();
		++ current_interval;
		if(current_interval == 5)
		// if(current_interval == 4)
		{
			current_interval = 0;
			// cycle_finished = YES;
		}
	}
}