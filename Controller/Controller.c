#include<io.h>
#include<util/delay.h>
#include<interrupt.h>


/***** Define Variables and constants *****/

int extraTime = 0, whichLED = 0, count = 0;
uint16_t adc_value;
#define F_CPU 8000000
#define BUAD 31250
#define BUAD_PRESCALE (((F_CPU / (BUAD * 16UL))) - 1)

/* Midi test inputs */

int MIDIon = 0b10010000;
int MIDIb1 = 0b00111100;
int MIDIb2 = 0b01111111;
int MIDIoff = 0b10000000;

/* Method Declarations */
void setupMIDI(unsigned int baudrate);
void setupPins();
void setupAnalog();
void setupTimer();

void record();
void playBack();
void modify();

void midiTransitTest();
void wait(int time);
void ledOFF();
uint16_t ReadADC();
void analogLEDTest();
void midi_transmit(unsigned char data);
unsigned char midi_Receive(void);
void midi_Flush(void);
unsigned char midi_ReadUCSRC(void);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);
void midiReceiveTest();






/***** Main Loop *****/
int main(void){
   setupPins();
   setupTimer();
   setupAnalog();
   setupMIDI(BUAD_PRESCALE);

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

/***** Setup Methods *****/

void setupMIDI(unsigned int baudrate){
	UBRRH = (unsigned char) (baudrate >> 8);
	UBRRL = (unsigned char) baudrate;
	UCSRB = (1 << TXEN) | (1 << RXEN);
	UCSRC = (1 << URSEL )|(1 << USBS) | (3 << UCSZ0); //only use 8 bit words
}
void setupPins(){
	DDRB = 0xFF;  //Set outputs
	DDRA = 0x00;  //Set inputs
	PORTB = 0x00; //Turns all leds off
	PORTA = 0x07; // sets inputs to return 5v on PA0, PA1, PA2
	DDRD = 0x00;
}
void setupAnalog(){
	ADMUX = (1 << REFS0) | (1<< MUX0) | (1<< MUX1) | (1<< MUX2); // sets the analog input of the photosensor to 0-5v, sets to look at the 7th analog pin.
	ADCSRA =  (1 << ADEN) | (1<< ADPS2) | (1<< ADPS1) | (1<< ADPS0); // ADEN turns ADC on; ADPS sets prescaler to 128;
}
void setupTimer(){
	TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS00); // Sets CTC mode for clock and sets prescaler to clk/1024
	OCR0 = 78; // every 78 ticks = .01 miliseconds (comparsion variable)
	TIMSK = (1 << OCIE0); // Set an interput whenver the ticks and my comarsion variable matchs)
	sei(); // needed to set interrupts
}



/***** Create Methods *****/



void record(){
	 
    //PORTB = 0x01;
    midiReceiveTest();
	//midiTransitTest();
	//midi_Receive();
}

void playBack(){
	PORTB = 0x02;
}

void modify(){
	analogLEDTest();
}

void midiReceiveTest(){
	if((midi_Receive()) != 0 ) {
		//did I recieve a message? do something!
		PORTB = 0xFF;	
	}
}

void midiTransitTest(){
	while ((UCSRA & (1 << UDRE)) == 0) {};

	midi_Transmit(MIDIon);
	midi_Transmit(MIDIb1);
	midi_Transmit(MIDIb2);
	midi_Transmit(MIDIoff);
}


void wait(int time)
{
   _delay_ms(time);
}

void ledOFF(){
	PORTB = 0x00;
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

void analogLEDTest(){
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

void midi_Transmit( unsigned char data){
	/* Wait for empty transmit buffer */
	while(! UCSRA & (1 << UDRE) );

	/* Put data into buffer, sends the data */
	UDR =  data;
}

unsigned char midi_Receive(void){
	/* Wait for data to be recieved */
	while(!(UCSRA & (1<<RXC)));

	/* get and return data from buffer */
	return UDR;
}

/* flushs the buffer of the midi */
void midi_Flush(void){ 
	unsigned char dummy;
	while (UCSRA & (1 << RXC) ) dummy = UDR;
}

unsigned char midi_ReadUCSRC(void){
	unsigned char ucsrc;
	/* read UCSRC */
	//note may need cli() to disable interrupts
	ucsrc = UBRRH;
	ucsrc = UCSRC;
	return ucsrc;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData){
	/* wait for completion of previous write */
	while (EECR & (1 <<EEWE));

	/* Set up address and data registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMWE */
	EECR |= (1 << EEMWE);
	/* Start eeporm write by setting EEWE */
	EECR |= (1 << EEWE);
}

unsigned char EEPROM_read(unsigned int uiAddress){
	/* wait for completion of previous write */
	while(EECR & (1<< EEWE));
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<< EERE);
	/* Return data from data register */
	return EEDR;
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
