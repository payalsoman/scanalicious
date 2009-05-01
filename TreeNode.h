//
//  TreeNode.h
//  Scanalicious
//
//  Created by Dale Taylor on 4/22/09.
//  Copyright 2009 Cornell University All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface TreeNode : NSObject 
{
	NSString *name;
	NSString *objectPath;
	NSMutableArray *childNodes;
	TreeNode *parent;
	BOOL groupItem;
}

- (id)initWithName:(NSString *)nameString objectPath:(NSString *)path parent:(TreeNode *)parentNode groupItem:(BOOL)group;
- (NSArray *)childNodes;
- (void)addChildNode:(TreeNode *)treeNode;
- (TreeNode *)parent;
- (NSString *)objectPath;
- (void)setObjectPath:(NSString *)path;
- (NSString *)name;
- (void)setName:(NSString *)nameString;
- (BOOL)groupItem;
- (void)removeChildNode:(TreeNode *)treeNode;

@end
