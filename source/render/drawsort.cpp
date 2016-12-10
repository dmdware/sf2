












#include "drawsort.h"
#include "drawqueue.h"
#include "shader.h"
#include "tile.h"
#include "sprite.h"
#include "../texture.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/isomath.h"
#include "../sim/map.h"
#include "../gui/gui.h"
#include "tile.h"
#include "../save/savemap.h"
#include "../sim/unit.h"
#include "../debug.h"
#include "heightmap.h"
#include "../sim/building.h"
#include "../path/collidertile.h"

std::list<Dl> g_drawlist;
std::list<Dl*> g_subdrawq;

/*
TODO
speed boost to remove ability to have mv in same collider tile,
only allowing warcraft ii-like tile occupation. speed boost for
possible embedded platforms and simplifies hard cases that have
no solution in stuck pathing.
*/

/*
TODO
Similar to "iterative deepening" from AI Game Wisdom article by
Timothy Cain.
Only do x steps of pathing each frame for unit, saving
"to reset" pathnodes as closed nodes, and slow list,
setting up the field each time. The advantage is that
it processes paths like in real life, depending on the
complexity giving it more time. This should result in
a constant, controllable processing load.
Problem is it will take a int time to learn a
path is not possible, if comparing job opps or store
opps.
Maybe combine with street level hierarchical pathing.
Maybe start moving like flow field before path found,
so that if somebody moves in the way in the mean while,
you'll already be in front.
*/

/*
TODO
Instead of pathing to a collision with a target building,
set a goal area of pathnodes immediately around the building,
and declare path arrived at these nodes. Advantages are
simpler path success check. And immediate discarding of
blocked nodes without putting them on show list,
removing need to heapify twice. Also makes it more like
any standard A* out there, so it won't break when
somebody tries to make it avoid colliding with target
building nodes.
*/

/*
TODO
Reimplement street hierarchical pathfinding, this time
with check for blockage that routes around on the street
(higher) level.

TODO
Also, reimplement multithreaded pathfinding.
Might work this time, if done properly.
*/

/*
TODO
Also from iterative deepening article,
try discarding nodes that are X or higher cost
(CostFromStart + CostToGoal), so that they
don't have to be sorted or tried. This
will work if there is a path with lower than
X cost nodes.
Or, stop the search altogether and return
failure as soon as a node is popped
that has TotalCost exceeding X, because
any future node is guaranteed to have
worse cost.
*/

/*
TODO
What if x,y of pathnode is stored in
the struct? Like in "lighting fast A*"
AI Wisdom article. Then instead of
having an array of all pathnodes,
only a list of show or closed
nodes is needed. Checking if a node
is closed or getting it on demand
from the list requires iterating
through all the items, maybe make
a pointer array for all, and reset
them all or set them up as needed?
For best of both worlds.
That way also if the linked list
is a pointer, all that's needed
to do to store the path search for
later continuing, is to assign the
pointer to the list. No memory
copying.
Also, use memory pooling on the
show/closed/whatever list nodes.
This makes iterative frame-by-
frame pathfinding impossible though,
since path search progresss from
prev frame must be memory
copied piece by piece.
Still possible, but not as
convenient.
*/

/*
TODO
Use hash table function for show/closed
list pathnodes lookup instead of array
of pointers. Array of pointers will take
at least 6 MB for 64x64 tile map.
From "lightning fast A*" article:
AStarStorage::AddNodeToClosedList(Node* inNode)
{
 int hash = this->Hash(inNode->mX, inNode->mY);

 if(this->mClosedLists[hash] != NULL)
  this->mClosedLists[hash]->mListParent = inNode;

 inNode->mNext = this->mClosedLists[hash];

Size 53 hash table recommended by article.
Will be good with hierarchical pathfinding,
for short hops between tiles.
*/

/*
TODO
Use a tile for collider tile, instead of
pathnode size. This means there will be
1 collider tile per game tile, not 20x20,
so although there will be more checks to
see if mv collide when checking a
certain pathnode, there will be a lot less
memory used, saving cache.
This is incompatible with the first
TODO, to prevent multiple mv in one
pathnode collider tile, although it's
still possible. It's probably best to
implement it to prevent mv getting
stuck. Although it will be much harder to
pass through busy streets and there
might not be a path found when there
might actually be a narrow path.
*/

/*
TODO
"Cheap list" from "lightning fast A*"
article. Basically binary heap, but
with only the 15 cheapest slow nodes.
When cheap list runs out,
select the 15 cheapest items from
slow list and heapify them.
So keep track of the highest cost
in the cheap list. Maybe not even
binary heap. Only add to
cheap list if below highest cost
value. Otherwise, only add to
general slow list.
When feeding more items into the
cheap list, scan the whole general
slow list for lowest cost item.
Add only nodes that are that cheap.
Then find next lowest cost and
add those, until we've filled
15.
Be careful when finding a path
to an slow node that is shorter;
it might have to be reinserted
into the cheap list.
The advantage of the cheap list,
as said in the article, is that
the cost of insertion into the
list and removal of the cheapest
item from the list is minimized,
getting the best of both lists
and binary heaps.
*/

/*
TODO
For mv that take up 2 or some
even number of pathnodes in width,
make sure they are always aligned
to boundaries of pathnodes. This
is so they take up the least space
and can squeeze through the samllest
possible spaces. Need to go through
all the pathfinding, and path
following code to make sure this is
adhered to.
So, a 2-pathnode-wide unit would
be centered on the line between
the two pathnodes.
*/

/*
TODO
In collider tiles and wherever
multiple unit indices are stored,
space can be saved by splitting
unsigned shorts into two or
more integers.
Mv limit is:
#define MOVERS	(4096)
An ushort holds: 65535
65535 / 4096 = 15.999755859375
So one ushort in the collider
tile can hold 14 mv, with
room to hold the "no unit"
value (4096).
A 64x64 map with 20 bytes per
collider tile will only be
77.51953125 kB, as opposed to
6+ MB with one collider tile
per pathnode.
Also, what if the first value
in this ushort array is always
for a building? If 20x20 mv
can be in a collider tile,
and each ushort holds 14,
20x20/14x2bytes=
57.142857142857142857142857142857 bytes
per collider tile. Plus
water, passability info flag.
Also need foliage numbers.
This is going to involve
a tricky process including
getting the remainders,
since ... [edit] no,
actually, 4096 is a power of
2, so we can just divide the
bits of a bitfield.
58 bytes for mv
58 bytes for foliage
2 bytes for bl
= 118 bytes per collider tile
= 468342 bytes per 64x64 map
= 457.365234375 kB
"The original Pentium 4 processor
had a four-way set associative L1
data cache of 8 KB in size, with
64-byte cache blocks. Hence, there
are 8 KB / 64 = 128 cache blocks."
"iPhone 4S A5 has a BUS frequency
of 200MHz, a L1 cache size of 32KB,
and L2 Cache of 1024KB"
4096 is a lot more than AoE2
supports with 200 limit with 8
players. 4096 is enough for 512
with 8 players, or 85 per each of
48 players.
Also, actually, the "no unit"
value would have to be 4095
and unit limit 4095, to divide
up the bitfield into bits, and
not do remainder math etc.
(Count starts at 0 and ends at 4095
for 12-bit ints.)
*/

//topological depth sort
ecbool CompareDepth(const Dl* a, const Dl* b)
{
	//assert( a );
	//assert( b );

	//return a.jobutil > b.jobutil;
	//return ectrue if a goes before b

	/*
	x axis
	\
	 \
	  +

	y axis
	  /
	 /
	+
	*/

#if 0
	/*
	case g.
	overlapping
	b max z in front of a max z
	i.e., b above a
	*/
	if(a->cmmax.z < b->cmmax.z)
		return ectrue;

	/*
	case h.
	overlapping
	a max z in front of b max z
	i.e., a bove b
	*/
	if(a->cmmax.z > b->cmmax.z)
		return ecfalse;
#endif

	if( a->cmmax.z <= b->cmmin.z )
		return ectrue;
	if( b->cmmax.z <= a->cmmin.z )
		return ecfalse;
	
#if 1
	if( a->cmmax.z <= b->cmmax.z &&
		a->cmmax.x <= b->cmmax.x &&
		a->cmmax.y <= b->cmmax.y )
		return ectrue;
#endif
#if 1
	if( b->cmmax.z <= a->cmmax.z &&
		b->cmmax.x <= a->cmmax.x &&
		b->cmmax.y <= a->cmmax.y )
		return ecfalse;
#endif

	//if(!( a->cmmax.z <= b->cmmin.z ) &&
	//	!( b->cmmax.z <= a->cmmin.z ) )
	{
		/*
		case a.
		non-overlapping
		b min x,y in front of a max x,y
		 /\
		/a \
		\  /
		 \/
		 /\
		/b \
		\  /
		 \/
		*/

		if(a->cmmax.x <= b->cmmin.x &&
			a->cmmax.y <= b->cmmin.y)
			return ectrue;
#if 1
		if(b->cmmax.x <= a->cmmin.x &&
			b->cmmax.y <= a->cmmin.y)
			return ecfalse;
#endif

		/*
		case b.
		non-overlapping
		b min x in front of a max x
		b max y in front of a max y
		 /\
		/a \
		\  /
		 \/\
		 /b \
		 \  /
		  \/
		*/

		if(a->cmmax.x <= b->cmmin.x &&
			a->cmmax.y <= b->cmmax.y)
			return ectrue;
#if 1
		if(b->cmmax.x <= a->cmmin.x &&
			b->cmmax.y <= a->cmmax.y)
			return ecfalse;
#endif
		/*
		case c.
		non-overlapping
		b max x in front of a max x
		b min y in front of a max y
		  /\
		 /a \
		 \  /
		 /\/
		/b \
		\  /
		 \/
		*/

		if(a->cmmax.x <= b->cmmax.x &&
			a->cmmax.y <= b->cmmin.y)
			return ectrue;
	
#if 1
		if(b->cmmax.x <= a->cmmax.x &&
			b->cmmax.y <= a->cmmin.y)
			return ecfalse;
#endif

		/*
		case d.
		overlapping
		b max x in front of a max x
		b max y in front of a max y
		 /\
		/a \
		\/\/
		/\/\
		\b /
		 \/
		*/

		if(a->cmmax.x <= b->cmmax.x &&
			a->cmmax.y <= b->cmmax.y)
			return ectrue;
	
#if 1
		if(b->cmmax.x <= a->cmmax.x &&
			b->cmmax.y <= a->cmmax.y)
			return ecfalse;
#endif
	
		/*
		 case i.
		 overlapping and non-overlapping
		 b max x in front of a max x
		 b min y in front of a min y
		 b max y behind of a max y
			/\
		   / a\
		  /    \
		 /  /\ /
		 \  \b\
		  \  \/
		   \/
		 */
	
		if(a->cmmax.x <= b->cmmax.x &&
		   a->cmmin.y <= b->cmmin.y &&
		   a->cmmax.y >= b->cmmax.y)
			return ectrue;
	
#if 1
		if(b->cmmax.x <= a->cmmax.x &&
		   b->cmmin.y <= a->cmmin.y &&
		   b->cmmax.y >= a->cmmax.y)
			return ecfalse;
#endif
	
		/*
		 case j.
		 overlapping and non-overlapping
		 b min x in front of a min x
		 b max x behind of a max x
		 b max y in front of a max y
			/\
		   / a\
		  /    \
		 /     /
		 \/\  /
		 /b/ /
		 \/\/
		 */
	
		if(a->cmmin.x <= b->cmmin.x &&
		   a->cmmax.x >= b->cmmax.x &&
		   a->cmmax.y <= b->cmmax.y)
			return ectrue;
	
#if 1
		if(b->cmmin.x <= a->cmmin.x &&
		   b->cmmax.x >= a->cmmax.x &&
		   b->cmmax.y <= a->cmmax.y)
			return ecfalse;
#endif

	#if 0
		/*
		case e.
		non-overlapping
		b min x in front of a max x
		b max y behind a min y
		 /\  /\
		/a \/b \
		\  /\  /
		 \/  \/
		*/

		if(a->cmmax.x <= b->cmmin.x &&
			a->cmmin.y >= b->cmmax.y)
			return ectrue;
	#endif

	#if 0
		/*
		case f.
		non-overlapping
		b max x behind a min x
		b min y in front of a max y
		 /\  /\
		/b \/a \
		\  /\  /
		 \/  \/
		*/

		if(a->cmmax.x <= b->cmmin.x &&
			a->cmmin.y >= b->cmmax.y)
			return ectrue;
	#endif
	}

	return ecfalse;
}

// https://mazebert.com/2013/04/18/isometric-depth-sorting/

void VisitNode(Dl* d, std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue)
{
	if(d->visited)
		return;

	d->visited = ectrue;

	for(std::list<Dl*>::iterator subd=d->behind.begin(); subd!=d->behind.end(); subd++)
	{
		if(*subd == NULL)
			break;
		else
		{
			VisitNode(*subd, drawlist, drawqueue);
			*subd = NULL;
		}
	}

	drawqueue.push_back(d);
	d->behind.clear();	//housekeeping clean up
}

void DrawSort3(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue)
{
	//drawqueue = drawlist;
	//return;

	for(std::list<Dl*>::iterator dit=drawlist.begin(); dit!=drawlist.end(); dit++)
	{
		Dl* d = *dit;

		std::list<Dl*>::iterator dit2 = drawqueue.begin();

		for(; dit2!=drawqueue.end(); dit2++)
		{
			Dl* d2 = *dit2;
			
			//is d behind d2?
			if(CompareDepth(d2, d))
				break;
		}

		drawqueue.insert(dit2, *dit);
	}

	drawqueue.reverse();
}

//topological depth sort
void DrawSort(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue)
{
	StartTimer(TIMER_DRAWSORT);

#if 1
	//mark off what is behind each object
	for(std::list<Dl*>::iterator dit=drawlist.begin(); dit!=drawlist.end(); dit++)
	{
		Dl* d = *dit;

		for(std::list<Dl*>::iterator dit2=drawlist.begin(); dit2!=drawlist.end(); dit2++)
		{
			Dl* d2 = *dit2;

			if(d2 == d)
				continue;

			//is d2 behind d?
			if(!CompareDepth(d2, d))
				continue;	//if not, continue

			//if yes, add to d's behind list
			d->behind.push_back(d2);
		}

		d->visited = ecfalse;
	}

	for(std::list<Dl*>::iterator dit=drawlist.begin(); dit!=drawlist.end(); dit++)
	{
		Dl* d = *dit;
		VisitNode(d, drawlist, drawqueue);
	}
#else

	for(std::list<Widget*>::iterator dit=drawlist.begin(); dit!=drawlist.end(); dit++)
	{
		Dl* d = *dit;
		drawqueue.push_back(d);
	}
#endif

	StopTimer(TIMER_DRAWSORT);
}

//compare order numbers based on pathnodes
ecbool CompareDepth2(Dl* a, Dl* b)
{
	//assert( a );
	//assert( b );

	//return a.jobutil > b.jobutil;
	//return ectrue if a goes before b

	return a->pathnode < b->pathnode;
}

//sort using pathnode drawing order
void DrawSort2(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue)
{
	for(std::list<Dl*>::iterator dit=drawlist.begin(); dit!=drawlist.end(); dit++)
	{
		/*
		x axis
		\
		 \
		  +

		y axis
		  /
		 /
		+
		*/

		Dl* d = *dit;

		/*
		use the max x and min y, because we draw like

		for(int x=0; x<...; x++)
			for(int y=0; y<...; y++)

		and out of the four corners of a building box,
		this gives the correct order number with
		surrounding mv.

		We use the min z, because otherwise all bl
		will always be in front of mv.
		*/

		int nx = d->cmmax.x / PATHNODE_SIZE;
		int ny = d->cmmin.y / PATHNODE_SIZE;
		int nz = d->cmmin.z / PATHNODE_SIZE;

		d->pathnode = nx + ny * g_pathdim.x + nz * g_pathdim.x * g_pathdim.y;

		drawqueue.push_back(d);
	}

	//drawqueue.sort(CompareDepth2);
}
