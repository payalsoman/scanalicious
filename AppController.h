//
//  AppController.h
//  Scanalicious
//
//  Created by Dale Taylor on 4/19/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NetController.h"
#import "TreeNode.h"
#import "OutlineView.h"
#import	"Definitions.h"
#import	"ConfigureController.h"

@interface AppController : NSObject 
{
	BOOL listenOn;
    NSImage *lastCamFrame;
    NSImage *lastLine;
	NSMutableArray *rootTreeNodes;
	IBOutlet NSImageView *camView;
    IBOutlet NSImageView *lineView;
    IBOutlet NSToolbar *toolbar;
	IBOutlet OutlineView *sourceList;
	
	NSImage *red;
	NSImage *green;
	NSImage *blue;
	NSImage *frame;
	NSImage *filtered;
	NSImage *threshold;

}

- (IBAction)openConfigurationPanel:(id)sender;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
- (void)addCompletedFrame:(NSString *)name;
- (void)addCompletedLine:(NSString *)name;
- (void)setLastLine:(NSImage *)image;
- (NSImage *)lastLine;
- (void)setLastCamFrame:(NSImage *)image;
- (NSImage *)lastCamFrame;
- (IBAction)startScan:(id)sender;
- (IBAction)setRed:(id)sender;
- (IBAction)setGreen:(id)sender;
- (IBAction)setReg:(id)sender;
- (IBAction)setBlue:(id)sender;
- (IBAction)setFiltered:(id)sender;
- (IBAction)snapPhoto:(id)sender;
- (void)saveImageAndProcess;
- (NSImage *) imageFromCIImage:(CIImage *)ciImage;

@end
