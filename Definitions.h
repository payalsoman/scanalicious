/*
 *  Definitions.h
 *  Cocoa OpenGL
 *
 *  Created by Dale Taylor on 4/23/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#define	LINEARSTEP		0
#define	ROTATIONSTEP	1
#define	IMAGE			2

///This could be more efficient, but its so small that it shouldnt really be an issue
typedef struct
{
    unsigned char command;
	///For LINEARSTEP and ROTATIONSTEP
	unsigned char numSteps;
	BOOL direction; ///<(CCW = 0, CW = 1)
	///For IMAGE
	BOOL lineScan;  ///<(normal Image = 0, line scan = 1)
	
} ScannerCommand;


