

#include "octree.h"
#include "../bsp/brush.h"

OcNode::OcNode()
{
	child[OCNODE_TOP_LEFT_FRONT] = NULL;
	child[OCNODE_TOP_LEFT_BACK] = NULL;
	child[OCNODE_TOP_RIGHT_BACK] = NULL;
	child[OCNODE_TOP_RIGHT_FRONT] = NULL;
	child[OCNODE_BOTTOM_LEFT_FRONT] = NULL;
	child[OCNODE_BOTTOM_LEFT_BACK] = NULL;
	child[OCNODE_BOTTOM_RIGHT_BACK] = NULL;
	child[OCNODE_BOTTOM_RIGHT_FRONT] = NULL;
}

OcNode::~OcNode()
{
	for(int i=0; i<8; i++)
	{
		if(child[i])
			delete child[i];
	}
}

void MakeOcNode(OcNode **ocnode, std::list<OcBrushRef> &parentbrushes, Vec3f vmin, Vec3f vmax, int level)
{
	std::list<OcBrushRef> herebrushes;

	for(std::list<OcBrushRef>::iterator bit = parentbrushes.begin(); bit != parentbrushes.end(); bit++)
	{
		Brush* pbrush = bit->pbrush;
		
		ecbool allin = ectrue;

		for(int vertindex = 0; vertindex < pbrush->nsharedv; vertindex++)
		{
			Vec3f v = pbrush->sharedv[vertindex];

			if(v.x < vmin.x || v.y < vmin.y || v.z < vmin.z || v.x > vmax.x || v.y > vmax.y || v.z > vmax.z)
			{
				allin = ecfalse;
				break;
			}
		}

		if(!allin)
			continue;

		herebrushes.push_back(*bit);
		parentbrushes.erase(bit);
	}

	if(herebrushes.size() <= 0)
		return;

	(*ocnode) = new OcNode;

	// pass to lower level nodes

	(*ocnode)->brushes = herebrushes;
}

void MapMinMax(std::list<Brush> &brushes, Vec3f *vmin, Vec3f *vmax)
{
	ecbool firstset = ecfalse;

	for(std::list<Brush>::iterator brushitr = brushes.begin(); brushitr != brushes.end(); brushitr++)
	{
		Brush* b = &*brushitr;

		for(int vertindex = 0; vertindex < b->nsharedv; vertindex++)
		{
			Vec3f v = b->sharedv[vertindex];

			if(!firstset)
			{
				*vmin = v;
				*vmax = v;
				firstset = ectrue;
			}
			else
			{
				if(v.x < vmin->x)
					vmin->x = v.x;
				if(v.y < vmin->y)
					vmin->y = v.y;
				if(v.z < vmin->z)
					vmin->z = v.z;
				
				if(v.x > vmax->x)
					vmax->x = v.x;
				if(v.y > vmax->y)
					vmax->y = v.y;
				if(v.z > vmax->z)
					vmax->z = v.z;
			}
		}
	}
}

void ConstructOc(OcNode **ochead, std::list<Brush> &brushes)
{
	std::list<OcBrushRef> passbrushes;

	int brushindex = 0;
	for(std::list<Brush>::iterator brushitr = brushes.begin(); brushitr != brushes.end(); brushitr++)
	{
		OcBrushRef ocbrushref;
		ocbrushref.brushindex = brushindex;
		ocbrushref.pbrush = &*brushitr;
		passbrushes.push_back(ocbrushref);
		brushindex++;
	}

	Vec3f vmin, vmax;

	MapMinMax(brushes, &vmin, &vmax);

	MakeOcNode(ochead, passbrushes, vmin, vmax, 0);
}