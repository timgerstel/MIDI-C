#include<io.h>
#include<util/delay.h>
#include<interrupt.h>

int extraTime = 0, whichLED = 0;

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

uint16_t ReadADC(){
	//Start a single conversion
	ADCSRA |= (1 << ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//clear data (conversion is complete)
	ADCSRA |= (1 << ADIF);

	return(ADC);
}

void Wait()
{
   uint8_t i;
   for(i=0;i<20;i++)
      _delay_loop_2(0);
}

void record(){
	PORTB = 0x01; 
}

void playBack(){
	PORTB = 0x02;
}

void modify(){
	//PORTB = 0x04;
	TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS00); // Sets CTC mode for clock and sets prescaler to clk/1024
	OCR0 = 39; // every 39 ticks = .01 miliseconds (comparsion variable)
	TIMSK = (1 << OCIE0); // Set an interput whenver the ticks and my comarsion variable matchs)
	sei(); // needed to set interrupts

	ADMUX = (1 << REFS0) | (1<< MUX0) | (1<< MUX1) | (1<< MUX2); // sets the analog input of the photosensor to 0-5v, sets to look at the 7th analog pin.
	ADCSRA =  (1 << ADEN) | (1<< ADPS2) | (1<< ADPS1) | (1<< ADPS0); // ADEN turns ADC on; ADPS sets prescaler to 128;
	uint16_t adc_value;
    while(1){	
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
		Wait();
	}
}

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

	default:
		printf("Current led mode is %d\n", whichLED);
	}
}*/
