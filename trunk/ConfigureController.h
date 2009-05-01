//
//  ConfigureController.h
//  Scanalicious
//
//  Created by Dale Taylor on 4/24/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define PARAMROTINC @"rotInc"
#define PARAMROTNUM @"rotNum"
#define PARAMLININC @"linInc"
#define PARAMLINNUM @"linNum"

@interface ConfigureController : NSWindowController 
{
	NSString *linearIncrementSize;
	NSString *rotationalIncrementSize;
	NSNumber *defaultDirection;
	IBOutlet NSTextField *rotIncSize;
	IBOutlet NSTextField *linIncSize;
	IBOutlet NSTextField *numRot;
	IBOutlet NSTextField *numLin;
}
+ (ConfigureController *)sharedController;
- (IBAction)linearCCW10:(id)sender;
- (IBAction)linearCW10:(id)sender;
- (IBAction)linearIncCCW:(id)sender;
- (IBAction)linearIncCW:(id)sender;
- (IBAction)rotCCW10:(id)sender;
- (IBAction)rotCW10:(id)sender;
- (IBAction)rotIncCCW:(id)sender;
- (IBAction)rotIncCW:(id)sender;
- (void)linearStep:(unsigned char)numSteps direction:(BOOL)CW;
- (void)rotationalIncrementInDirection:(BOOL)CW;
- (void)rotationalStep:(char)numSteps direction:(BOOL)CW;
- (void)linearIncrementInDirection:(BOOL)CW;
- (NSDictionary *)parametersForScan;

@end
