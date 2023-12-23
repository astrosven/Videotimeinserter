/*Videotimeinserter

  benoetigt als GPS-RX einen GPS18LVC

  ATMega8 laeuft mit 18,432MHz, ATMega168 sollte auch funktionieren (pinkompatibel,aber andere Registernamen,ausprobieren!)

V1.7 20070226

Diese Software darf für nichtkommerzielle astronomische und Amateurfunkanwendungen genutzt werden.

Die Software darf verändert werden, die geänderte Software muß wieder veröffentlicht werden!

 */

/*******************************************************************************

				Main Video Overlay function library.


Created:		1.00	12/12/04	GND	Gary Dion

Revisions:	1.01	12/15/04	GND	Improved screen font and added background

Copyright(c)	2004, Gary N. Dion (me@garydion.com). All rights reserved.
					This software is available only for non-commercial amateur radio
					or educational applications.  ALL other uses are prohibited.
					This software may be modified only if the resulting code be
					made available publicly and the original author(s) given credit.

*******************************************************************************/
 

/* OS headers */
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
/* General purpose include files */
//#include "StdDefines.h"

/* App required include files. */
//#include "Main.h"
//#define F_CPU 18432000
#define MUXAUS()  PORTD |= (1<<PD7) //Multiplexer umschalten auf Einblendung (vertauscht bei V2!!!)
#define MUXAN() PORTD &= ~ (1<<PD7) //Multiplexer umschalten auf Video
//#define MUXAN()  PORTD |= (1<<PD7) // für Hardware V1.0 Multiplexer umschalten auf Einblendung (vertauscht bei V2!!!)
//#define MUXAUS() PORTD &= ~ (1<<PD7) //für Hardware V1.0 Multiplexer umschalten auf Video

#define Warten_USART_RX () while (!(UCSRA & (1<< RXC))) //auf ein Zeichen in der USART warten
#define WatchdogReset() asm("wdr")	/* Macro substitution to kick the dog		*/
#define Wait() while(!(SPSR & (1<<SPIF))) //warten bis per SPI ein Bytes ausgegeben ist

/* Static functions and variables */

unsigned volatile char groklein = 1;
unsigned volatile char ppssta = 0;
unsigned volatile short adc = 500 ;
unsigned char laenge = 5;

unsigned short wert;
unsigned volatile char gpssta[1]={'-'};
unsigned char i = 0;
unsigned char z = 0;
unsigned volatile char oesta = 1; //even/odd-Bild
unsigned volatile char sta = 255;   //Status des PPS-Signals
unsigned volatile char vsta1 = 1;
unsigned char buf ;
unsigned volatile char s[5];
unsigned char pps[2] = {'@','P'};
unsigned char odd[2] = {'O','@'};
unsigned char even[2] = {'@','E'};
unsigned short msec = 9000;
unsigned volatile short line = 0;					/* Videozeile						*/
unsigned volatile char meld;
unsigned volatile char meld1;
unsigned volatile char groklei;
short fonttemp;
//unsigned short laufz = 0;
//short ltemp;						/* State of Delay function						*/
//short ntemp;						/* State of Delay function						*/

unsigned volatile char head[20] = {'S','V','E','N','S','O','F','T','@','V','T','I','@','V','2','.','1','@','@','@'};

unsigned volatile char ha[7] = {'1','2','3','4','5','6','7'};
unsigned volatile char hg[6] = {'1','2','3','4','5','6'};
unsigned volatile char qual[1] = {'0'};
unsigned volatile char sat[2] = {'9','9'};
unsigned volatile char time[12] = {'9','9','9','9','9','9','8','8','8','8','8','8'};
unsigned volatile char time1[6] = {'9','9','9','9','9','9'};
unsigned volatile char date[6] = {'9','9','9','9','9','9'};

unsigned volatile char lat[10] = {'N','9','9','9','9','.','9','9','9','9'};
unsigned volatile char lon[11] = {'1','7','9','9','9','.','9','9','9','9','W'};




/* 8*7 Font , fängt mit ASCII 32 an, bis   90(Z)            */
/*unsigned char font[59][6] ={
							{255,255,255,255,255,255,255},//SPACE
							{0,0,0,0,0,0,0},//!
							{0,0,0,0,0,0,0},//"
							{0,0,0,0,0,0,0},//#
							{0,0,0,0,0,0,0},//$
							{0,0,0,0,0,0,0},//%
							{0,0,0,0,0,0,0},//&
							{0,0,0,0,0,0,0},//'
							{0,0,0,0,0,0,0},//(
							{0,0,0,0,0,0,0},//)
							{0,0,0,0,0,0,0},// *
							{0,0,0,0,0,0,0},//+
							{0,0,0,0,0,0,0},//,
							{255,255,255,131,255,255,255},//-
							{255,255,255,255,255,231,231},//.
							{207,187,187,207,255,255,255},///
							{131,125,125,125,125,125,131},//0
							{239,207,239,239,239,239,199},//1
							{131,125,253,243,207,191,  1},//2
							{131,125,253,227,253,125,131},//3
							{227,219,187,123,  1,251,251},//4
							{  1,127,127,131,253,125,131},//5
							{131,127,127,  3,125,125,131},//6
							{  1,253,251,247,239,239,239},//7
							{131,125,125,131,125,125,131},//8
							{131,125,125,129,251,247,207},//9
							{255,239,255,255,255,239,255},//:
							{0,0,0,0,0,0,0},//;
							{0,0,0,0,0,0,0},//<
							{0,0,0,0,0,0,0},//=
							{0,0,0,0,0,0,0},//>
							{0,0,0,0,0,0,0},//?
							{0,0,0,0,0,0,0},//@
							{231,219,189,189,129,189,189},//A
							{131,189,189,131,189,189,131},//B
							{195,189,191,191,191,189,195},//C
							{135,187,189,189,189,187,135},//D
							{129,191,191,131,191,191,129},//E
							{129,191,191,131,191,191,191},//F
							{195,189,191,177,189,189,195},//G
							{189,189,189,129,189,189,189},//H
							{131,239,239,239,239,239,131},//I
							{129,247,247,247,247,183,207},//J
							{189,187,183,143,183,187,189},//K
							{191,191,191,191,191,129,129},//L
							{125, 57, 85,109,125,125,125},//M
							{125, 61, 93,109,117,121,125},//N
							{195,189,189,189,189,189,195},//O
							{131,189,189,131,191,191,191},//P
							{195,189,189,189,179,185,205},//Q
							{131,189,189,131,183,187,189},//R
							{195,189,191,195,253,189,195},//S
							{131,239,239,239,239,239,239},//T
							{189,189,189,189,189,189,195},//U
							{189,189,189,219,219,219,231},//V
							{125,125,109,109,109,147,147},//W
							{125,187,215,239,215,187,125},//X
							{125,187,215,239,239,239,239},//Y
							{  1,251,247,239,223,191,  1},//Z
														
							}; */
//Zeichensatz
/*							' '  A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z  */
/*unsigned char ltrs[189] = { 255,231,131,195,135,129,129,195,189,131,129,189,191,125,125,195,131,195,131,195,131,189,189,125,125,125,  1,
					    	255,219,189,189,187,191,191,189,189,239,247,187,191, 57, 61,189,189,189,189,189,239,189,189,125,187,187,251,
							255,189,189,191,189,191,191,191,189,239,247,183,191, 85, 93,189,189,189,189,191,239,189,189,109,215,215,247,
							255,189,131,191,189,131,131,177,129,239,247,143,191,109,109,189,131,189,131,195,239,189,219,109,239,239,239,
							255,129,189,191,189,191,191,189,189,239,247,183,191,125,117,189,191,179,183,253,239,189,219,109,215,239,223,
							255,189,189,189,187,191,191,189,189,239,183,187,129,125,121,189,191,185,187,189,239,189,219,147,187,239,191,
							255,189,131,195,135,129,191,195,189,131,207,189,129,125,125,195,191,205,189,195,239,195,231,147,125,239,  1};

/*							-   .   /   0   1   2   3   4   5   6   7   8   9   :  */
/*unsigned char nums[98] = {255,255,207,131,239,131,131,227,  1,131,  1,131,131,255,
						  255,255,187,125,207,125,125,219,127,127,253,125,125,239,
						  255,255,187,125,239,253,253,187,127,127,251,125,125,255,
						  131,255,207,125,239,243,227,123,131,  3,247,131,129,255,
						  255,255,255,125,239,207,253,  1,253,125,239,125,251,255,
						  255,231,255,125,239,191,125,251,125,125,239,125,247,239,
						  255,231,255,131,199,  1,131,251,131,131,239,131,207,255};

/*						    !  ' '  "   #   $   %   &   "   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?  '@'  A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z  */
unsigned char font[413] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,207,131,239,131,131,227,  1,131,  1,131,131,255,255,255,255,255,255,255,231,131,195,135,129,129,195,189,131,129,189,191,125,125,195,131,195,131,195,131,189,189,125,125,125,  1,
					       255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,187,125,207,125,125,219,127,127,253,125,125,239,255,255,255,255,255,255,219,189,189,187,191,191,189,189,239,247,187,191, 57, 61,189,189,189,189,189,239,189,189,125,187,187,251,
						   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,187,125,239,253,253,187,127,127,251,125,125,255,255,255,255,255,255,255,189,189,191,189,191,191,191,189,239,247,183,191, 85, 93,189,189,189,189,191,239,189,189,109,215,215,247,
						   255,255,255,255,255,255,255,255,255,255,255,255,255,131,255,207,125,239,243,227,123,131,  3,247,131,129,255,255,255,255,255,255,255,189,131,191,189,131,131,177,129,239,247,143,191,109,109,189,131,189,131,195,239,189,219,109,239,239,239,
						   255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,125,239,207,253,  1,253,125,239,125,251,255,255,255,255,255,255,255,129,189,191,189,191,191,189,189,239,247,183,191,125,117,189,191,179,183,253,239,189,219,109,215,239,223,
						   255,255,255,255,255,255,255,255,255,255,255,255,255,255,231,255,125,239,191,125,251,125,125,239,125,247,239,255,255,255,255,239,255,189,189,189,187,191,191,189,189,239,183,187,129,125,121,189,191,185,187,189,239,189,219,147,187,239,191,
						   255,255,255,255,255,255,255,255,255,255,255,255,255,255,231,255,131,199,  1,131,251,131,131,239,131,207,255,255,255,255,255,255,255,189,131,195,135,129,191,195,189,131,207,189,129,125,125,195,191,205,189,195,239,195,231,147,125,239,  1};




/*#define void LongToString( wert, laenge, char* s) {for(i = 0, i < laenge, i++) {
      s[laenge-i-1] = (wert % 10) + 48;
      wert = wert / 10;
   }
   s[laenge] = '\0';
}*/

/*$FUNCTION$*******************************************************************/
int	main(void)
/*******************************************************************************
* ABSTRACT:	Main program entry function.
*
* INPUT:		None
* OUTPUT:	None
*/
{


	PORTB = 0x00;
	DDRB = 0B11111110;							/* PB0 Eingabe, alle anderen PB Ausgabe		*/

	PORTC = 0xFF ;								 //Pull up ein   
	DDRC  = 0x00 ;								 //alles auf input C0 AIN C1,2,3,4,5 digital

	PORTD = 0B10111111;							/* Initial state is both LEDs off			*/
	DDRD  = 0B10000000;							/* D7 Ausgabe Data direction register for port D		*/

    ADMUX  = 0B01100000 ; 						//MUX auf C0 setzen
	ADCSRA = 0B11100111 ;						//ADC an, Freerunmode

	SPCR = /* (1<<SPIE) | */ (1<<SPE) | (1<<MSTR) | (1<<CPHA); //SPI einschalten
	SPSR = 1;

#if defined (__AVR_ATmega8__)

//USART initialisieren
    UBRRH = 0;
	UBRRL = 29; //239 = 4800Baud bei 18.432MHz 29 = 38400
	UCSRB = (1<<RXEN)|(1<<TXEN);
    UCSRC = (1<<URSEL)  | (3<<UCSZ0) ; //8n1

//Timer1 initialisieren
	OCR1A = 18000 ;
	TCCR1A = 0B01000011;				/*mode15  OCR1A aktiv*/
	TCCR1B = 0B00011101;				/* Timer1 clock prescale of 1024 CaptureIR mode15					*/
     TIMSK = 0B00100100;				//TimerOverFlowIR und CaptureIR an
	 

	MCUCR	= (1<<ISC01) + (1<<ISC11) + (1<<ISC10);	/* Set interrupt0 on falling edge, IR1 rise			*/
	GICR	= (1<<INT0) + (1<<INT1);	/* Enable interrupt for interrupt0+1			*/

/* Enable the watchdog timer */
//	WDTCR	= (1<<WDCE) | (1<<WDE);		/* Wake-up the watchdog register				*/
//	WDTCR	= (1<<WDE) | 7;				/* Enable and timeout around 2.1s			*/
#endif

#if defined (__AVR_ATmega168__)

	UBRR0H = 0; //38400Baud
	UBRR0L =29;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (1<<UCSZ01)  | (1<<UCSZ00) ; //8n1


	EICRA = (1<<ISC01) + (1<<ISC11) + (1<<ISC10);
	EIMSK = (1<<INT0) + (1<<INT1);	/* Enable interrupt for interrupt0+1			*/

	OCR1A = 18000 ;
	TCCR1A = 0B01000011;							/*mode15  OCR1A aktiv*/
	TCCR1B = 0B00011101;							/* Timer2 clock prescale of 1024 CaptureIR mode15					*/
    TIMSK1 = 0B00100101;							//TimerOverFlow und CaptureIR freigeben

#define UDR  UDR0       //Registernamen umdefinieren
#define UCSRA  UCSR0A
#define RXC  RXC0	 


#endif
/* Enable interrupts */
	sei();

/* Reset watchdog */
/*	WatchdogReset();*/


//LongToString(msec, 4, s);
//set_sleep_mode(SLEEP_MODE_IDLE);

/* Initialization complete - system ready.  Run program loop indefinitely. */

buf = UDR;
//adc = ADCH + 50;
//adc = ADCH + 50;
//asm volatile ("nop"::); //nur zum testen

while (time1[4] != '1') {meld = 1;} //9s Kopfzeile zeigen

head[0] =  'M'; //Kopfzeile mit anderem Text versehen
head[1] =  'Y'; //here entry your observatory name
head[2] =  '@';
head[3] =  'O';
head[4] =  'B'; 
head[5] =  'S';
head[6] =  'E';
head[7] =  'R';
head[8] =  'T'; 
head[9] =  'O';
head[10] = 'R';
head[11] = 'Y';
head[12] = '@';
head[13] = '@';
head[14] = 'X';
head[15] = 'X';
head[16] = 'X';
head[17] = '-';
head[18] = '5';
head[19] = '@';




while(1) //Daten vom GPS einlesen und einordnen
      {	

   
		/*if (vsta1 == 0) {   //nur einmal nach dem Vsynch ausführen!

          msec = msec/18 ;//18000 ist 1s /18 ergibt wert in ms


			//Zahl in String zerlegen
			for(i = 0; i < laenge ; i++) {
      		s[laenge-i-1] = (msec % 10) + 48;
      		msec = msec / 10;
   								}

			vsta1 = 1;}*/
        
        if ((UCSRA & (1<< RXC)))  {buf = UDR; 
				if (buf == 77) //M?
					{while (!(UCSRA & (1<< RXC))) ; buf = UDR; /*Ist ein Zeichen im RX-Puffer, wenn ja, abholen*/
						if (buf == 67) //C?
						{
			  			     
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); time1[0] = UDR;
					while (!(UCSRA & (1<< RXC))); time1[1] = UDR;
					while (!(UCSRA & (1<< RXC))); time1[2] = UDR;
					while (!(UCSRA & (1<< RXC))); time1[3] = UDR;
					while (!(UCSRA & (1<< RXC))); time1[4] = UDR;
					while (!(UCSRA & (1<< RXC))); time1[5] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR; //Komma
					while (!(UCSRA & (1<< RXC))); gpssta[0] = UDR; //StatusGPS-RX A - aktuell V - falsch
					if (gpssta[0] == 'A') {gpssta[0] = ':';} else {gpssta[0] = '-';};
					while (!(UCSRA & (1<< RXC))); buf = UDR; //Komma
					while (!(UCSRA & (1<< RXC))); lat[1] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[2] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[3] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[4] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//Punkt
					while (!(UCSRA & (1<< RXC))); lat[6] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[7] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[8] = UDR;
					while (!(UCSRA & (1<< RXC))); lat[9] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//komma
					while (!(UCSRA & (1<< RXC))); lat[0] = UDR;//N-S
					while (!(UCSRA & (1<< RXC))); buf = UDR;//komma
					while (!(UCSRA & (1<< RXC))); lon[0] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[1] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[2] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[3] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[4] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//Punkt
					while (!(UCSRA & (1<< RXC))); lon[6] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[7] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[8] = UDR;
					while (!(UCSRA & (1<< RXC))); lon[9] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//Komma
					while (!(UCSRA & (1<< RXC))); lon[10] = UDR;//E-W
					while (!(UCSRA & (1<< RXC))); buf = UDR;//komma
					while (!(UCSRA & (1<< RXC))); buf = UDR;//Hier Kommas abfragen, fall etwas anderes kommt
					 if (buf ==',') {goto weiter;};
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//Geschwindigkeit und Kurs
					while (!(UCSRA & (1<< RXC))); buf = UDR;
				    while (!(UCSRA & (1<< RXC))); buf = UDR;//komma
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
		weiter:			while (!(UCSRA & (1<< RXC))); buf = UDR;//komma	
					while (!(UCSRA & (1<< RXC))); date[4] = UDR;
					while (!(UCSRA & (1<< RXC))); date[5] = UDR;
					while (!(UCSRA & (1<< RXC))); date[2] = UDR;
					while (!(UCSRA & (1<< RXC))); date[3] = UDR;
					while (!(UCSRA & (1<< RXC))); date[0] = UDR;
					while (!(UCSRA & (1<< RXC))); date[1] = UDR;

                /*    time[5]++ ; //empfangene Zeit um 1sec erhöhen, wird beim nächsten PPS eingeblendet
if (time[11-ppssta] > 57) {time[11-ppssta] = 48; time[10-ppssta]++;
	if (time[10-ppssta] > 53) {time[10-ppssta] = 48 ; time[9-ppssta] ++ ;
        if (time[9-ppssta] > 57) {time[9-ppssta] = 48 ; time[8-ppssta] ++ ; 
			if (time[8-ppssta] > 53) {time[8-ppssta] = 48 ; time[7-ppssta] ++ ;
				if (time[6-ppssta] == 50 && time[7-ppssta] > 52) {time[7-ppssta] = 48;} //hier nochmal ansehen
				if (time[7-ppssta] > 57) {time[7-ppssta] = 48 ; time[6-ppssta] ++ ;
					if (time[6-ppssta] > 50) {time[6-ppssta] = 48  ;}}}}}} */
					}}
  	  	}
                   if ((UCSRA & (1<< RXC)))  {buf = UDR; 
  					if (buf == 'G') //G? 71
					{while (!(UCSRA & (1<< RXC))) ; buf = UDR; /*Ist ein Zeichen im RX-Puffer, wenn ja, abholen*/
						if (buf == 'A') //A? 65
						{
					  for (z=0;z<=32;z++){
				    while (!(UCSRA & (1<< RXC))); buf = UDR;}//,
					
					while (!(UCSRA & (1<< RXC))); qual[0] = UDR;//Qualität
					while (!(UCSRA & (1<< RXC))); buf = UDR;//,
					while (!(UCSRA & (1<< RXC))); sat[0] = UDR;//Anzahl der empfangenen Sateliten
					while (!(UCSRA & (1<< RXC))); sat[1] = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;//,
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					if (buf == ',') {goto weiter3;}         //wenn , dann kommt gleich die Höhe 
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					if (buf=='.'){
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;}//,
					else {
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;
					while (!(UCSRA & (1<< RXC))); buf = UDR;}//,
		weiter3:	while (!(UCSRA & (1<< RXC))); ha[0] = UDR;//Höhe der Antenne
					if (ha[0] == ',') {ha[0]='-';ha[1]='-';ha[2]='-';ha[3]='-';ha[4]='-';ha[5]='-';ha[6]='-';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[1] = UDR;
					if (ha[1] == ',') {ha[1]='0';ha[2]='0';ha[3]='0';ha[4]='0';ha[5]='0';ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[2] = UDR;
					if (ha[2] == ',') {ha[2]='0';ha[3]='0';ha[4]='0';ha[5]='0';ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[3] = UDR;
					if (ha[3] == ',') {ha[3]='0';ha[4]='0';ha[5]='0';ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[4] = UDR;
					if (ha[4] == ',') {ha[4]='0';ha[5]='0';ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[5] = UDR;
					if (ha[5] == ',') {ha[5]='0';ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); ha[6] = UDR;
					if (ha[6] == ',') {ha[6]='0';goto weiter1;}
					while (!(UCSRA & (1<< RXC))); buf = UDR;//,
		weiter1:	while (!(UCSRA & (1<< RXC))); buf = UDR;//M
					while (!(UCSRA & (1<< RXC))); buf = UDR;//,
					while (!(UCSRA & (1<< RXC))); hg[0] = UDR;//Höhe Geoid
					if (hg[0] == ',') {hg[0]='-';hg[1]='-';hg[2]='-';hg[3]='-';hg[4]='-';hg[5]='-';goto weiter2;}
					while (!(UCSRA & (1<< RXC))); hg[1] = UDR;
					if (hg[1] == ',') {hg[1]='0';hg[2]='0';hg[3]='0';hg[4]='0';hg[5]='0';goto weiter2;}
					while (!(UCSRA & (1<< RXC))); hg[2] = UDR;
					if (hg[2] == ',') {hg[2]='0';hg[3]='0';hg[4]='0';hg[5]='0';goto weiter2;}
					while (!(UCSRA & (1<< RXC))); hg[3] = UDR;
					if (hg[3] == ',') {hg[3]='0';hg[4]='0';hg[5]='0';goto weiter2;}
					while (!(UCSRA & (1<< RXC))); hg[4] = UDR;//.
					if (hg[4] == ',') {hg[4]='0';hg[5]='0';goto weiter2;}
					while (!(UCSRA & (1<< RXC))); hg[5] = UDR;
					if (hg[5] == ',') {hg[5]='0';goto weiter2;}}}}
	    weiter2:
//$GPRMC,hhmmss,A,ddmm.mmmm,N,dddmm.mmmm,E,000.0,000.0,ddmmyy,000.0,E,A*hh<CR><LF> 		 
//       Zeit     Lat         Lon                      Datum
//$GPGGA,hhmmss,ddmm.mmmm,N.dddmm.mmmm,E,0,00,0.5(99.9),hhhhh.h,M,hhhh.h,M,x,x
//       Zeit   Lat         Lon          

		
/*	
void LongToString(long wert, unsigned int laenge, char* s) {
   int i;
   
   for(i = 0; i < laenge; i++) {
      s[laenge-i-1] = (wert % 10) + 48;
      wert = wert / 10;
   }
   s[laenge] = '\0';
}



 char s[10];

long x = 1234567;

LongToString(x, 7, s); 

*/

	
	
//$GPRMC,hhmmss,A,ddmm.mmmm,N,dddmm.mmmm,E,000.0,000.0,ddmmyy,000.0,E,A*hh<CR><LF> 		 
//       Zeit     Lat         Lon                      Datum
		
		
		buf = 0;
		if (!( PINC & (1<<PINC5)))  {meld = 0;} //Kopfzeile ein/aus
		else {meld = 1;};
	    if (!( PINC & (1<<PINC4)))  {meld1 = 0;}//Position ein/aus
		else {meld1 = 1;};
	    if (!( PINC & (1<<PINC3)))  {groklei = 0;}//Schriftgröße der Zeitzeile
		else {groklei = 1;};
	  
	  
	  adc = ADCH + 50;  //Poti abfragen, ergibt Position der Zeitzeile
		}



return (0);}	



/*$FUNCTION$*******************************************************************/
ISR(INT0_vect)//SIGNAL(SIG_OVERFLOW0)
/*******************************************************************************
* 
* INPUT:		None
* OUTPUT:	None
*/
{

	//TIMSK &= ~(1<<TOIE0);				/* Disable the timer0 interrupt				*/
   line ++  ;
	if ((line > 50) && (line < 58))
	{
	  if (meld == 1) {
	
		/*ltemp = (line - 51) * 27 - 64;
		ntemp = (line - 51) * 14 - 45;  */
		fonttemp = (line-51) * 59 - 32;
		
		SPSR = 0;
		
   _delay_loop_1 (40);

//Kopfzeile ausgeben

	MUXAN();
	   	/*SPDR = ltrs[head[0]+ltemp];		Wait();
		SPDR = ltrs[head[1]+ltemp];		Wait();
		SPDR = ltrs[head[2]+ltemp];		Wait();
		SPDR = ltrs[head[3]+ltemp];		Wait();
		SPDR = ltrs[head[4]+ltemp];		Wait();
		SPDR = ltrs[head[5]+ltemp];		Wait();
		SPDR = ltrs[head[6]+ltemp];		Wait();
		SPDR = ltrs[head[7]+ltemp];		Wait();
		SPDR = ltrs[head[8]+ltemp];		Wait();
		SPDR = ltrs[head[9]+ltemp];		Wait();
		SPDR = ltrs[head[10]+ltemp];    Wait();
		SPDR = ltrs[head[11]+ltemp];    Wait();
		SPDR = ltrs[head[12]+ltemp];    Wait();
		SPDR = ltrs[head[13]+ltemp];    Wait();
	    SPDR = ltrs[head[14]+ltemp];    Wait();
		SPDR = ltrs[head[15]+ltemp];    Wait();
	    SPDR = ltrs[head[16]+ltemp];    Wait();*/
		SPDR = font[head[0]+fonttemp];	Wait();
		SPDR = font[head[1]+fonttemp];	Wait();
		SPDR = font[head[2]+fonttemp];	Wait();
		SPDR = font[head[3]+fonttemp];	Wait();
		SPDR = font[head[4]+fonttemp];	Wait();
		SPDR = font[head[5]+fonttemp];	Wait();
		SPDR = font[head[6]+fonttemp];	Wait();
		SPDR = font[head[7]+fonttemp];	Wait();
		SPDR = font[head[8]+fonttemp];	Wait();
		SPDR = font[head[9]+fonttemp];	Wait();
		SPDR = font[head[10]+fonttemp];	Wait();
		SPDR = font[head[11]+fonttemp];	Wait();
		SPDR = font[head[12]+fonttemp];	Wait();
		SPDR = font[head[13]+fonttemp];	Wait();
		SPDR = font[head[14]+fonttemp];	Wait();
		SPDR = font[head[15]+fonttemp];	Wait();
		SPDR = font[head[16]+fonttemp];	Wait();
		SPDR = font[head[17]+fonttemp];	Wait();
		SPDR = font[head[18]+fonttemp];	Wait();
		SPDR = font[head[19]+fonttemp];	Wait();



	MUXAUS();
	
	}};

	
	if ((line > 260/*adc*/) && (line < 268/*adc + 8*/))
	{ if (meld1 == 1){
		//ltemp = (line - 261/*adc + 1*/) * 27 - 64;
		//ntemp = (line - 261/*adc + 1*/) * 14 - 45;
        fonttemp = (line-261) * 59 - 32;
	  // SPSR = 1;
	//MUXAN ();
		//SPDR = nums[time[0] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[time[1] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[':' + ntemp];		Wait();		/* ':'							*/
		//SPDR = nums[time[2] + ntemp];	Wait();		/* Minutes						*/
		//SPDR = nums[time[3] + ntemp];	Wait();		/* Minutes						*/
		//SPDR = nums[':' + ntemp];		Wait();		/* ':'							*/
		//SPDR = nums[time[4] + ntemp];	Wait();		/* Seconds						*/
		//SPDR = nums[time[5 ] + ntemp];	Wait();		/* Seconds						*/
		//SPDR = ltrs['@' + ltemp];		Wait();		/* ' 'bis MUXAN nur Verzögerung							*/
		//SPDR = ltrs['@' + ltemp];		Wait();		// ' '	
		//SPDR = ltrs['@' + ltemp];		Wait();		// ' '
		//SPDR = ltrs['@' + ltemp];		Wait();		// ' '
        _delay_loop_1 (34);
	  
	SPSR =0;
	//line1 = line - 261;		
	MUXAN ();													/* Switch to large font		*/
//		SPDR = ltrs[lat[0]+ ltemp];	Wait();		/* Degrees						*/
//		SPDR = ltrs['@'+ ltemp];		Wait();		/* ' '							*/
//		SPDR = nums[lat[1]+ ntemp];	Wait();		/* Degrees						*/
//		SPDR = nums[lat[2]+ ntemp];	Wait();		/* Degrees						*/
//		SPSR = 1;											/* Switch to small font		*/
//		SPDR = nums['/'+ ntemp];		Wait();		/* Print degrees symbol		*/
//		SPSR = 0;											/* Switch to large font		*/
//		SPDR = nums[lat[3]+ ntemp];	Wait();		/* Minutes						*/
//		SPDR = nums[lat[4]+ ntemp];	Wait();		/* Minutes						*/
//		SPSR = 1;											/* Switch to small font		*/
//		SPDR = nums[lat[5]+ ntemp];	Wait();		/* '.'							*/
//		SPSR = 0;											/* Switch to large font		*/
//		SPDR = nums[lat[6]+ ntemp];	Wait();		/* Decimal Minutes			*/
//		SPDR = nums[lat[7]+ ntemp];	Wait();		/* Decimal Minutes			*/
//        SPDR = nums[lat[8]+ ntemp];	Wait();		/* Decimal Minutes			*/
//        SPDR = nums[lat[9]+ ntemp];	Wait();		/* Decimal Minutes			*/
    		
//		SPSR = 1;
		//SPDR = font[' ' - 32][line-261];		Wait();		/* ' '*/
//		SPDR = ltrs['A'+ ltemp];		Wait();		// 'A'
//		SPDR = nums[ha[0]+ ntemp];	Wait();		/* Höhe Antenne			*/
//		SPDR = nums[ha[1]+ ntemp];	Wait();		/* 			*/
//        SPDR = nums[ha[2]+ ntemp];	Wait();		/* 			*/
//        SPDR = nums[ha[3]+ ntemp];	Wait();	
//        SPDR = nums[ha[4]+ ntemp];	Wait();		/* 			*/
//	    SPDR = nums[ha[5]+ ntemp];	Wait();		/* 			*/
//        SPDR = nums[ha[6]+ ntemp];	Wait();		/* 			*/
//        SPDR = ltrs['M'+ ltemp];		Wait();		// 'M'
//	    SPDR = ltrs['@'+ ltemp];		Wait();		/* ' '*/
	//	SPDR = font[' ' - 32][line-261];		Wait();		/* ' '*/ 
//	    SPDR = ltrs['Q'+ ltemp];		Wait();		/* 'Q'*/ 
//		SPDR = nums[qual[0]+ ntemp];	Wait();		/* Qualität			*/

		SPDR = font[lat[0]+ fonttemp];	Wait();		/* Degrees						*/
		SPDR = font['@'+ fonttemp];		Wait();		/* ' '							*/
		SPDR = font[lat[1]+ fonttemp];	Wait();		/* Degrees						*/
		SPDR = font[lat[2]+ fonttemp];	Wait();		/* Degrees						*/
		SPSR = 1;											/* Switch to small font		*/
		SPDR = font['/'+ fonttemp];		Wait();		/* Print degrees symbol		*/
		SPSR = 0;											/* Switch to large font		*/
		SPDR = font[lat[3]+ fonttemp];	Wait();		/* Minutes						*/
		SPDR = font[lat[4]+ fonttemp];	Wait();		/* Minutes						*/
		SPSR = 1;											/* Switch to small font		*/
		SPDR = font[lat[5]+ fonttemp];	Wait();		/* '.'							*/
		SPSR = 0;											/* Switch to large font		*/
		SPDR = font[lat[6]+ fonttemp];	Wait();		/* Decimal Minutes			*/
		SPDR = font[lat[7]+ fonttemp];	Wait();		/* Decimal Minutes			*/
        SPDR = font[lat[8]+ fonttemp];	Wait();		/* Decimal Minutes			*/
        SPDR = font[lat[9]+ fonttemp];	Wait();		/* Decimal Minutes			*/
    		
		SPSR = 1;
		//SPDR = font[' ' - 32][line-261];		Wait();		/* ' '*/
		SPDR = font['A'+ fonttemp];		Wait();		// 'A'
		SPDR = font[ha[0]+ fonttemp];	Wait();		/* Höhe Antenne			*/
		SPDR = font[ha[1]+ fonttemp];	Wait();		/* 			*/
        SPDR = font[ha[2]+ fonttemp];	Wait();		/* 			*/
        SPDR = font[ha[3]+ fonttemp];	Wait();	
        SPDR = font[ha[4]+ fonttemp];	Wait();		/* 			*/
	    SPDR = font[ha[5]+ fonttemp];	Wait();		/* 			*/
        SPDR = font[ha[6]+ fonttemp];	Wait();		/* 			*/
        SPDR = font['M'+ fonttemp];		Wait();		// 'M'
	    SPDR = font['@'+ fonttemp];		Wait();		/* ' '*/
	//	SPDR = font[' ' - 32][line-261];		Wait();		/* ' '*/ 
	    SPDR = font['Q'+ fonttemp];		Wait();		/* 'Q'*/ 
		SPDR = font[qual[0]+ fonttemp];	Wait();		/* Qualität			*/

	MUXAUS();
	}}

    
	if ((line > 270/*adc + 10*/) && (line < 278/*adc + 18*/))
	{  if (meld1 == 1){

		//ltemp = (line - 271/*adc + 11*/) * 27 - 64;
		//ntemp = (line - 271/*adc + 11*/) * 14 - 45;
		fonttemp = (line-271) * 59 - 32;
		SPSR = 0;											/* Switch to large font		*/
		 _delay_loop_1 (28);
	//MUXAN();
		//SPDR = nums[date[0] + ntemp];	Wait();		/* Month							*/
		//SPDR = nums[date[1] + ntemp];	Wait();		/* Month							*/
		//SPDR = nums['-' + ntemp];		Wait();		/* '-'							*/
		//SPDR = nums[date[2] + ntemp];	Wait();		/* Day							*/
		//SPDR = nums[date[3] + ntemp];	Wait();		/* Day							*/
		//SPDR = nums['-' + ntemp];		Wait();		/* '-'							*/
		//SPDR = nums[date[4] + ntemp];	Wait();		/* Year							*/
		//SPDR = nums[date[5] + ntemp];	Wait();		/* Year							*/
		//SPDR = ltrs['@' + ltemp];		Wait();		// ' '
		//SPDR = ltrs['@' + ltemp];		Wait();		/* ' '							*/
        //line1 = line-271;      
	 MUXAN ();
		//SPSR = 0;											/* Switch to large font		*/
//		SPDR = ltrs[lon[10]+ ltemp];		Wait();		
//		SPDR = nums[lon[0]+ ntemp];	Wait();		/* Degrees						*/
	
//		SPDR = nums[lon[1]+ ntemp];	Wait();		/* Degrees						*/
//		SPDR = nums[lon[2]+ ntemp];	Wait();		/* Degrees						*/
//		SPSR = 1;											/* Switch to small font		*/
//		SPDR = nums['/'+ ntemp];		Wait();		/* Print degrees symbol		*/
//		SPSR = 0;											/* Switch to large font		*/
//		SPDR = nums[lon[3]+ ntemp];	Wait();		/* Minutes						*/
//		SPDR = nums[lon[4]+ ntemp];	Wait();		/* Minutes						*/
//		SPSR = 1;											/* Switch to small font		*/
//		SPDR = nums[lon[5]+ ntemp];	Wait();		/* '.'							*/
//		SPSR = 0;											/* Switch to large font		*/
//		SPDR = nums[lon[6]+ ntemp];	Wait();		/* Decimal Minutes			*/
//		SPDR = nums[lon[7]+ ntemp];	Wait();		/* Decimal Minutes			*/
//		SPDR = nums[lon[8]+ ntemp];	Wait();		/* Decimal Minutes			*/
//		SPDR = nums[lon[9]+ ntemp];	Wait();		/* Decimal Minutes			*/
									
//      	SPSR =1;
		//SPDR = ltrs[' ' + ltemp][line-271];		Wait();		// ' '
//		SPDR = ltrs['G'+ ltemp];		Wait();		// 'G'
//		SPDR = nums[hg[0]+ ntemp];	Wait();		/* 	Höhe Geoid		*/
//		SPDR = nums[hg[1]+ ntemp];	Wait();		/* 			*/
//		SPDR = nums[hg[2]+ ntemp];	Wait();		/* 			*/
//		SPDR = nums[hg[3]+ ntemp];	Wait();		/* 			*/
//	  	SPDR = nums[hg[4]+ ntemp];	Wait();		/* 			*/
//	  	SPDR = nums[hg[5]+ ntemp];	Wait();		/* 			*/
//	    SPDR = ltrs['M'+ ltemp];		Wait();		/* 'M'*/
//	    SPDR = ltrs['@'+ ltemp];		Wait();		/* ' '*/
//		SPDR = ltrs['S'+ ltemp];		Wait();		/* 'S'*/
//	    SPDR = nums[sat[0]+ ntemp];	Wait();		/* 	Satelitenanzahl		*/
//	    SPDR = nums[sat[1]+ ntemp];	Wait();		/* 			*/
	 	//SPDR = ltrs['@' + ltemp];		Wait();		/* ' '*/ 
	    //SPDR = nums[qual[0] + ntemp];	Wait();		/* Qualität			*/
	 
	 
	 	SPDR = font[lon[10]+ fonttemp];		Wait();		
		SPDR = font[lon[0]+ fonttemp];	Wait();		/* Degrees						*/
	
		SPDR = font[lon[1]+ fonttemp];	Wait();		/* Degrees						*/
		SPDR = font[lon[2]+ fonttemp];	Wait();		/* Degrees						*/
		SPSR = 1;											/* Switch to small font		*/
		SPDR = font['/'+ fonttemp];		Wait();		/* Print degrees symbol		*/
		SPSR = 0;											/* Switch to large font		*/
		SPDR = font[lon[3]+ fonttemp];	Wait();		/* Minutes						*/
		SPDR = font[lon[4]+ fonttemp];	Wait();		/* Minutes						*/
		SPSR = 1;											/* Switch to small font		*/
		SPDR = font[lon[5]+ fonttemp];	Wait();		/* '.'							*/
		SPSR = 0;											/* Switch to large font		*/
		SPDR = font[lon[6]+ fonttemp];	Wait();		/* Decimal Minutes			*/
		SPDR = font[lon[7]+ fonttemp];	Wait();		/* Decimal Minutes			*/
		SPDR = font[lon[8]+ fonttemp];	Wait();		/* Decimal Minutes			*/
		SPDR = font[lon[9]+ fonttemp];	Wait();		/* Decimal Minutes			*/
									
      	SPSR =1;
		//SPDR = ltrs[' ' + ltemp][line-271];		Wait();		// ' '
		SPDR = font['G'+ fonttemp];		Wait();		// 'G'
		SPDR = font[hg[0]+ fonttemp];	Wait();		/* 	Höhe Geoid		*/
		SPDR = font[hg[1]+ fonttemp];	Wait();		/* 			*/
		SPDR = font[hg[2]+ fonttemp];	Wait();		/* 			*/
		SPDR = font[hg[3]+ fonttemp];	Wait();		/* 			*/
	  	SPDR = font[hg[4]+ fonttemp];	Wait();		/* 			*/
	  	SPDR = font[hg[5]+ fonttemp];	Wait();		/* 			*/
	    SPDR = font['M'+ fonttemp];		Wait();		/* 'M'*/
	    SPDR = font['@'+ fonttemp];		Wait();		/* ' '*/
		SPDR = font['S'+ fonttemp];		Wait();		/* 'S'*/
	    SPDR = font[sat[0]+ fonttemp];	Wait();		/* 	Satelitenanzahl		*/
	    SPDR = font[sat[1]+ fonttemp];	Wait();		/* 			*/
	 
	 
	 
	  MUXAUS();  
	//DDRD  = 0x00;							/* Data direction register for port D	MUX aus	*/
	}}

if ((line > adc/*+20*280*/) && (line < (adc+8/*288*/)))   //220   228   Zeile soll mit Poti zu verschieben sein
	{
	//ltemp = (line - (adc+1/*281*/)) * 27 - 64;       //221
	//ntemp = (line - (adc+1/*281*/)) * 14 - 45;       //221
    fonttemp = (line-(adc + 1)) * 59 - 32;
	// MUXAN();
		//SPSR = 1;											/* Switch to small font		*/
	//DDRD  = 0x80;	
	//Warten();						/* Data direction register for port D	MUX an	*/
		//SPDR = ltrs['S' + ltemp];		Wait();
		//SPDR = nums[':' + ntemp];		Wait();
		//SPDR = nums[spd[0] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[spd[1] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[spd[3] + ntemp];	Wait();		/* Minutes						*/

		//SPDR = ltrs['@' + ltemp];		Wait();
		//SPDR = ltrs['H' + ltemp];		Wait();
		//SPDR = nums[':' + ntemp];		Wait();
		//SPDR = nums[hdg[0] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[hdg[1] + ntemp];	Wait();		/* Hours							*/
		//SPDR = nums[hdg[3] + ntemp];	Wait();		/* Minutes						*/

		//SPSR = 1;											/* Switch to small font		*/
		//SPDR = ltrs['@' + ltemp];		Wait();
		//SPDR = ltrs['A' + ltemp];		Wait();
		//SPDR = nums[':' + ntemp];		Wait();
		SPSR = groklei;										/* Switch to large font		*/
		_delay_loop_1 (20);
		//adc++;//SPDR = ltrs['@' + ltemp];		Wait();
	    //line1 = line - (++adc);
	MUXAN ();
		
/*		SPDR = nums[date[0]+ ntemp];     Wait();	/* Year							*/
//		SPDR = nums[date[1]+ ntemp]; 	Wait();		/* Year							*/
			
		//SPDR = nums['-' + ntemp];		Wait();		/* '-'							*/
//		SPDR = nums[date[2]+ ntemp]; 	Wait();		/* Month							*/
//		SPDR = nums[date[3]+ ntemp]; 	Wait();		/* Month							*/
//		SPDR = nums[date[4]+ ntemp]; 	Wait();		/* Day							*/
//		SPDR = nums[date[5]+ ntemp]; 	Wait();		/* Day							*/
	    //SPSR = 1;
//		SPDR = nums[gpssta[0]+ ntemp]; 	Wait();	//GPS-Status
		//SPSR = groklei;
		//SPDR = ltrs['@' + ltemp];		Wait();		/* ' '							*/
//		SPDR = nums[time[0]+ ntemp]; 	Wait();		/* Hours							*/
//		SPDR = nums[time[1]+ ntemp]; 	Wait();		/* Hours							*/
//		SPDR = nums[time[2]+ ntemp]; 	Wait();		/* Minutes						*/
//		SPDR = nums[time[3]+ ntemp]; 	Wait();		/* Minutes						*/
//		SPDR = nums[time[4] + ntemp];	Wait();		/* Sekunde					*/
//		SPDR = nums[time[5] + ntemp];	Wait();		/* Sekunde						*/
//		SPDR = sta; /*trs[pps[sta]+ ltemp]; */		Wait();     //PPS-Anzeige
//		SPDR = nums[s[2]+ ntemp]; 		Wait();		/* msec				*/
//		SPDR = nums[s[3] + ntemp];		Wait();		/* msec				*/
//		SPDR = nums[s[4]+ ntemp]; 		Wait();		/* msec				*/
		//SPSR =1 ;
//		SPDR = ltrs[even[oesta]+ ltemp];   Wait();//Even
//		SPDR = ltrs[odd[oesta]+ ltemp];    Wait();//Odd	

			SPDR = font[date[0]+ fonttemp];     Wait();	/* Year							*/
		SPDR = font[date[1]+ fonttemp]; 	Wait();		/* Year							*/
			
		//SPDR = nums['-' + ntemp];		Wait();		/* '-'							*/
		SPDR = font[date[2]+ fonttemp]; 	Wait();		/* Month							*/
		SPDR = font[date[3]+ fonttemp]; 	Wait();		/* Month							*/
		SPDR = font[date[4]+ fonttemp]; 	Wait();		/* Day							*/
		SPDR = font[date[5]+ fonttemp]; 	Wait();		/* Day							*/
	    //SPSR = 1;
		SPDR = font[gpssta[0]+ fonttemp]; 	Wait();	//GPS-Status
		//SPSR = groklei;
		//SPDR = ltrs['@' + ltemp];		Wait();		/* ' '							*/
		SPDR = font[time[0]+ fonttemp]; 	Wait();		/* Hours							*/
		SPDR = font[time[1]+ fonttemp]; 	Wait();		/* Hours							*/
		SPDR = font[time[2]+ fonttemp]; 	Wait();		/* Minutes						*/
		SPDR = font[time[3]+ fonttemp]; 	Wait();		/* Minutes						*/
		SPDR = font[time[4] + fonttemp];	Wait();		/* Sekunde					*/
		SPDR = font[time[5] + fonttemp];	Wait();		/* Sekunde						*/
		SPDR = sta; /*trs[pps[sta]+ ltemp]; */		Wait();     //PPS-Anzeige
		SPDR = font[s[2]+ fonttemp]; 		Wait();		/* msec				*/
		SPDR = font[s[3] + fonttemp];		Wait();		/* msec				*/
		SPDR = font[s[4]+ fonttemp]; 		Wait();		/* msec				*/
		//SPSR =1 ;
		SPDR = font[even[oesta]+ fonttemp];   Wait();//Even
		SPDR = font[odd[oesta]+ fonttemp];    Wait();//Odd	
			/* '-'*/
	//DDRD  = 0x00;							/* Data direction register for port D MUX aus		*/
    MUXAUS();
	}

}	/* SIGNAL(SIG_OVERFLOW0) */


/*$FUNCTION$*******************************************************************/
//SIGNAL(SIG_INTERRUPT0)
/*******************************************************************************
* ABSTRACT:	Reset line timer at the left side of the screen.
*
* INPUT:		None
* OUTPUT:	None
* HSYNCH-Impuls*/
//{
//	line++;
//	TCNT0 = 175;							/*135 Set timeout period for the first line	*/
			
//	if (line > 260/*adc*/) TCNT0 = 200;		/* 195 175Set left edge of time/lat line			*/
//	if (line > 270/*adc + 10*/) TCNT0 = 210;		/*210 185Set left edge of date/lon line			*/
//	if (line > /*280*/adc + 20) TCNT0 = 250;        /*172 Set timeout period for fourth line  220		*/
//	TIFR |= 1<<TOV0;						/* Clear any potential interrupt				*/
//	TIMSK |= 1<<TOIE0;					/* Enable the timer0 interrupt				*/

//}	


/*$FUNCTION$*******************************************************************/
ISR(INT1_vect)
/*******************************************************************************
* ABSTRACT:	
*
* INPUT:		None
* OUTPUT:	None
*/
// PPS-Interrupt 
{
      
	 TCNT1 = 0;                        //ms-Zähler auf 0 setzen
     sta = 131;
	 SFIOR = (1<<PSR10); //Vorteiler auf Null setzen
	 //pps[0] = 'P';							// "P" an

                    time1[5]++ ; //empfangene Zeit um 1sec erhöhen
if (time1[5] > 57) {time1[5] = 48; time1[4]++;
	if (time1[4] > 53) {time1[4] = 48 ; time1[3] ++ ;
        if (time1[3] > 57) {time1[3] = 48 ; time1[2] ++ ; 
			if (time1[2] > 53) {time1[2] = 48 ; time1[1] ++ ;
				if (time1[0] == 50 && time1[1] > 52) {time1[1] = 48;} //hier nochmal ansehen
				if (time1[1] > 57) {time1[1] = 48 ; time1[0] ++ ;
					if (time1[0] > 50) {time1[0] = 48  ;}}}}}} 
	   
     
}	

//TimerCaptureInterrupt
ISR (TIMER1_CAPT_vect)
{

//VSynch 

 msec = ICR1; //Capturewert holen
	line = 0; //Zeilenzähler auf Null setzen

time[0] = time1[0]; //mit dem VSynch die Zeit übernehmen
time[1] = time1[1];
time[2] = time1[2];
time[3] = time1[3];
time[4] = time1[4];
time[5] = time1[5];

  msec = msec/18 ;//18000 ist 1s /18 ergibt wert in ms


			//Zahl in String zerlegen
			for(i = 0; i < laenge ; i++) {
      		s[laenge-i-1] = (msec % 10) + 48;
      		msec = msec / 10;
   								} 
//vsta1 = 0;

/*   msec = msec/18 ;//18000 ist 1s /18 ergibt wert in ms


	//Zahl in String zerlegen
	for(i = 0; i < laenge ; i++) {
      s[laenge-i-1] = (msec % 10) + 48;
      msec = msec / 10;
   }
*/
 if (! (PIND & (1<<PIND6)))   {oesta = 1;}//Even/Odd auslesen
		else {oesta = 0;}  
               sta = 255;// "P" aus 

//laufz = TCNT1; //Laufzeit der Routine ermitteln und korrigieren
//laufz = laufz - ICR1 ;
//line = line + laufz ;
}

//Timer1 Overflowinterrupt
//Wenn kein PPS, dann läuft hier die Uhr, wird alle Sekunde aufgerufen
ISR (TIMER1_OVF_vect)
{


                    time1[5]++ ; //empfangene Zeit um 1sec erhöhen
if (time1[5] > 57) {time1[5] = 48; time1[4]++;
	if (time1[4] > 53) {time1[4] = 48 ; time1[3] ++ ;
        if (time1[3] > 57) {time1[3] = 48 ; time1[2] ++ ; 
			if (time1[2] > 53) {time1[2] = 48 ; time1[1] ++ ;
				if (time1[0] == 50 && time1[1] > 52) {time1[1] = 48;} //hier nochmal ansehen
				if (time1[1] > 57) {time1[1] = 48 ; time1[0] ++ ;
					if (time1[0] > 50) {time1[0] = 48  ;}}}}}} 
}
