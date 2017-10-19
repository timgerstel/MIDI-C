#include<io.h>
#include<util/delay.h>
#include<interrupt.h>


/***** Define Variables and constants *****/

int extraTime = 0, whichLED = 0;
uint16_t adc_value;
#define F_CPU 8000000
#define BUAD 31250
#define BUAD_PRESCALE (((F_CPU / (BUAD * 16UL))) - 1)



/***** Setup Methods *****/

void setupMIDI(){
	UCSRB = (1 << TXEN) | (1 << RXEN); //Turns on transmitter and Reciever
	UCSRC = (1 << URSEL )|(1 << UCSZ1) | (1 << UCSZ0); //only use 8 bit words

	UBRRL = (BUAD_PRESCALE >> 8);
	UBRRH = BUAD_PRESCALE;
}
void setupPins(){
	DDRB = 0xFF;  //Set outputs
	DDRA = 0x00;  //Set inputs
	PORTB = 0x00; //Turns all leds off
	PORTA = 0x07; // sets inputs to return 5v on PA0, PA1, PA2
}
void setupAnalog(){
	ADMUX = (1 << REFS0) | (1<< MUX0) | (1<< MUX1) | (1<< MUX2); // sets the analog input of the photosensor to 0-5v, sets to look at the 7th analog pin.
	ADCSRA =  (1 << ADEN) | (1<< ADPS2) | (1<< ADPS1) | (1<< ADPS0); // ADEN turns ADC on; ADPS sets prescaler to 128;
}
void setupTimer(){
	TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS00); // Sets CTC mode for clock and sets prescaler to clk/1024
	OCR0 = 39; // every 39 ticks = .01 miliseconds (comparsion variable)
	TIMSK = (1 << OCIE0); // Set an interput whenver the ticks and my comarsion variable matchs)
	sei(); // needed to set interrupts
}

/***** Create Methods *****/

uint16_t ReadADC(){
	//Start a single conversion
	ADCSRA |= (1 << ADSC);
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	//clear data (conversion is complete)
	ADCSRA |= (1 << ADIF);
	return(ADC);
}

void wait(int time)
{
   _delay_ms(time);
}

void record(){
	PORTB = 0x01; 
}

void playBack(){
	PORTB = 0x02;
}

void ledOFF(){
	PORTB = 0x00;
}

void modify(){
	analogLEDTest();
}

void analogLEDTest(){
	setupAnalog();
	for(;;){
		adc_value = ReadADC();
		if (adc_value > 30){
			PORTB = (1 << PORTB0);
		}
		if (adc_value > 60){
			PORTB = (1 << PORTB1);
		}
		if (adc_value > 90){
			PORTB = (1 << PORTB2);
		}
		if (adc_value > 120){
			PORTB = (1 << PORTB3);
		}
		if (adc_value > 150){
			PORTB = (1 << PORTB4);
		}
		if (adc_value > 180){
			PORTB = (1 << PORTB5);
		}
		if (adc_value > 210){
			PORTB = (1 << PORTB6);
		}
		if (adc_value > 240){
			PORTB = (1 << PORTB7);
		}
		if (adc_value >= 240){
			PORTB = 0xFF;
		}
	}
}


/***** Main Loop *****/
int main(void){
   setupPins();
   setupTimer();
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
		ledOFF();
		}
    }
}



/***** Timer Interrupts *****/



/*** Timer test, uses timer to increment 1 LED on per second, resets and does the loop again ***/

/*ISR(TIMER0_COMP_vect){


	extraTime++;
	if(extraTime > 100) // 100 = 1second
	{
		whichLED++;
		PORTB = (1 << PORTB0);
		//PORTB ^= (1 << PORTB0); //toggles on and off for every cycle.
		extraTime = 0;
	}
	switch(whichLED){
	case 1:
		PORTB = (1 << PORTB0) |(1 << PORTB1);
		break;
	case 2:
		PORTB = (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2);
		break;
	case 3:
		PORTB = (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3);
		break;
	case 4:
		PORTB = (1 << PORTB0) |(1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) |(1 << PORTB4);
		break;
	case 5:
		PORTB = (1 << PORTB0) |(1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) |(1 << PORTB4) |(1 << PORTB5);
		break;
	case 6:
		PORTB = (1 << PORTB0) |(1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) |(1 << PORTB4) |(1 << PORTB5) | (1 << PORTB6);
		break;
	case 7:
		PORTB = (1 << PORTB0) |(1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) |(1 << PORTB4) |(1 << PORTB5) | (1 << PORTB6) | (1 << PORTB7);
		break;
	case 8:
		whichLED = 0;
	}
}
*/
