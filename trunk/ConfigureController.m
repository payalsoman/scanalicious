//
//  ConfigureController.m
//  Scanalicious
//
//  Created by Dale Taylor on 4/24/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import "ConfigureController.h"
#import "NetController.h"
#import "Definitions.h"

#define CONFIGNIB	@"ConfigPanel"

static ConfigureController *sharedController = nil;

@implementation ConfigureController

+ (ConfigureController *)sharedController
{
	if (!sharedController) 
	{
		sharedController = [[self alloc] initWithWindowNibName:CONFIGNIB];
	}
	return sharedController;
}

- (id)initWithWindow:(NSWindow *)window
{
	if (self = [super initWithWindow:nil]) 
	{
		defaultDirection = [NSNumber numberWithBool:0];
	}
	return self;
}

- (void)dealloc
{
	[defaultDirection release];
	[super dealloc];
}

- (void)windowWillLoad
{
}

#pragma mark -
#pragma mark ScannerControl

- (IBAction)linearCCW10:(id)sender
{
	[self linearStep:100 direction:NO];
}

- (IBAction)linearCW10:(id)sender
{
	[self linearStep:100 direction:YES];
}

- (IBAction)linearIncCCW:(id)sender
{
	[self linearIncrementInDirection:NO];
}

- (IBAction)linearIncCW:(id)sender
{
	[self linearIncrementInDirection:YES];
}

- (IBAction)rotCCW10:(id)sender
{
	[self rotationalStep:10 direction:NO];
}

- (IBAction)rotCW10:(id)sender
{
	[self rotationalStep:10 direction:YES];
}

- (IBAction)rotIncCCW:(id)sender
{
	[self rotationalIncrementInDirection:NO];
}

- (IBAction)rotIncCW:(id)sender
{
	[self rotationalIncrementInDirection:YES];
}

- (void)linearIncrementInDirection:(BOOL)CW
{
	[[NetController sharedInstance] sendCommand: LINEARSTEP
									 parameters:[NSDictionary dictionaryWithObjectsAndKeys:
												 [NSNumber numberWithUnsignedChar:(unsigned char)[linIncSize intValue]], @"numSteps",
												 [NSNumber numberWithBool:CW], @"direction", nil]];
}

- (void)linearStep:(unsigned char)numSteps direction:(BOOL)CW
{
	[[NetController sharedInstance] sendCommand: LINEARSTEP
									 parameters:[NSDictionary dictionaryWithObjectsAndKeys:
												 [NSNumber numberWithUnsignedChar:numSteps], @"numSteps",
												 [NSNumber numberWithBool:CW], @"direction", nil]];
}

- (void)rotationalIncrementInDirection:(BOOL)CW
{
	[[NetController sharedInstance]  sendCommand: ROTATIONSTEP
									  parameters:[NSDictionary dictionaryWithObjectsAndKeys:
												  [NSNumber numberWithUnsignedChar:(unsigned char)[rotIncSize intValue]], @"numSteps",
												  [NSNumber numberWithBool:CW], @"direction", nil]];
}

- (void)rotationalStep:(char)numSteps direction:(BOOL)CW
{
	[[NetController sharedInstance]  sendCommand: ROTATIONSTEP
									  parameters:[NSDictionary dictionaryWithObjectsAndKeys:
												  [NSNumber numberWithUnsignedChar:numSteps], @"numSteps",
												  [NSNumber numberWithBool:CW], @"direction", nil]];
}

- (NSDictionary *)parametersForScan
{
	return [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:[rotIncSize intValue]], PARAMROTINC,
													[NSNumber numberWithInt:[numRot	intValue]], PARAMROTNUM,
													[NSNumber numberWithInt:[linIncSize intValue]], PARAMLININC,
													[NSNumber numberWithInt:[numLin intValue]], PARAMLINNUM, nil];
}


@end
