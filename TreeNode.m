//
//  TreeNode.m
//  Scanalicious
//
//  Created by Dale Taylor on 4/22/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import "TreeNode.h"


@implementation TreeNode
- (id)initWithName:(NSString *)nameString objectPath:(NSString *)path parent:(TreeNode *)parentNode groupItem:(BOOL)group
{
	if (self = [super init])
	{
		childNodes = [[NSMutableArray alloc] init];
		parent = [parentNode retain];
		objectPath = [path retain];
		name = [nameString retain];
		groupItem = group;
	}
	return self;
}

- (void)dealloc
{
	[childNodes release];
	[parent release];
	[objectPath release];
	[name release];
	[super dealloc];
}

- (NSArray *)childNodes
{
	return childNodes;
}

- (void)addChildNode:(TreeNode *)treeNode
{
	[childNodes addObject:treeNode];
}

- (void)removeChildNode:(TreeNode *)treeNode
{
	[childNodes removeObject:treeNode];
}

- (TreeNode *)parent
{
	return parent;
}

- (NSString *)objectPath
{
	return objectPath;
}

- (void)setObjectPath:(NSString *)path
{
	if (objectPath)
	{
		[objectPath release];
	}
	objectPath = [path retain];
}

- (NSString *)name
{
	return name;
}

- (void)setName:(NSString *)nameString
{
	if (name)
	{
		[name release];
	}
	name = [nameString retain];
}

-(BOOL)groupItem
{
	return groupItem;
}

@end
