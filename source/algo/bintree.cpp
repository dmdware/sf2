

#include "bintree.h"


unsigned short g_tree[MAXTREE*3];
TREEDATA g_leaf[MAXTREE];

//set g_tree[n] = -1 to unset

#if 0
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

/* TODO figure out */

//first check bit 0, then 1, etc
//15 lower bits indicate next index to go to, if bit 16 is set, and current check bit is set to 1
//if bit 16 is 0, jump to index value in leaf table and return
//if bit 16 is set, but current check bit is not 0, next 16 bits give index to jump to (and if the 16th bit of those is not set, return leaf at index)
//so important to keep 2 16-bit numbers for each split/check bit
//easy to update and expand with additional values
//TREEBITS = 21, ie there are 21 bits of variation in keys, so a max split depth of 21, but not all of them are used, only 1%<
#endif

TREEDATA* GetLeaf(HashKey key)
{
	unsigned char d;
	unsigned short i = FindNode(key, &d);

	if(i == -1)
		return NULL;

	return &g_leaf[i];
}

unsigned short FindNode(HashKey key, unsigned char* depth)
{
	unsigned short i = 0;
	unsigned char d = 0;

	while(ectrue)
	{
		unsigned short t = g_tree[i];
		unsigned short t2 = g_tree[i+1];
		unsigned short j;

		if(t == -1)
			return -1;

		if(t & (1<<15))
		{
			if((*(HASHINT*)&key) & (1<<d))
			{
				j = t & ~(1<<15);
				goto jump;
			}
			else
			{
				if(t2 == -1)
					return -1;

				j = t2 & ~(1<<15);

				if(t2 & (1<<15))
					goto jump;
				else
					goto give;
			}
		}
		else
		{
			j = t & ~(1<<15);
			goto give;
		}

jump:

		++d;
		i = j;
		continue;

give:
		
		*depth = d;
		return j;
	}
}

unsigned short AddNode(HashKey key)
{

}

unsigned short MoveDown(unsigned short parent, HashKey key, unsigned char* depth)
{
}