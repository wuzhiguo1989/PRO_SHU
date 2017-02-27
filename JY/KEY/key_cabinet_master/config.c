# include "config.h"
# include "utils.h"
void driver_high_pin(u8 pin)
{
	if(pin == 29){
		high_pin(29);
	}else if(pin == 30){
		high_pin(30);
	}
}
void driver_low_pin(u8 pin)
{
	if(pin == 29){
		low_pin(29);
	}else if(pin == 30){
		low_pin(30);
	}
}

void driver_io_init()
{
    // // P00~P04
    // push_pull(0, 0x01);
    // push_pull(0, 0x02);
    // push_pull(0, 0x04);
    // push_pull(0, 0x08);
    // push_pull(0, 0x10);

    // // P20~P27
    // push_pull(2, 0x01);
    // push_pull(2, 0x02);
    // push_pull(2, 0x04);
    // push_pull(2, 0x08);
    // push_pull(2, 0x10);
    // push_pull(2, 0x20);
    // push_pull(2, 0x40);
    // push_pull(2, 0x80);

    // // P35
    // push_pull(3, 0x20);
    // // P41~P46
    // push_pull(4, 0x02);
    // push_pull(4, 0x04);
    // push_pull(4, 0x08);
    // push_pull(4, 0x10);
    // push_pull(4, 0x20);
    // push_pull(4, 0x40);

    // // P51~P52
    // push_pull(5, 0x02);
    // push_pull(5, 0x04);

    // // P70~P77
    // push_pull(7, 0x01);
    // push_pull(7, 0x02);
    // push_pull(7, 0x04);
    // push_pull(7, 0x08);
    // push_pull(7, 0x10);
    // push_pull(7, 0x20);
    // push_pull(7, 0x40);
    // push_pull(7, 0x80);
    // P40
    push_pull(4, 0x01);
    // P64
    push_pull(6, 0x10);
}