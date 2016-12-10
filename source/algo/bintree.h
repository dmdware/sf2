

#ifndef BINTREE_H
#define BINTREE_H

#include "../platform.h"
#include "../path/pathnode.h"
#include "hashkey.h"

struct BinTree
{

};

//#define TREEDATA	sizeof(PathNode)
#define TREEDATA PathNode

#define TREEKEY(x)		HASHKEY(x)

#if 0
struct TreeNode
{
	//front bit: if set go to index node if checkbit set, or else go to next index node
	//otherwise go to data
	unsigned short bin;
};
#endif

#define TREEBITS	21

//max children
//#define MAXTREE	((1<<14)-1)
#define MAXTREE	((1<<10)-1)
//#define MAXTREE		((1<<15)-1)

//first check bit 0, then 1, etc
//15 lower bits indicate next index to go to, if bit 16 is set, and current check bit is set to 1
//if bit 16 is 0, jump to index value in leaf table and return
//if bit 16 is set, but current check bit is not 0, next 16 bits give index to jump to (and if the 16th bit of those is not set, return leaf at index)
//so important to keep 2 16-bit numbers for each split/check bit
//easy to update and expand with additional values
//TREEBITS = 21, ie there are 21 bits of variation in keys, so a max split depth of 21, but not all of them are used, only 1%<

//extern unsigned char g_checkbit[TREEBITS];
//extern unsigned short g_treelast;
extern unsigned short g_tree[MAXTREE*3];
extern TREEDATA g_leaf[MAXTREE];

//set g_tree[n] = -1 to unset

unsigned short FindNode(HashKey key, unsigned char* depth);
void AddNode(unsigned short parent, unsigned short value1, unsigned short value2);
unsigned short MoveDown(unsigned short parent, unsigned int key, unsigned char* depth);

#endif