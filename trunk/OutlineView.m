//
//  OutlineView.m
//  Scanalicious
//
//  Created by Dale Taylor on 4/23/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import "OutlineView.h"


@implementation OutlineView

///remove the disclosure rect for group items
- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row
{
	TreeNode *node = [self itemAtRow:row];
	
	if ([node groupItem])
	{
		return NSZeroRect;
	}
	return [super frameOfOutlineCellAtRow:row];
}

@end
