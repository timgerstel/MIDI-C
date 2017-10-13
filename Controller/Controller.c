#include<io.h>
#include<util/delay.h>
int main(void){
    DDRB = 0xFF;
    while(1){
	for (uint8_t bval = 0; bval <= 255; bval++){
	    _delay_ms(20);
	    PORTB = bval;
	}
    }
}
