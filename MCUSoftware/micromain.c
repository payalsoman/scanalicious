/*
 *  micromain.c
 *  Cocoa OpenGL
 *
 *  Created by Dale Taylor on 4/20/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <inttypes.h>
#include "i2cmaster.h"
#include "ip_arp_udp.h"
#include "enc28j60.h"
#include "net.h"

///Mode Constants
#define IDLE		0
#define CAPTURE		1
#define SENDING		2
#define STEPPING	3
#define	MODEUPDATEINC	100

volatile unsigned char nextMode;

#define FCPU 20000000UL

///Net Constants
#define MAXSENDSIZE	240
#define UDPPACKETSZ	282
unsigned char sendBuffer[UDPPACKETSZ+1];

#define ETHERNET_RESET_PORT		PORTB
#define ETHERNET_RESET_PORT_DD	DDRB
#define ETHERNET_RESET_PIN		PB0

///Camera Constants
#define VSYNCUP		1
#define HREFUP		2
#define HREFDWN		3
#define PCKKOFF		0
#define PCLKON		1
#define MAXNUMLINES	292
#define MAXPIXELS	720  //(just to be safe)
#define CAMADDR		0xC0

//Mac and ip addresses as well as transmit/recieve port
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};
static uint8_t myip[4] = {10,0,0,24};

static uint8_t mypcip[4] = {10,0,0,3};
static uint8_t mypcmac[6] = {0x00,0x14,0x51,0x00,0xf9,0xd8};

static uint16_t myport =1200; // listen port for udp

volatile long lineNum;
volatile long pixNum;
volatile char syncState;
volatile char pxState;
char cameraOn;
unsigned char lineBuffer[MAXPIXELS];

///Protypes
void turnOffCamera();

//VSYNC and HREF have to share an interrupt, use a state machine to keep track of what is going on PINC2, PINC3
ISR(PCINT2_vect)
{ 
	switch (syncState)
	{
		case VSYNCUP:
			///Starting a new frame, reset the line number
			lineNum = 0;
			///Enable HSync ISR
			PCMSK2=(1<<PCINT18);
			syncState = HREFUP;
			break;
		case HREFUP:
			///Turn on pixel clock ISR
			EIMSK=(1<<INT0);
			pixNum = 0;
			lineNum++;
			syncState = HREFDWN;
			break;
		case HREFDWN:
			///Turn off the pixel clock ISR
			EIMSK=0x00;
			///"Turn off" the camera if the frame is done
			if ( lineNum < MAXNUMLINES)
			{
				syncState = HREFUP;
			}
			else
			{
				turnOffCamera();
			}
			
			///Send some data
			nextMode = SENDING; 
			break;
			
	}

	//Clear the flag
	PCIFR = 0x00;
}

///Pxclk (PIND2)
ISR(INT0_vect)
{ 
    //capture inputs from PORTA
	lineBuffer[pixNum] = PINA;
	pixNum++;
	//Clear the flag
	EIFR = 0x00;
}

/// --- Camera Code ---

/// I2C read from camera
unsigned char cameraRead(unsigned char regNum) 
{
	unsigned char write;
	unsigned char read;
	
	///Check these if they start to cause trouble
	write = i2c_start(CAMADDR+I2C_WRITE);   
	write = i2c_write(regNum);          
	i2c_stop();                              
	
	read = i2c_start(CAMADDR+I2C_READ); ; 
	read = i2c_readNak();
	i2c_stop();                              
	return read;
}

/// I2C Write to camera
unsigned char cameraWrite(unsigned char regNum, unsigned char data) {
	unsigned char write;
	
	///Check these if they start to cause trouble
	write = i2c_start(CAMADDR+I2C_WRITE);   
	write = i2c_write(regNum);    
	write = i2c_write(data);          
	i2c_stop();   
	return 0;
}

///Set up parameters good for normal images
void setNormalImageRegs()
{
}

///Set up parameters good for detecting the laser
void setLineScanningRegs()
{
}

///its actually already on, we are just deciding to listen to it here
void turnOnCamera()
{
	PCMSK2=(1<<PCINT19);
	syncState = VSYNCUP;
	cameraOn = 1;
}

///Ignore the camera's chatter
void turnOffCamera()
{
	///Turn off camera interrupts
	PCMSK2 = 0x00;
	EIMSK = 0x00;
	cameraOn = 0;
}

///Set up basic parameters for the camera
void setUpCamera()
{
	_delay_ms(10);
	cameraWrite(0x12, 0x80);  //soft reset
	_delay_ms(2);
	cameraWrite(0x12, 0x2C);  //AGC enable, set RGB mode, with AWB enabled
	_delay_ms(2);
	cameraWrite(0x28, 0x80);  //one line RGB output  IS THIS RIGHT??
	_delay_ms(2);
	cameraWrite(0x13, 0x11);  //tri-state the Y/UV lines, select 8 bit format
	_delay_ms(2);
	cameraWrite(0x11, 0x20);  //clock div by 32
	_delay_ms(2);
	//		///TODO:  figure out what gain/white balance levels we need
}

/// --- Ethernet Communication ---

///Send everything in our line buffer to the PC
void sendLineBuffer()
{
	long bytesToSend = pixNum;
	long loc = 0;
	unsigned char dataSize;
	///Loop trough until we've sent everything
	while (bytesToSend > 0)
	{
		dataSize = (bytesToSend >= MAXSENDSIZE) ? MAXSENDSIZE : bytesToSend;
		send_udp_packet(&sendBuffer[0], &lineBuffer[loc], dataSize);
		bytesToSend -=dataSize;
		loc+=dataSize;
	}
}

void Initialize()
{
	lineNum = 0;
	pixNum = 0;
	
	///Set PORTA as an input
	DDRA = 0x00;
	
	///Set External interrupt pins as inputs
	DDRD = DDRD | (1 << PIND2);
	DDRC = DDRC | (1 << PINC2) | (1 << PINC3);
	
	///Set Stepper output ports C[4:7], D[4:7]
	DDRD = DDRD | (0xf << PORTD4);
	DDRC = DDRC | (0xf << PORTC4);
	
	///Set up i2c communcation
	i2c_init();
	
	//Setup ethernet communication
	
	/* enable PB0, reset as output */
	ETHERNET_RESET_PORT_DD |= (1<<ETHERNET_RESET_PIN);
	
	/* set output to gnd, reset the ethernet chip */
	ETHERNET_RESET_PORT &= ~(1<<ETHERNET_RESET_PIN);
	_delay_ms(10);
	/* set output to Vcc, reset inactive */
	ETHERNET_RESET_PORT |= (1<<ETHERNET_RESET_PIN);
	_delay_ms(300);
	
	/*initialize enc28j60*/
	enc28j60Init(mymac);
	_delay_ms(20);
	
	/* Magjack leds configuration, see enc28j60 datasheet, page 11 */
	// LEDB=yellow LEDA=green
	//
	// 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	// enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	enc28j60PhyWrite(PHLCON,0x476);
	_delay_ms(20);
	
	//init the ethernet/ip layer:
	init_ip_arp_udp(mymac,myip,mypcip,mypcmac, sendBuffer);
	_delay_ms(100);

	//send out a test packet
	char str[10] = "hello ryan";
	send_udp_packet(sendBuffer, str, 10);

	
	//Enable Internal pull up resistors
	DDRC = DDRC & ~(1<< PORTC0);
	DDRC = DDRC & ~(1<< PORTC1);
	PORTC = PORTC | (1<< PORTC0);
	PORTC = PORTC | (1<< PORTC1);

	cameraOn = 0;
	
}

int main()
{
	Initialize();
	_delay_ms(100);
	setUpCamera();
	_delay_ms(100);

//Set current Mode (temporary)

	nextMode = CAPTURE;	

	sei ();
	while(1)
	{
		switch (nextMode)
		{
			case IDLE:
				///Don't do anything, just chill
				break;
			case CAPTURE:
				///Turn on camera if necessary
				if (cameraOn == 0)
				{
					turnOnCamera();
				}
				break;
			case SENDING:
				//SEND DATA CALL HERE
				sendLineBuffer();
				break;
			case STEPPING:
				//Enable stepper motor timer here
				break;
		}
	}
}
