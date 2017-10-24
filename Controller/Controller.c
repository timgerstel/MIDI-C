#include<io.h>
#include<util/delay.h>
#include<interrupt.h>
#include<eeprom.h>

/***** Define Variables and constants *****/

int extraTime = 0, whichLED = 0, count = 0, notecount = 0, lastNoteTime = 0;
unsigned int eeprom_address=0x00, start_addr = 0x00, stop_addr;
uint16_t adc_value;
#define F_CPU 4000000
#define BAUD 31250
#define BAUD_PRESCALE (((F_CPU / (BAUD * 16UL))) - 1)

/* Midi test inputs */
unsigned char midiData[5];

/* Method Declarations */
void setupMIDI(unsigned int baudrate);
void setupPins();
void setupAnalog();
void setupTimer();

void record();
void playBack();
void modify();

void ledOFF();
uint16_t ReadADC();
void midi_transmit(unsigned char data);
unsigned char midi_Receive(void);
void midi_Flush(void);
unsigned char midi_ReadUCSRC(void);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);
void midiReceiveTest();
unsigned char TIM16_ReadTCNT1(void);
void playSong();
void writeSong2();


/***** Main Loop *****/
int main(void){
   setupPins();
   setupTimer();
   setupAnalog();
   setupMIDI(BAUD_PRESCALE);

    while(1){
		uint8_t rec = PINA & 0x04;
		uint8_t play = PINA & 0x02;
		uint8_t mod = PINA & 0x01;
	
		if(rec && !play){
			record();
		} else if(play && !rec){
			if (mod){ // Modify Mode
				modify();
			}else{

				playBack();
			}	
		} else {
			ledOFF();
			eeprom_address = 0x00;
			midi_Flush();
		}
		
    }
}
/***** Main Methods *****/

void record(){
	 writeSong2();
	//midiTransitTest();
}

void playBack(){
	//midiTransitTest();
	//eeprom_test();
	playSong();
}


void modify(){
	playSong();
}


/***** Setup Methods *****/

void setupMIDI(unsigned int baudrate){
	UBRRH = (unsigned char) (baudrate >> 8);
	UBRRL = (unsigned char) baudrate;
	UCSRB = (1 << TXEN) | (1 << RXEN);
	UCSRC = (1 << URSEL )|(0 << USBS) | (3 << UCSZ0); //only use 8 bit words
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
	TCCR1A = 0x00; // enable normal mode interrupts
	TCCR1B = (1 << CS10) | (1 << CS12); //prescaler 1024
	TIMSK = (1 << OCIE1B);
	sei();
	//OCR1A = 3906; // 1000ms delay  equation = (1000*10^-3/(1/3906.25));
	OCR1B = 1952; // 500ms Delay (note this causes the leds to turn off after button press)
	TCNT1 = 0;

}

/***** Create Methods *****/

void writeSong2(){
	uint8_t lsb;
	uint8_t msb;
	for(int i = 0; i <3; i++){
		midiData[i] = midi_Receive();
		if(i==0){
			 lsb = TCNT1&0xFF;
			 msb = (TCNT1>>8);
			 TCNT1 = 0;
		}
	}
	midiData[3]= lsb;
	midiData[4]= msb;
	PORTB = midiData[1];
	
	
	for(int j= 0; j < 5; j++){
		EEPROM_write(eeprom_address, midiData[j]);
		eeprom_address++;		
	}
	stop_addr = eeprom_address;
	
}

void playSong(){
	while(start_addr < stop_addr && (PINA & 0x02) ){
		float speedMod = 1;

		for(int i = 0; i < 5; i++){

			midiData[i] = EEPROM_read(start_addr);
			start_addr++;
			if(i==4){
				TCNT1 = 0;
			}
		}

		uint16_t lsb = midiData[3];
		uint16_t msb = midiData[4];
		uint16_t timeInterval = lsb + (0xFF00 & (msb << 8) );
		if((PINA & 0x02 ) && (PINA & 0x01)){

		if(ReadADC() > 0 && ReadADC() < 60){
			speedMod = 3;
		}else if(ReadADC() > 60 && ReadADC() < 200){
			speedMod = .5;
		}
		else if(ReadADC() > 200){
			speedMod = .1;
		}
		}else{
			speedMod = 1;
		}
		

		if(start_addr != 5){
			while(TCNT1 < timeInterval * speedMod);
		}
		
		for(int i = 0; i < 3; i++){
			midi_Transmit(midiData[i]);
			if(i==1){
				PORTB = midiData[i];
			}		
		}
		
	}
	start_addr = 0;
	
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



void midi_Transmit( unsigned char data){
	/* Wait for empty transmit buffer */
	while((PINA & 0x02) && !(UCSRA & (1 << UDRE)) ) ;

	/* Put data into buffer, sends the data */
	UDR = data;
}

unsigned char midi_Receive(void){
	/* Wait for data to be recieved */
	while( (PINA&0x04)&&(!(UCSRA & (1<<RXC)) ));

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

unsigned char TIM16_ReadTCNT1(void){
	unsigned char sreg;
	unsigned char i;
	/* save gloval interrupt flag */
	sreg = SREG;
	/* disable interrupts */
	cli();
	/* read TCNT1 into i */
	i = TCNT1;
	/* restore global interrupt flag */
	SREG = sreg;
	return i;
}

void TIM16_WriteTCNT1 (unsigned int i){
	unsigned char sreg;
	/* save global interrupt flag */
	sreg = SREG;
	/* disable interrupts */
	cli();
	/* set tcnt1 to i */
	TCNT1 = i;
	/* restore global interrupt flag */
	SREG = sreg;
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
	//SREG = cSREG;
}

unsigned char EEPROM_read(unsigned int uiAddress){
	/* wait for completion of previous write */
	while(EECR & (1<< EEWE));
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<< EERE);
	/* Return data from data register */
	//SREG = cSREG;
	return EEDR;
}

/***** Timer Interrupts *****/

ISR(TIMER1_COMPB_vect){
	PORTB = 0x00; // blink
	 //Reset timer
}