//
//  AppController.m
//  Scanalicious
//
//  Created by Dale Taylor on 4/19/09.
//  Copyright 2009 Cornell University All rights reserved.
//
#import "AppController.h"
#import <QuartzCore/CoreImage.h>
#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIImage.h>

#define RED 0
#define GREEN 1
#define BLUE 2
#define WIDTHKEY @"width"
#define HEIGHTKEY @"height"

#define REDTHRESH	254
#define GREENTHRESH	230
#define BLUETHRESH	230

static int scanIteration = 1;
static int scanNumber = 0;
static BOOL waitFlag = NO; 

@interface AppController(private)
- (void)createImageFromBuffer:(NSDictionary *)dict;
- (NSImage *)imageFromCGImageRef:(CGImageRef)image;
@end

@implementation AppController

- (id)init
{
    NSLog(@"init");
    if (self = [super init])
    {
        lastCamFrame = [[NSImage imageNamed:@"Picture 1.png"] retain];
        lastLine = [[NSImage imageNamed:@"Picture 2.png"] retain];
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(frameReceived:)  
													 name:FRAMERCV 
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(waitOver:)  
													 name:DONEWAIT 
												   object:nil];
		///Set up outiline view
		rootTreeNodes = [[NSMutableArray alloc] init];
		TreeNode *treeNode = [[TreeNode alloc] initWithName:@"Frames" objectPath:nil parent:nil groupItem:YES];
		[rootTreeNodes addObject:treeNode];
		[treeNode release];
		treeNode = [[TreeNode alloc] initWithName:@"Lines" objectPath:nil parent:nil groupItem:YES];
		[rootTreeNodes addObject:treeNode];
		[treeNode release];
		
		[[NetController sharedInstance] watchSocket:DEVICE_ADDR port:PORT_NUMBER];

		///Add some sample data
//		[self addCompletedMesh:@"mesh1.obj"];
//		[self addCompletedMesh:@"mesh2.obj"];
//		[self addCompletedMesh:@"mesh3.obj"];
//		[self addInProgressMesh:@"mesh4.obj"];
	}
    return self;
}

- (void)dealloc
{
	[lastCamFrame release];
	[lastLine release];
	[frame release];
	[red release];
	[green release];
	[blue release];
	[filtered release];
	[super dealloc];
}

- (void)awakeFromNib
{
    [toolbar setAllowsUserCustomization:FALSE];
	[sourceList expandItem:nil expandChildren:YES];
}

- (NSString *)nextScanName
{
	NSString *nameString = [NSString stringWithFormat:@"Scan_%d_%d.tiff", scanIteration, scanNumber];
	scanIteration++;
	return [nameString autorelease];
}

- (NSString *)nextFrameName
{
	NSString *nameString = [NSString stringWithFormat:@"Frame_%d_%d.tiff", scanIteration, scanNumber];
	scanIteration++;
	return [nameString autorelease];
}

- (IBAction)startScan:(id)sender
{
	///Disable some buttons
	
	///Change the Toolbar icon
	NSDictionary *options = [[ConfigureController sharedController] parametersForScan];
	
	///Start up a thread to run this bad boy
	[NSThread detachNewThreadSelector:@selector(scanThread:) toTarget:self withObject:options];
}

- (void)scanThread:(NSDictionary *)options
{
	NSLog(@"In scan thread");
	unsigned char linInc = [[options objectForKey:PARAMLININC] unsignedCharValue];
	int linNum = [[options objectForKey:PARAMLINNUM] intValue];
	unsigned char rotInc = [[options objectForKey:PARAMROTINC] unsignedCharValue];
	int rotNum = [[options objectForKey:PARAMROTNUM] intValue];
	
	int i,j;
	long stepPos;
	long rotPos;
	BOOL CW = YES;
	
	///Naming Stuff
	scanNumber++;
	
//	///Go through rotations
	for (i = 0; i < rotNum; i++)
	{
		NSLog(@"In scan thread rot: %d", i);

		///Go through increments
		for (j = 0; j < linNum; j++)
		{
			NSLog(@"In scan thread step: %d", j);

			///Take a picture
			waitFlag = YES;
			[self snapPhoto:nil];
			
			///Wait
			while (waitFlag) sleep(1);
			
			///Save images
			[self saveImageAndProcess];
			
			// -Move one increment
			waitFlag = YES;
			[[ConfigureController sharedController] linearStep:linInc direction:CW];
			while (waitFlag) sleep(1);
			
			// -update stepPos
			stepPos+=linInc;
			///Wait
		}
		waitFlag = YES;
		///-Spin turntable
		[[ConfigureController sharedController] rotationalStep:rotInc direction:0];

		///Wait
		while (waitFlag) sleep(1);
		
		///Switch CW (sweep direction)
		CW = !CW;
		rotPos+= rotInc;
	}
}

///The scanner if we are safe to proceed
- (void)waitOver:(NSNotification *)notification
{
	waitFlag = NO;
}


- (IBAction)openConfigurationPanel:(id)sender
{
	[[ConfigureController sharedController] showWindow:nil];
}

///A full frame has been received, handle it accordingly
- (void)frameReceived:(NSNotification *)notification
{
	NSLog(@"I have frame data");
	NSData *frameData = [[notification userInfo] objectForKey:FRAMEDAT];
	
	NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:frameData, FRAMEDAT, 
						  [NSNumber numberWithInt:xDim], WIDTHKEY,
						  [NSNumber numberWithInt:yDim], HEIGHTKEY, nil];
	
	[NSThread detachNewThreadSelector:@selector(createImageFromBuffer:) toTarget:self withObject:dict];
	///Insert call here to image processor
}

- (IBAction)setRed:(id)sender
{
	[self setLastCamFrame:red];
}

- (IBAction)setGreen:(id)sender
{
	[self setLastCamFrame:green];
}

- (IBAction)setBlue:(id)sender
{
	[self setLastCamFrame:blue];
}

- (IBAction)setReg:(id)sender
{
	[self setLastCamFrame:frame];
}

- (IBAction)setFiltered:(id)sender
{
	[self setLastCamFrame:filtered];
}

- (IBAction)snapPhoto:(id)sender
{
	NetController *net = [NetController sharedInstance];
	NSDictionary *info = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithBool:NO],@"lineScan", nil];
	[net sendCommand: IMAGE parameters:info];
	[info release];
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}

- (void)addCompletedFrame:(NSString *)name
{
	TreeNode *rootNode = [rootTreeNodes objectAtIndex:0];
	TreeNode *childNode = [[TreeNode alloc] initWithName:name objectPath:name parent:rootNode groupItem:NO];
	[rootNode addChildNode:childNode];
	[childNode release];
	[sourceList expandItem:nil expandChildren:YES];
}

- (void)addCompletedLine:(NSString *)name
{
	TreeNode *rootNode = [rootTreeNodes objectAtIndex:1];
	TreeNode *childNode = [[TreeNode alloc] initWithName:name objectPath:name parent:rootNode groupItem:NO];
	[rootNode addChildNode:childNode];
	[childNode release];
	[sourceList expandItem:nil expandChildren:YES];
}

#pragma mark -
#pragma mark KVC accessors
- (void)setLastLine:(NSImage *)image
{
	if (lastLine)
	{
		[lastLine release];
	}
	lastLine = [image retain];
}

- (NSImage *)lastLine
{
	return lastLine;
}

- (void)setLastCamFrame:(NSImage *)image
{
	if (lastCamFrame)
	{
		[lastCamFrame release];
	}
	lastCamFrame = [image retain];
}

- (NSImage *)lastCamFrame
{
	return lastCamFrame;
}

#pragma mark -
#pragma mark OutlineView DataSource
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if (item == nil)
	{
		return 2;
	}
	else if ([item childNodes])
	{
		NSLog(@"child nodes");
		return [[item childNodes] count];
	}
    return 0;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	if (item == nil)
	{
		return YES;
	}
	else if ([item childNodes])
	{
		return ([[item childNodes] count] > 0);
	}
	return NO;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
    return (item == nil) ? [rootTreeNodes objectAtIndex:index] : [[item childNodes] objectAtIndex:index];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	return (item == nil) ? nil : [item name];
}

#pragma mark OutlineView Delegate

- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item
{
	return (item == nil) ? YES : [item groupItem];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
	return (item == nil) ? YES : ![item groupItem];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowCellExpansionForTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	return (item == nil) ? NO : ![item groupItem];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
	return (item == nil) ? NO : ![item groupItem];
}

#pragma mark Image Processing Nonsense That Belongs Somewhere Else

- (NSImage *)bayerReconstruct:(CGImageRef)input
{
	CIImage *inputImage = [[CIImage alloc] initWithCGImage:input];
	
	///Thank you http://fdiv.net/2006/09/19/61-hidden-patches/ for the private filter name
	CIFilter *reconstruct = [CIFilter filterWithName:@"CIBayerReconstruction"];
	
	[reconstruct setDefaults];
	[reconstruct setValue: inputImage forKey: @"inputImage"];
	CIImage *result = [reconstruct valueForKey: @"outputImage"];
	NSImage *finalImage = [self imageFromCIImage:result];
	return finalImage;
}

- (NSImage *) imageFromCIImage:(CIImage *)ciImage
{
	NSImage *image = [[[NSImage alloc] initWithSize:NSMakeSize([ciImage extent].size.width, [ciImage extent].size.height)] autorelease];
	
	[image addRepresentation:[NSCIImageRep imageRepWithCIImage:ciImage]];
	
	return image;
}

- (void)createImageFromBuffer:(NSDictionary *)dict
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CGContextRef    context = NULL;
    CGColorSpaceRef colorSpace;
    void *          bitmapData;
    int             bitmapByteCount;
    int             bitmapBytesPerRow;
	
	int w = [[dict objectForKey:WIDTHKEY] intValue];
	int h = [[dict objectForKey:HEIGHTKEY] intValue];
	NSData *data = [dict objectForKey:FRAMEDAT];
	
	int dataLen = [data length];
	unsigned char *buffer = malloc(dataLen*sizeof(u_char));
	[data getBytes:buffer length:dataLen];
	
	/// 3 bytes (RGB)
    bitmapBytesPerRow   = (w * 4);
    bitmapByteCount     = (bitmapBytesPerRow * h);
	
	unsigned char *rgbArray = malloc(bitmapByteCount);
	unsigned char *rgbArrayR = malloc(bitmapByteCount);
	unsigned char *rgbArrayG = malloc(bitmapByteCount);
	unsigned char *rgbArrayB = malloc(bitmapByteCount);
	unsigned char *threshArray = malloc(bitmapByteCount/4);
	
	memset(rgbArray, 0, bitmapByteCount);
	memset(rgbArrayR, 0, bitmapByteCount);
	memset(rgbArrayG, 0, bitmapByteCount);
	memset(rgbArrayB, 0, bitmapByteCount);
	memset(threshArray, 0, bitmapByteCount/4);

	
	int i,j, col;	
	long loc;
	BOOL BG = NO;
	BOOL G = NO;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			loc = (j*xDim) + i;
			BG = (j % 2 == 0);
			G = (BG) ? !(i % 2 == 0) : (i % 2 == 0);
			loc = (j * xDim) + i;
			col = G ? GREEN : (BG ? BLUE : RED);
			rgbArray[loc*4 + col] = buffer[loc];
			switch(col)
			{
				case RED:
					rgbArrayR[loc*4+col] = buffer[loc];
					if (buffer[loc] >= REDTHRESH)
					{
						threshArray[loc] = buffer[loc];
					}
					break;
				case GREEN:
					rgbArrayG[loc*4+col] = buffer[loc];
					if (buffer[loc] >= GREENTHRESH)
					{
						threshArray[loc] = buffer[loc];
					}
					break;
				case BLUE:
					rgbArrayB[loc*4+col] = buffer[loc];
					if (buffer[loc] >= GREENTHRESH)
					{
						threshArray[loc] = buffer[loc];
					}
					break;
			}
			
		}
	}
	
	
	///Make images for all separate color channels
    colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    context = CGBitmapContextCreate (rgbArray,
									 w,
									 h,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaNoneSkipLast);
	
	CGImageRef imageRef = CGBitmapContextCreateImage(context);
	frame = [[self imageFromCGImageRef:imageRef] retain];
	filtered = [[self bayerReconstruct:imageRef] retain];
	
	CFRelease(context);
	
	context = CGBitmapContextCreate (rgbArrayR,
									 w,
									 h,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaNoneSkipLast);
	
	imageRef = CGBitmapContextCreateImage(context);
	red = [[self imageFromCGImageRef:imageRef] retain];
	
	CFRelease(context);
	
	context = CGBitmapContextCreate (rgbArrayG,
									 w,
									 h,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaNoneSkipLast);
	
	imageRef = CGBitmapContextCreateImage(context);
	green = [[self imageFromCGImageRef:imageRef] retain];
	
	CFRelease(context);
	
	context = CGBitmapContextCreate (rgbArrayB,
									 w,
									 h,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaNoneSkipLast);
	
	imageRef = CGBitmapContextCreateImage(context);
	blue = [[self imageFromCGImageRef:imageRef] retain];
	
    CGColorSpaceRelease( colorSpace );
	
	///Greyscale for line
	colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
	context = CGBitmapContextCreate (threshArray,
									 w,
									 h,
									 8,      // bits per component
									 w,
									 colorSpace,
									 kCGImageAlphaNone);
	
	imageRef = CGBitmapContextCreateImage(context);
	threshold = [[self imageFromCGImageRef:imageRef] retain];
	
	CFRelease(context);
	CGColorSpaceRelease( colorSpace );
	
	[self setLastLine:threshold];
	[self setLastCamFrame:frame];
	
	free(buffer);
	free(rgbArray);
	free(rgbArrayR);
	free(rgbArrayG);
	free(rgbArrayB);
	free(threshArray);

	[pool release];
	
}

- (NSImage *)imageFromCGImageRef:(CGImageRef)image
{
	
	NSRect imageRect = NSMakeRect(0.0, 0.0, 0.0, 0.0);
	CGContextRef imageContext = nil;
	NSImage *newImage = nil;
	
	//Get dimensions
	imageRect.size.height = CGImageGetHeight(image);
	imageRect.size.width = CGImageGetWidth(image);
	
	newImage = [[[NSImage alloc] initWithSize:imageRect.size]autorelease];
	[newImage lockFocus];
	imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
	CGContextDrawImage(imageContext, *(CGRect*)&imageRect, image);
	[newImage unlockFocus];
	
	return newImage;
}

- (void)saveImageAndProcess
{
	NSString *frameName = [self nextFrameName];
	NSString *lineName = [self nextScanName];
	
	NSString *framePath = [NSString stringWithFormat:@"/Users/dat38/Desktop/Scans/%@", frameName];
	NSString *linePath = [NSString stringWithFormat:@"~/Users/dat38/Scans/%@", lineName];
	
	[[frame TIFFRepresentation] writeToFile:framePath atomically:YES];	
	[self addCompletedFrame:frameName];
	[[threshold TIFFRepresentation] writeToFile:framePath atomically:YES];	
	[self addCompletedLine:lineName];

}

@end



