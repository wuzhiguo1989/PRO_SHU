# include "config.h"
# include "utils.h"
void driver_high_pin(u8 pin)
{
	if(pin == 1){
		high_pin(1);
	}else if(pin == 2){
		high_pin(2);
	}else if(pin == 3){
		high_pin(3);
	}else if(pin == 4){
		high_pin(4);
	}else if(pin == 5){
		high_pin(5);
	}else if(pin == 6){
		high_pin(6);
	}else if(pin == 7){
		high_pin(7);
	}else if(pin == 8){
		high_pin(8);
	}else if(pin == 9){
		high_pin(9);
	}else if(pin == 10){
		high_pin(10);
	}else if(pin == 11){
		high_pin(11);
	}else if(pin == 12){
		high_pin(12);
	}else if(pin == 13){
		high_pin(13);
	}else if(pin == 14){
		high_pin(14);
	}else if(pin == 15){
		high_pin(15);
	}else if(pin == 16){
		high_pin(16);
	}else if(pin == 17){
		high_pin(17);
	}else if(pin == 18){
		high_pin(18);
	}else if(pin == 19){
		high_pin(19);
	}else if(pin == 20){
		high_pin(20);
	}else if(pin == 21){
		high_pin(21);
	}else if(pin == 22){
		high_pin(22);
	}else if(pin == 23){
		high_pin(23);
	}else if(pin == 24){
		high_pin(24);
	}else if(pin == 25){
		high_pin(25);
	}else if(pin == 26){
		high_pin(26);
	}else if(pin == 27){
		high_pin(27);
	}else if(pin == 28){
		high_pin(28);
	}
}
void driver_low_pin(u8 pin)
{
	if(pin == 1){
		low_pin(1);
	}else if(pin == 2){
		low_pin(2);
	}else if(pin == 3){
		low_pin(3);
	}else if(pin == 4){
		low_pin(4);
	}else if(pin == 5){
		low_pin(5);
	}else if(pin == 6){
		low_pin(6);
	}else if(pin == 7){
		low_pin(7);
	}else if(pin == 8){
		low_pin(8);
	}else if(pin == 9){
		low_pin(9);
	}else if(pin == 10){
		low_pin(10);
	}else if(pin == 11){
		low_pin(11);
	}else if(pin == 12){
		low_pin(12);
	}else if(pin == 13){
		low_pin(13);
	}else if(pin == 14){
		low_pin(14);
	}else if(pin == 15){
		low_pin(15);
	}else if(pin == 16){
		low_pin(16);
	}else if(pin == 17){
		low_pin(17);
	}else if(pin == 18){
		low_pin(18);
	}else if(pin == 19){
		low_pin(19);
	}else if(pin == 20){
		low_pin(20);
	}else if(pin == 21){
		low_pin(21);
	}else if(pin == 22){
		low_pin(22);
	}else if(pin == 23){
		low_pin(23);
	}else if(pin == 24){
		low_pin(24);
	}else if(pin == 25){
		low_pin(25);
	}else if(pin == 26){
		low_pin(26);
	}else if(pin == 27){
		low_pin(27);
	}else if(pin == 28){
		low_pin(28);
	}
}

void driver_io_init()
{
    // P00~P03
    //push_pull(0, 0x01);
    //push_pull(0, 0x02);
    //push_pull(0, 0x04);
    //push_pull(0, 0x08);
		push_pull(0, 0x0F);
    // P10~P17
    //push_pull(1, 0x01);
    //push_pull(1, 0x02);
    //push_pull(1, 0x04);
    //push_pull(1, 0x08);
    //push_pull(1, 0x10);
    //push_pull(1, 0x20);
    //push_pull(1, 0x40);
    //push_pull(1, 0x80);
		push_pull(1, 0xFF);
    // P20~P27
    //push_pull(2, 0x01);
    //push_pull(2, 0x02);
    //push_pull(2, 0x04);
    //push_pull(2, 0x08);
    //push_pull(2, 0x10);
    //push_pull(2, 0x20);
    //push_pull(2, 0x40);
    //push_pull(2, 0x80);
		push_pull(2, 0xFF);
    // P32~P37
    //push_pull(3, 0x04);
    //push_pull(3, 0x08);
    //push_pull(3, 0x10);
    //push_pull(3, 0x20);
    //push_pull(3, 0x40);
    //push_pull(3, 0x80);
		push_pull(3, 0xFC);
    // P54~P55
    //push_pull(5, 0x10);
    //push_pull(5, 0x20);
		push_pull(5, 0x30);
}