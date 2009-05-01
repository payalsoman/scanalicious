//
//  NetController.h
//  Scanalicious
//
//  Created by Dale Taylor on 4/20/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define PORT_NUMBER 1200
#define DEVICE_ADDR 0x0a000002		//10.0.0.2
#define MAX_UDP_DATAGRAM_SIZE 500
#define	SENDTIMEOUT	200

#define xDim	352
#define yDim	288
#define xSplit	200

#define FRAMERCV	@"Framereceive"
#define FRAMEDAT	@"FrameData"
#define DONEWAIT    @"DoneWait"

@interface NetController : NSObject 
{
	NSFileHandle *fileHandle;
	CFSocketRef cfSocket;
	CFRunLoopSourceRef cfSource;
	in_addr_t targetAddress;
	int	targetPort;
	int packetCount;
	BOOL watchingSocket;
	int sockHandle;
	int sendSockHandle;
}

+(NetController *)sharedInstance;
-(void)sendCommand:(char)cmd parameters:(NSDictionary *)param;
//-(void)setUpSocket:(in_addr_t)addr port:(int)port;
-(void)watchSocket:(in_addr_t)addr port:(int)port;
//-(int)startListenting;
-(int)stopListening;
-(void)writeData:(NSData *)data;

@end
