










#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../algo/binheap.h"
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "pathjob.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/transport.h"

//#define TRANSPORT_DEBUG

// https://github.com/Yonaba/Jumper/blob/master/jumper/search/jps.lua

// Returns neighbours. The returned value is an array of __walkable__ nodes neighbouring a given `node`.
std::list<PathNode*> GetNeighbours(PathJob* pj, PathNode* node)
{
	std::list<PathNode*> neighbours;
	
#if 0	//fixme
	Vec2i pos = PATHNODEPOS(node);

	for( int32_t i=0; i<ARRSZ(STRAIGHTOFF); i++ )
	{
		PathNode* n = PATHNODEAT(
			pos.x + STRAIGHTOFF[i].x,
			pos.y + STRAIGHTOFF[i].y
			);

		Vec2i p = PATHNODEPOS(n);

		//fixme
		//if( n && Standable(pj, p.x, p.y) )
		{
			neighbours.push_back(n);
		}
	}

	if( !1 )
	{
		return neighbours;    //if not allow diagonal
	}

	for( int32_t i=0; i<ARRSZ(DIAGOFF); i++ )
	{
		PathNode* n = PATHNODEAT(
			pos.x + DIAGOFF[i].x,
			pos.y + DIAGOFF[i].y
			);

		Vec2i p = PATHNODEPOS(n);

		bool tunnel = false;

		//fixme
		//if( n && Standable(pj, p.x, p.y) )
		{
			if( tunnel )
			{
				neighbours.push_back(n);
			}
			else
			{
				bool skip = false;
				//PathNode* n1 = PATHNODEAT(p.x+DIAGOFF[i].x, p.y);
				//PathNode* n2 = PATHNODEAT(p.x, p.y+DIAGOFF[i].y);
				//polyf fix
				PathNode* n1 = PATHNODEAT(pos.x+DIAGOFF[i].x, pos.y);
				PathNode* n2 = PATHNODEAT(pos.x, pos.y+DIAGOFF[i].y);
				Vec2i p1 = PATHNODEPOS(n1);
				Vec2i p2 = PATHNODEPOS(n2);

				//if( (n1 && n2) && !Standable(pj, p1.x, p1.y) && !Standable(pj, p2.x, p2.y) )
				//polyf fix
				//fixme
				//if( !n1 || !n2 || !Standable(pj, p1.x, p1.y) || !Standable(pj, p2.x, p2.y) )
				{
					skip = true;
				}
#ifdef RANDOM8DEBUG
				if(pj->thisu == thatunit)
				{
					Log("diag off PATH skip="<<skip<<" off="<<DIAGOFF[i].x<<","<<DIAGOFF[i].y<<" Standable(pj, p1.x, p1.y)="<<Standable(pj, p1.x, p1.y)<<" p1="<<p1.x<<","<<p1.y<<" ");
				}
#endif

				if( !skip )
				{
					neighbours.push_back(n);
				}
			}
		}
	}
#endif

	return neighbours;
}


/*
Looks for the neighbours of a given node.
Returns its natural neighbours plus forced neighbours when the given
node has no parent (generally occurs with the starting node).
Otherwise, based on the direction of move from the parent, returns
neighbours while pruning directions which will lead to symmetric paths.

In case diagonal moves are forbidden, when the given node has no
parent, we return straight neighbours (up, down, left and right).
Otherwise, we add left and right node (perpendicular to the direction
of move) in the neighbours std::list.
*/
std::list<PathNode*> FindNeighbours(PathJob* pj, PathNode* node)
{
#if 0	//fixme
	if( node->prev && node->prev != node )
	{
		std::list<PathNode*> neighbours;

		Vec2i pos = PATHNODEPOS(node);

		// Node have a parent, we will prune some neighbours
		// Gets the direction of move
		Vec2i delta = pos - PATHNODEPOS(node->prev);
		Vec2i prevpos = PATHNODEPOS(node->prev);
		delta.x = (pos.x-prevpos.x)/imax(iabs(pos.x-prevpos.x),1);
		delta.y = (pos.y-prevpos.y)/imax(iabs(pos.y-prevpos.y),1);

		// Diagonal move case
		if( delta.x!=0 && delta.y!=0 )
		{
			bool walkY = false;
			bool walkX = false;

			// Natural neighbours
			//fixme//if( Standable(pj, pos.x, pos.y+delta.y) )
			{
				neighbours.push_back(PATHNODEAT(pos.x, pos.y+delta.y));
				walkY = true;
			}
			//fixme//if( Standable(pj, pos.x+delta.x, pos.y) )
			{
				neighbours.push_back(PATHNODEAT(pos.x+delta.x, pos.y));
				walkX = true;
			}

#if 0
			if( walkX || walkY ) neighbours.push_back(PATHNODEAT(pos.x+delta.x, pos.y+delta.y));
#else
			// polyf edit - trip corners
			if( walkX && walkY ) neighbours.push_back(PATHNODEAT(pos.x+delta.x, pos.y+delta.y));
#endif

			// Forced neighbours
#if 0
			if( !Standable(pj, pos.x-delta.x, pos.y) && walkY ) neighbours.push_back(PATHNODEAT(pos.x-delta.x, pos.y+delta.y));
			if( !Standable(pj, pos.x, pos.y-delta.y) && walkX ) neighbours.push_back(PATHNODEAT(pos.x+delta.x, pos.y-delta.y));
#elif 0
			// polyf edit - trip corners
			if( !Standable(pj, pos.x-delta.x, pos.y) && walkY && walkX ) neighbours.push_back(PATHNODEAT(pos.x-delta.x, pos.y+delta.y));
			if( !Standable(pj, pos.x, pos.y-delta.y) && walkX && walkY ) neighbours.push_back(PATHNODEAT(pos.x+delta.x, pos.y-delta.y));
#else
			// polyf edit - trip corners

#endif
		}
		else
		{
			// Move along Y-axis case
			if( delta.x==0 )
			{
				bool walkY = false;

				//fixme//if( Standable(pj, pos.x, pos.y+delta.y) )
				{
					neighbours.push_back( PATHNODEAT(pos.x, pos.y+delta.y) );

#if 0
					// Forced neighbours are left and right ahead along Y
					if( !Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x+1, pos.y+delta.y) );
					if( !Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x-1, pos.y+delta.y) );
#elif 0
					//polyf edit - trip corners
					if( Standable(pj, pos.x, pos.y+delta.y) )
					{
						if( !Standable(pj, pos.x+1, pos.y-delta.y) && Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x+1, pos.y+delta.y) );
						if( !Standable(pj, pos.x-1, pos.y-delta.y) && Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x-1, pos.y+delta.y) );
					}
#else
				}
#endif
				//polyf edit - trip corners
				//fixme//if( !Standable(pj, pos.x+1, pos.y-delta.y) )
				{
					neighbours.push_back( PATHNODEAT(pos.x+1, pos.y) );

					//fixme//if( Standable(pj, pos.x, pos.y+delta.y) && Standable(pj, pos.x+1, pos.y) )
						neighbours.push_back( PATHNODEAT(pos.x+1, pos.y+delta.y) );
				}
				//fixme//if( !Standable(pj, pos.x-1, pos.y-delta.y) )
				{
					neighbours.push_back( PATHNODEAT(pos.x-1, pos.y) );

					//fixme//if( Standable(pj, pos.x, pos.y+delta.y) && Standable(pj, pos.x-1, pos.y) )
						neighbours.push_back( PATHNODEAT(pos.x-1, pos.y+delta.y) );
				}

				// In case diagonal moves are forbidden : Needs to be optimized
				if( !1 ) //if not allow diagonal
				{
					//fixme//if( Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x+1, pos.y) );
					//fixme//if( Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PATHNODEAT(pos.x-1, pos.y) );
				}
			}
			else
			{
				// Move along X-axis case
				//fixme//if( Standable(pj, pos.x+delta.x, pos.y) )
				{
					neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y) );

					// Forced neighbours are up and down ahead along X
#if 0
					if( !Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y+1) );
					if( !Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y-1) );
#elseif 0
					//polyf edit - trip corners
					if( Standable(pj, pos.x+delta.x, pos.y) )
					{
						if( !Standable(pj, pos.x-delta.x, pos.y+1) && Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y+1) );
						if( !Standable(pj, pos.x-delta.x, pos.y-1) && Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y-1) );
					}
#else
				}
#endif

				//polyf edit - trip corners
				//fixme//if( !Standable(pj, pos.x-delta.x, pos.y+1) )
				{
					neighbours.push_back( PATHNODEAT(pos.x, pos.y+1) );

					//fixme//if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y+1) )
						neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y+1) );
				}
				//fixme//if( !Standable(pj, pos.x-delta.x, pos.y-1) )
				{
					neighbours.push_back( PATHNODEAT(pos.x, pos.y-1) );

					//fixme//if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y-1) )
						neighbours.push_back( PATHNODEAT(pos.x+delta.x, pos.y-1) );
				}

				// : In case diagonal moves are forbidden
				if( !1 ) //if no diagonal
				{
					//fixme//if( Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PATHNODEAT(pos.x, pos.y+1) );
					//fixme//if( Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PATHNODEAT(pos.x, pos.y-1) );
				}
			}
		}

		return neighbours;
	}
#endif

	// Node do not have parent, we return all neighbouring nodes
	return GetNeighbours(pj, node);
}


/*
Searches for a jump point (or a turning point) in a specific direction.
This is a generic translation of the algorithm 2 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf
The current expanded node is a jump point if( near a forced node

In case diagonal moves are forbidden, when lateral nodes (perpendicular to
the direction of moves are walkable, we force them to be turning points in other
to perform a straight move.
*/

PathNode* Jump(PathJob* pj, PathNode* node, PathNode* parent)
{
	if(!node) return NULL;

#if 0	//fixme
	Vec2i pos = PATHNODEPOS(node);

#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("pathing at("<<pos.x<<","<<pos.y<<") dgoal("<<(pj->goalx-pos.x)<<","<<(pj->goaly-pos.y));
		
	}
#endif

	pj->searchdepth++;

	// If the node to be examined is unwalkable, return nil
	//fixme//if( !Standable(pj, pos.x, pos.y) )
	{

#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("\t !st1 sd"<<pj->searchdepth<<" ");
	}
#endif

		return NULL;
	}

	// If the node to be examined is the endNode, return this node
	if( AtGoal(pj, node) )
	{
#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("AT GOAL SUCCESS pathing at("<<pos.x<<","<<pos.y<<") dgoal("<<(pj->goalx-pos.x)<<","<<(pj->goaly-pos.y));
		
	}
#endif

		return node;
	}

	Vec2i delta = pos - PATHNODEPOS(parent);

	// Diagonal search case
	if( delta.x!=0 && delta.y!=0 )
	{
#if 0	//?works?
		// Current node is a jump point if one of his leftside/rightside neighbours ahead is forced
		if( (Standable(pj, pos.x-delta.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y-delta.y) && !Standable(pj, pos.x, pos.y-delta.y)) )
		{
			return node;
		}
#elif 0
		//try fix polyf
		if( (Standable(pj, pos.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y+delta.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y) && !Standable(pj, pos.x+delta.x, pos.y-delta.y)) )
		{
			return node;
		}
		//this shouldn't return, should discover jump point with straight horizontal search.
		//this should return only when corners can be cut, which would mean a collision in our case.
#elif 0
		//try fix polyf 2
		if( (Standable(pj, pos.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y+delta.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y) && !Standable(pj, pos.x+delta.x, pos.y-delta.y)) )
		{
			return node;
		}
#endif
	}
	else
	{
		// Search along X-axis case
		if( delta.x!=0 )
		{
			if( 1 )		//allow diagonal?
			{
#if 0
				// Current node is a jump point if one of his upside/downside neighbours is forced
				if( (Standable(pj, pos.x+delta.x, pos.y+1) && !Standable(pj, pos.x, pos.y+1)) ||
					(Standable(pj, pos.x+delta.x, pos.y-1) && !Standable(pj, pos.x, pos.y-1)) )
#else
				//polyf edit - corner fix
				//fixme//if( (Standable(pj, pos.x, pos.y+1) && !Standable(pj, pos.x-delta.x, pos.y+1)) ||
				//fixme//	(Standable(pj, pos.x, pos.y-1) && !Standable(pj, pos.x-delta.x, pos.y-1)) )
#endif
				{


#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("\t deltax!=0 if sd"<<pj->searchdepth<<" ");
	}
#endif

					return node;
				}
				//polyf fix
				//fixme//else
					return Jump(pj, PATHNODEAT(pos.x+delta.x, pos.y), node);
			}
			else
			{
				// : in case diagonal moves are forbidden
				//fixme//if( Standable(pj, pos.x+1, pos.y) || Standable(pj, pos.x-1, pos.y) )
				{

#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("\t deltax!=0 else sd"<<pj->searchdepth<<" ");
	}
#endif

					return node;
				}
			}
		}
		else if( delta.y!=0 )
		{
			// Search along Y-axis case
			// Current node is a jump point if one of his leftside/rightside neighbours is forced
			if( 1 ) //allow diagonal?
			{
#if 0
				if( (Standable(pj, pos.x+1, pos.y+delta.y) && !Standable(pj, pos.x+1, pos.y)) ||
					(Standable(pj, pos.x-1, pos.y+delta.y) && !Standable(pj, pos.x-1, pos.y)) )
#else
				//fixme//if( (Standable(pj, pos.x+1, pos.y) && !Standable(pj, pos.x+1, pos.y-delta.y)) ||
				//fixme//	(Standable(pj, pos.x-1, pos.y) && !Standable(pj, pos.x-1, pos.y-delta.y)) )
#endif
				{
					return node;
				}
				//polyf fix
				//fixme//else
					return Jump(pj, PATHNODEAT(pos.x, pos.y+delta.y), node);
			}
			else
			{
				// : in case diagonal moves are forbidden
				//fixme//if( Standable(pj, pos.x, pos.y+1) || Standable(pj, pos.x, pos.y-1) )
				{
					return node;
				}
			}
		}
	}

	// Recursive horizontal/vertical search
	if( delta.x!=0 && delta.y!=0 )
	{
		PathNode* jump;
		if( jump = Jump(pj, PATHNODEAT(pos.x+delta.x, pos.y), node) )
		{
			//jump->prev = node;
			return node;
		}
		if( jump = Jump(pj, PATHNODEAT(pos.x, pos.y+delta.y), node) )
		{
			//jump->prev = node;
			return node;
		}

		// Recursive diagonal search
		if( 1 ) //allow diagonal?
		{
#if 0
			if( Standable(pj, pos.x+delta.x, pos.y) || Standable(pj, pos.x, pos.y+delta.y) )
			{
				return Jump(pj, PATHNODEAT(pos.x+delta.x, pos.y+delta.y), node);
			}
#else
			// polyf edit - tripping on corners
			//fixme//if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y+delta.y) )
			{
				PathNode* jump = Jump(pj, PATHNODEAT(pos.x+delta.x, pos.y+delta.y), node);
				return jump;
			}
#endif
		}
	}
#endif

	return NULL;
}

/*
Searches for successors of a given node in the direction of each of its neighbours.
This is a generic translation of the algorithm 1 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf

Also, we notice that processing neighbours in a reverse order producing a natural
looking path, as the pathfinder tends to keep heading in the same direction.
In case a jump point was found, and this node happened to be diagonal to the
node currently expanded in a straight mode search, we skip this jump point.
*/
void Expand_JPS(PathJob* pj, PathNode* node)
{
#if 0	//fixme
	PathNode* endnode = NULL;
	Vec2i pos = PATHNODEPOS(node);

	// Gets the valid neighbours of the given node
	// Looks for a jump point in the direction of each neighbour
	std::list<PathNode*> neighbours = FindNeighbours(pj, node);


#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("find n "<<neighbours.size());
	}
#endif

	for( std::list<Widget*>::iterator niter = neighbours.rbegin(); niter != neighbours.rend(); niter++ )
	{
		bool skip = false;
		PathNode* neighbour = *niter;

		Vec2i npos = PATHNODEPOS(neighbour);

		PathNode* jumpnode = Jump(pj, neighbour, node);

		Vec2i jumppos = PATHNODEPOS(jumpnode);

#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("jump? "<<(bool)(jumpnode != NULL));
	}
#endif

		// : in case a diagonal jump point was found in straight mode, skip it.
		if( jumpnode && !1 ) // and not allow diagonal
		{
			if( jumppos.x != pos.x && jumppos.y != pos.y )
			{
				skip = true;
			}
		}

		// Performs regular A-star on a set of jump points
		if( jumpnode && !skip )
		{
			//int32_t jumpi = jumpnode - g_pathnode;

			// Update the jump node and move it in the closed std::list if it wasn't there
			if( !jumpnode->closed )
			{
				Vec2i jumpoff = jumppos - pos;
				int32_t extraD = PATHHEUR(jumpoff) * 2;
				int32_t newd = node->rund + extraD;

				if( !jumpnode->opened || newd < jumpnode->rund )
				{
					g_toclear.push_back(jumpnode); // Records this node to reset its properties later.
					jumpnode->rund = newd;
					Vec2i jumpoff = jumppos - Vec2i(pj->goalx, pj->goaly);
					int32_t H = PATHHEUR( jumpoff ) * 2;
					jumpnode->heapkey.cost = jumpnode->rund + H;
					jumpnode->prev = node;

					if( !jumpnode->opened )
					{
						g_openlist.insert(jumpnode);
						jumpnode->opened = true;
					}
					else
					{
						g_openlist.heapify(jumpnode);
					}
				}
			}
		}
	}
#endif
}
