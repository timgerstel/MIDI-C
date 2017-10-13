#include<io.h>
#include<util/delay.h>


int main(void){
    DDRB = 0xFF;  //Set outputs
	DDRA = 0x00;  //Set inputs
	PORTB = 0x00; //Turns all leds off
	PORTA = 0x07; // sets inputs to return 5v on PA0, PA1, PA2
    while(1){
		uint8_t rec = PINA & 0x04;
		uint8_t play = PINA & 0x02;
		uint8_t mod = PINA & 0x01;
	
		if(rec && !play){
			record();
		} else if (play && !rec){
			if (mod){ // Modify Mode
				modify();
			}else{
				playBack();
			}	
		}
	
	 
		else // if all off, leds are off
		{
		PORTB = 0x00;
		}
    }
}

void record(){
	PORTB = 0x01; 
}

void playBack(){
	PORTB = 0x02;
}

void modify(){
	PORTB = 0x04;
}
