
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
#include "Definitions.h"

//image width and height
#define MAXLINES 290
#define MAXPIXELS 700
#define CAMADDR 0xc0

#define FCPU 20000000UL

///Net Constants
#define MAXSENDSIZE 355
#define UDPPACKETSZ 397
static uint8_t sendBuffer[UDPPACKETSZ];
uint8_t packetData[sizeof(ScannerCommand)];
#define DATA_POS		42

#define ETHERNET_RESET_PORT PORTB
#define ETHERNET_RESET_PORT_DD DDRB
#define ETHERNET_RESET_PIN PB0

//Mac and ip addresses as well as transmit/recieve port
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};
static uint8_t myip[4] = {10,0,0,2};

static uint8_t mypcip[4] = {10,0,0,5};
//static uint8_t mypcmac[6] = {0x00,0x14,0x51,0x00,0xf9,0xd8}; //Ryan's MAC
static uint8_t mypcmac[6] = {0x00,0x23,0x32,0xD9,0x48,0x36}; //Dale's MAC

//port to listen on for commands
#define listenPort 1201

//camera status
char cameraOn;

unsigned char lineBuffer[MAXPIXELS];

//mode for 
#define VSYNC		1
#define HREFUP		2
#define HREFDWN		3

///Mode Constants
#define IDLE		0
#define CAPTURE		1
#define SEND		2
#define STEP		3
#define SPIN		4
#define REPORT		5

unsigned char lineTimer = 0;

///Protypes
void turnOffCamera();
void sendLineBuffer( long pixNum);

volatile char syncState;
volatile char commandFlag = IDLE;

volatile long pixelNum;
volatile long lineNum;

///Stepper positions
#define STEP1	0
#define STEP2	1
#define STEP3	2
#define STEP4	3

char linearStepState = STEP1;
char rotationalStepState = STEP1;

//send out a test packet
char str[10] = "hello ryan";

//packet length
volatile char plen;

//listening communication status
char listening;

//detect a command
ISR(INT1_vect)
{ 	
	plen = enc28j60PacketReceive(UDPPACKETSZ, sendBuffer);

	//answer to ARP
	if(eth_type_is_arp_and_my_ip(sendBuffer, plen))
		make_arp_answer_from_request(sendBuffer, plen);

	//receive a UDP packet
    if (sendBuffer[IP_PROTO_P]==IP_PROTO_UDP_V&&sendBuffer[UDP_DST_PORT_H_P]==0x04&&sendBuffer[UDP_DST_PORT_L_P]==0xb1)
	 {
		
		//intepret the command
		memcpy(packetData, &sendBuffer[UDP_DATA_P], sizeof(ScannerCommand));
	
		switch(((ScannerCommand *)packetData)->command)
		{
			case LINEARSTEP:
				commandFlag = STEP;
				break;
			case ROTATIONSTEP:
				commandFlag = SPIN;
				break;
			case IMAGE:
				commandFlag = CAPTURE;
				break;
	}
	 }
	
	//reply to pings
	if(sendBuffer[IP_PROTO_P]==IP_PROTO_ICMP_V && sendBuffer[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V)
	{
                //a ping packet, let's send pong
              make_echo_reply_from_request(sendBuffer,plen);
			  
     }

	//clear the flag
	EIFR = 0x00;
}


//camera interrupt (VSYNC and HREF)
ISR(PCINT2_vect)

{ 	
		
	switch (syncState)
	{
		case VSYNC:
			
			syncState = HREFUP;
			
			//reset line number
			lineNum = 0;
			
			//turn on HREF ISR
			PCMSK2 = (1<<PCINT18);

			break;
			
		case HREFUP:
			
			pixelNum = 0;
			syncState = HREFDWN;

			//turn on pixel clock pos edge
			EIMSK = (1<<INT0);
			
			break;

		case HREFDWN:	
			
			//increment line number
			lineNum++;
			
			//turn off pixel counter			
			EIMSK = 0x00;

			if(lineNum >= MAXLINES)
			{
				commandFlag = REPORT;
				turnOffCamera();
				syncState = VSYNC;
				//turn off href ISR
				PCMSK2 = 0x00;
			}
	
			else
			{
				//the next state is HREFUP
				syncState = HREFUP;
				
				///Send some data
				commandFlag = SEND;
			}

			break;
			
	}

	//Clear the interrupt flag
	PCIFR = 0x00;

}

//pixel clock interrupt
ISR(INT0_vect)
{ 		
	lineBuffer[pixelNum] = PINA;
	pixelNum++;
	EIFR = 0x00;
}

/// --- Stepper Control ---

///Move the actuator
void moveActuator(char numSteps, bool CW)
{
	char t = 10;
	int i = 0;
	
	//clockwise motion
	if(!CW)
	{
		for(i=0; i<numSteps; i++)
		{
			PORTD = (PORTD&0x0f) | (1<<PINC4);
			_delay_ms(t);
			PORTD = (PORTD&0x0f) | (1<<PINC5);
			_delay_ms(t);
			PORTD = (PORTD&0x0f) | (1<<PINC7);
			_delay_ms(t);
			PORTD = (PORTD&0x0f) | (1<<PINC6);
			_delay_ms(t);
		}
	}
	else
	{
		for(i=0; i<numSteps; i++)
		{
		PORTD = (PORTD&0x0f) | (1<<PINC6);
		_delay_ms(t);
		PORTD = (PORTD&0x0f) | (1<<PINC7);
		_delay_ms(t);
		PORTD = (PORTD&0x0f) | (1<<PINC5);
		_delay_ms(t);
		PORTD = (PORTD&0x0f) | (1<<PINC4);
		_delay_ms(t);	
		}
	}

}

///Move the turntable
void spinTurnTable(char numSteps, bool CW)
{
	char t = 50;
	int i;

	for(i=0; i<numSteps; i++)
	{
		PORTC = (PORTC&0x0f) | (1<<PINC5);
		_delay_ms(t);
		PORTC = (PORTC&0x0f) | (1<<PINC7);
		_delay_ms(t);
		PORTC = (PORTC&0x0f) | (1<<PINC4);
		_delay_ms(t);
		PORTC = (PORTC&0x0f) | (1<<PINC6);
		_delay_ms(t);	
	}
	
	
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

//clear out any old commands sent while we werent listening
void clearRecBuf()
{
	char full =1;

	while(full)
	{
		full = enc28j60PacketReceive(UDPPACKETSZ, sendBuffer);
	}
}

//start listening to commands
void turnOnListen()
{
	turnOffCamera();

	//clear the recieve buffer to drive int high
	clearRecBuf();

	//turn on ethernet interrupt -- temp
	EIMSK = (1<<INT1);
	listening = 1;
}

void turnOffListen()
{
	//turn off ethernet interrupt 
	EIMSK = 0x00;
	listening = 0;
}

///its actually already on, we are just deciding to listen to it here
void turnOnCamera()
{
	//stop listrning to communcation 
	turnOffListen();

	//PINC3 interrupt set on
	PCICR = (1<<PCIE2);
	PCMSK2 = (1<<PCINT19);//VREF PINC3
	cameraOn = 1;
}

///Ignore the camera's chatter
void turnOffCamera()
{
	///Turn off camera interrupts
	PCICR = 0x00;//&= ~(1<<PCIE2);
	PCMSK2 = 0x00;//&= ~(1<<PCINT19) & ~(1<<PCINT18);

	//turn off pixel counter
	EIMSK = 0x00; //&= ~(1<<INT0);
	cameraOn = 0;

}

void initCam(){

	/* WHAT WE WANT TO DO
	 *
	 * 16-BIT one line RGB-output mode
	 */

    _delay_ms(10);
    cameraWrite(0x12, 0x80);  // initiate a soft reset to make sure that all the regsiters are at default values
	_delay_ms(1000);
    cameraWrite(0x11, 0x2f);  //slow the clock way down
	_delay_ms(10);
    cameraWrite(0x12, 0x2c);  //COMMON CTRL A: RGB output
	_delay_ms(10);
    cameraWrite(0x28, 0x80);  //one line RGB output 
	_delay_ms(10);

	//Exposure
	cameraWrite(0x25, 0xff);
	_delay_ms(10);
	cameraWrite(0x24, 0x00);
	_delay_ms(10);

	//Saturation
	cameraWrite(0x03, 0x04);
	_delay_ms(10);

	//contrast/ brightness
	cameraWrite(0x05, 0xff);
	_delay_ms(10);
	cameraWrite(0x06,0x00);
	_delay_ms(10);

}

/// --- Ethernet Communication ---

///Send everything in our line buffer to the PC
void sendLineBuffer(long pixNum)
{
	
	long bytesToSend = pixNum;
	long loc = 0;
	long dataSize;
	char seq = 0;
	///Loop trough until we've sent everything
	while (bytesToSend > 0)
	{
		dataSize = (bytesToSend >= MAXSENDSIZE) ? MAXSENDSIZE : bytesToSend;
		send_udp_packet(&sendBuffer[0], &lineBuffer[loc], dataSize, lineNum, seq);
		bytesToSend -=dataSize;
		loc+=dataSize;
		seq++;
		_delay_us(100);
	}
}

void Initialize()
{
	///Set PORTA as an input
	DDRA = 0x00;
	
	///Set External interrupt pins as inputs
	DDRD = DDRD & ~(1 << PINC2);
	DDRC = DDRC & ~(1 << PINC2) & ~(1 << PINC3);
	
	///Set Stepper output ports C[4:7], D[4:7]
	DDRD = DDRD | 0xf0;
	DDRC = DDRC | 0xf0;
	
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
	enc28j60PhyWrite(PHLCON,0x476);
	_delay_ms(20);
	
	//init the ethernet/ip layer:
	init_ip_arp_udp(mymac, myip, mypcip, mypcmac);
	_delay_ms(1000);


	//Enable Internal pull up resistors

	DDRC = DDRC & ~(1<< PORTC0);
	DDRC = DDRC & ~(1<< PORTC1);
	PORTC = PORTC | (1<< PORTC0);
	PORTC = PORTC | (1<< PORTC1);
	
	//turn off camera, don't listen to commands
	cameraOn = 0;
	listening = 0;
	
	//pixel clock triggers on positive edge and eth on neg edge
	EICRA = (1<<ISC01) | (1<<ISC00) | (1<<ISC11);

}

int main()
{
	Initialize();
	_delay_ms(30);
	initCam();
	_delay_ms(30);
	
	commandFlag = IDLE;
	syncState = VSYNC;
	ScannerCommand *command;

	PORTD = PORTD ^ (1<<PINC7);
	_delay_ms(500);
	PORTD = PORTD ^ (1<<PINC7);
	_delay_ms(500);
	PORTD = PORTD ^ (1<<PINC7);
	_delay_ms(1000);

	//crank up ISRs
	sei ();
	
	while(1)
	{
		command = (ScannerCommand *)packetData;
		
		switch (commandFlag)
		{
			//hang out until pc tells you to do something
			case IDLE:
				
				if(!listening && !cameraOn)
					turnOnListen();
				
				break;
				
			//take a picture
			case CAPTURE:
				///Turn on camera if necessary
				if (!cameraOn)
				{
					turnOnCamera();
				}
				commandFlag = IDLE;
				break;
			
			//send some data	
			case SEND:
				sendLineBuffer(pixelNum);
				commandFlag = IDLE;
				break;
				
			//step actuator
			case STEP:
				moveActuator(command->numSteps, command->direction);
				commandFlag = REPORT;
				break;
				
			//spin sample stage
			case SPIN:
				spinTurnTable(command->numSteps, command->direction);
				commandFlag = REPORT;
				break;
				
			//report to pc what youve done
			case REPORT:
				send_udp_packet(sendBuffer, packetData, sizeof(ScannerCommand), 0, 0);
				commandFlag = IDLE;
				break;

		}

		//clear all flags
		PCIFR = 0x00;
	}
}
