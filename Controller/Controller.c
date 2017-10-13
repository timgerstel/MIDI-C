#include<io.h>
#include<util/delay.h>


int main(void){
    DDRB = 0xFF;  //Set outputs
	DDRA = 0x00;  //Set inputs
	PORTB = 0x00; //Turns all leds off
	PORTA = 0x07; // sets inputs to return 5v on PA0, PA1, PA2
    while(1){
	
	if((PINA & 0x04) && !(PINA & 0x02)) //Record Mode
		{
			PORTB = 0x01; 
		}
	if((PINA & 0x02) && !(PINA & 0x04)) // Play Mode
		{ 	
			if (PINA & 0x01){ // Modify Mode
				PORTB = 0x04;
			}else{
				PORTB = 0x02;
			}	
		}
	
	 
		else // if all off, leds are off
		{
		PORTB = 0x00;
		}
    }
}
