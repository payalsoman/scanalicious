//
//  NetController.m
//  Scanalicious
//
//  Created by Dale Taylor on 4/20/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import "NetController.h"
#import "Definitions.h"
#import <sys/types.h>   // random types
#import <netinet/in.h>  // for sockaddr_in
#import <sys/socket.h>  // for socket(), AF_INET
#import <arpa/inet.h>   // for inet_ntop
#import <errno.h>       // for errno
#import <string.h>      // for strerror
#import <stdlib.h>      // for EXIT_SUCCESS
#import <unistd.h>      // for close
#include <netdb.h>

static NetController *sharedController;
static NSString	*UDPRCV = @"UDPreceive";
static NSString	*UDPDAT = @"Data";

static unsigned char pixelData[xDim*yDim];
static unsigned char lineData[xDim+3];

@implementation NetController

+(NetController *)sharedInstance
{
	if (!sharedController)
	{
		sharedController = [[NetController alloc] init];
	}
	return sharedController;
}

-(id)init
{
	if (self = [super init])
	{
		NSLog(@"created network controller");
		targetAddress = 0;
		targetPort = 0;
		packetCount = 0;
		watchingSocket = NO;
		
		///Create a send socket
		int sock;
		int rc;
		struct sockaddr_in clientAddr;
		
		sendSockHandle = socket(AF_INET,SOCK_DGRAM,0);
		if(sendSockHandle < 0) 
		{
			NSLog(@"%s: cannot open socket \n");
		}
		
		
		clientAddr.sin_family = AF_INET;
		clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		clientAddr.sin_port = htons(1201);
		
		rc = bind(sendSockHandle, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
		if(rc<0) 
		{
			NSLog(@"%s: cannot bind port\n");
		}
		
	}
	return self;	
}

- (void)watchSocket:(in_addr_t)addr port:(int)port
{
	[NSThread detachNewThreadSelector:@selector(watchSocketThread:) toTarget:self withObject:[NSNumber numberWithInt:port]];
}

-(void)watchSocketThread:(NSNumber *)portNum
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	int port = [portNum intValue];
	///Set up the new connection
	sockHandle = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT_NUMBER);	
	
	int res = bind (sockHandle, (struct sockaddr *) &sa,sizeof(sa));
	
	char msg[MAX_UDP_DATAGRAM_SIZE];
	unsigned int clientLen;
	struct sockaddr_in clientAddr;
	watchingSocket = YES;
	
	while(watchingSocket) {
		
		/* init buffer */
		memset(msg,0x0,MAX_UDP_DATAGRAM_SIZE);

		/* receive message */
		clientLen = sizeof(clientAddr);
		res = recvfrom(sockHandle, msg, MAX_UDP_DATAGRAM_SIZE, 0, (struct sockaddr *) &clientAddr, &clientLen);
		
		if(res<0) {
			NSLog(@"cannot receive data");
			continue;
		}
		
		[self performSelectorOnMainThread:@selector(dataReceived:) withObject:[NSData dataWithBytes:msg length:(res * sizeof(u_char))] waitUntilDone:NO];
		
	}
	[pool release];
	
}

-(void)sendCommand:(char)cmd parameters:(NSDictionary *)param
{
	ScannerCommand *packetData = malloc(sizeof(ScannerCommand));
	
	packetData->command = cmd;
	
	switch (cmd) 
	{
		case LINEARSTEP:
		case ROTATIONSTEP:
			packetData->numSteps = [[param objectForKey:@"numSteps"] charValue];
			packetData->direction = [[param objectForKey:@"direction"] boolValue];
			break;
		case IMAGE:
			packetData->lineScan = [[param objectForKey:@"lineScan"] boolValue];
			break;
		default:
			NSLog(@"Invalid packet command");
			return;
			break;
	}
	
	[self writeData:[NSData dataWithBytes:packetData length:sizeof(ScannerCommand)]];
	
}

///Dirty, but it works
-(void)writeData:(NSData *)data
{	
	///Create destination address
	struct sockaddr_in remoteServAddr;
	
	struct hostent *h;
	char ip[20] = "10.0.0.2";
	
	h = gethostbyname(ip);
	if(h==NULL) 
	{
		NSLog(@"unknown host");
	}	
	
	remoteServAddr.sin_family = h->h_addrtype;
	memcpy((char *) &remoteServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	remoteServAddr.sin_port = htons(1201);

	int res;
	void *buffer = malloc([data length]*sizeof(u_char));
	[data getBytes:buffer length: [data length]*sizeof(u_char)];
	NSLog(@"buffer %s\n data description:  %@\nlength:  %d", buffer, [data description], [data length]*sizeof(u_char));
	res= sendto(sendSockHandle, buffer, [data length]*sizeof(u_char), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
		
	if(res<0) 
	{
		NSLog(@"cannot send data, res:  %d", res);
	}
	free(buffer);
}

-(void)dataReceived:(NSData *)data
{	
	int dataSize = [data length];
	char packetInc;
	unsigned int lineNo;
	long index;
	
	if ([data length] <  2 * sizeof(ScannerCommand)) ///A rigorous check for a command done packet
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:DONEWAIT object:self];
		return;
	}
	
	///inspect the first 3 bytes
	[data getBytes:&lineData[0] length:dataSize];
	lineNo = (unsigned char)lineData[0] * 256 + (unsigned char)lineData[1];
	lineNo--; //starts at line 1
	packetInc = lineData[2];
	index = lineNo * xDim + (packetInc * xSplit) -1;
	[data getBytes:&pixelData[index] range:NSMakeRange(3, dataSize-3)];
	packetCount++;

	if (lineNo == 0x0120) ///Some lines may be lost, so just take the image right before 
	{
		
		[[NSNotificationCenter defaultCenter] postNotificationName:FRAMERCV
															object:self
														  userInfo:[NSDictionary dictionaryWithObject:[NSData dataWithBytes:pixelData length:(xDim * yDim)]
																							   forKey:FRAMEDAT]];
		///Free the data buffer
		packetCount = 0;
		memset(pixelData, 0, (xDim *yDim));
	}
}

-(void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	CFRunLoopRemoveSource(CFRunLoopGetCurrent(), cfSource, kCFRunLoopDefaultMode);
	CFRelease(cfSource);
	CFRelease(cfSocket);
	[fileHandle release];
	[super dealloc];
}

@end
