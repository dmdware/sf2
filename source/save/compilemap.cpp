

#include "compilemap.h"
#include "../utils.h"
#include "savemap.h"
#include "../math/camera.h"
#include "../app/segui.h"
#include "../window.h"
#include "../gui/gui.h"
#include "../sim/simtile.h"
#include "../app/appmain.h"
#include "../app/seviewport.h"
#include "../math/vec4f.h"
#include "../tool/rendersprite.h"
#include "../save/saveedm.h"

float g_defrenderpitch = RENDERPITCH;
float g_defrenderyaw = RENDERYAW;
int g_1tilewidth = TILE_PIXEL_WIDTH;
int g_renderframe = 0;

//if a brush vert is behind the plane, we can use it to add a brush side
//otherwise, if no verts are behind the plane, the brush must be pruned/destroyed.
ecbool PruneB2(Brush* b, Plane3f* p, float epsilon)
{
	for(int svi=0; svi<b->nsharedv; svi++)
	{
		Vec3f sv = b->sharedv[svi];

		if(PointOnOrBehindPlane(sv, p->normal, p->d, epsilon))
			return ecfalse;
	}

	return ectrue;
}

//all brush verts behind plane?
ecbool AllBeh(Brush* b, Plane3f* p, float epsilon)
{
	for(int svi=0; svi<b->nsharedv; svi++)
	{
		Vec3f sv = b->sharedv[svi];

		if(!PointOnOrBehindPlane(sv, p->normal, p->d, epsilon))
			return ecfalse;
	}

	return ectrue;
}

void ToCutSide(CutBrushSide* cuts, BrushSide* eds)
{
	/*
struct BrushSide
{
public:
	Plane3f plane;
	VertexArray drawva;
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;

struct BrushSide : public BrushSide
{
public:
	int ntris;
	Triangle2* tris;
	Plane3f tceq[2];	//tex coord uv equations
	Polyg outline;
	int* vindices;	//indices into parent brush's shared vertex array; only stores unique vertices as defined by polygon outline
	Vec3f centroid;
	*/

	/*
struct CutBrushSide
{
public:
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;
	Plane3f tceq[2];
	std::list<Triangle> frag;
	*/

	cuts->diffusem = eds->diffusem;
	cuts->specularm = eds->specularm;
	cuts->normalm = eds->normalm;
	cuts->tceq[0] = eds->tceq[0];
	cuts->tceq[1] = eds->tceq[1];

	for(int i=0; i<eds->ntris; i++)
		cuts->frag.push_back(eds->tris[i]);
}

void ToCutBrush(CutBrush* cutb, Brush* edb)
{
	for(int i=0; i<edb->nsides; i++)
	{
		CutBrushSide cuts;
		ToCutSide(&cuts, &edb->sides[i]);
		cutb->side.push_back(cuts);
	}
}

ecbool BrushTouch(Brush* b1, Brush* b2)
{
	for(int i=0; i<b1->nsides; i++)
	{
		BrushSide* s = &b1->sides[i];
		Plane3f p = s->plane;
		p.d += EPSILON;
		ecbool found = ecfalse;

		for(int j=0; j<b2->nsharedv; j++)
		{
			Vec3f v = b2->sharedv[j];

			if(PointOnOrBehindPlane(v, p))
			{
				found = ectrue;
				break;
			}
		}

		if(!found)
			return ecfalse;
	}

	return ectrue;
}

void FragBrush(CutBrush* cutb, Brush* edb)
{
	for(std::list<CutBrushSide>::iterator i=cutb->side.begin(); i!=cutb->side.end(); i++)
	{
		for(std::list<Triangle>::iterator j=i->frag.begin(); j!=i->frag.end(); j++)
		{

		}
	}
}

static int g_fragerased;

// Remove hidden triangles (triangles that are completely
// covered up by convex hulls)
void RemoveHiddenFrags(CutBrush* cutb, Brush* edb)
{
	for(std::list<CutBrushSide>::iterator i=cutb->side.begin(); i!=cutb->side.end(); i++)
	{
		std::list<Triangle>::iterator j = i->frag.begin();

		while(j != i->frag.end())
		{
			ecbool inall = ectrue;

			for(int k=0; k<edb->nsides; k++)
			{
				BrushSide* s = &edb->sides[k];
				Plane3f* p = &s->plane;

				if(!PointOnOrBehindPlane(j->vertex[0], *p))
				{
					inall = ecfalse;
					break;
				}

				if(!PointOnOrBehindPlane(j->vertex[1], *p))
				{
					inall = ecfalse;
					break;
				}

				if(!PointOnOrBehindPlane(j->vertex[2], *p))
				{
					inall = ecfalse;
					break;
				}
			}

			if(inall)
			{
				j = i->frag.erase( j );
				g_fragerased ++;
				continue;
			}

			j++;
		}
	}
}

static std::list<CutBrush> cutbs;	//the cut brush sides
static int ntouch = 0;	//the number of touching brushes
static EdMap* cmap;	//the map to compile
static std::list<Brush> finalbs; //This will hold the final brushes

void CutBrushes()
{
	cutbs.clear();

	for(std::list<Brush>::iterator i=cmap->brush.begin(); i!=cmap->brush.end(); i++)
	{
		CutBrush cutb;
		ToCutBrush(&cutb, &*i);
		cutbs.push_back(cutb);
	}

	ntouch = 0;

	std::list<Brush>::iterator a = cmap->brush.begin();
	std::list<CutBrush>::iterator cuta = cutbs.begin();
	for(; a!=cmap->brush.end(); a++, cuta++)
	{
		std::list<Brush>::iterator b = cmap->brush.begin();
		std::list<CutBrush>::iterator cutb = cutbs.begin();

		while(&*b != &*a)
		{
			b++;
			cutb++;
		}

		b++;
		cutb++;

		for(; b!=cmap->brush.end(); b++)
		{
			if(!BrushTouch(&*a, &*b) && !BrushTouch(&*b, &*a))
				continue;

			ntouch++;

			//FragBrush(&*cuta, &*b);
			//FragBrush(&*cutb, &*a);
		}
	}

//	Log("num touches: "<<ntouch<<std::endl;

	// For each brush, see which other brush
	//it touches, and remove any hidden fragments
	//covered up by the other brush.
	g_fragerased = 0;
	a = cmap->brush.begin();
	cuta = cutbs.begin();
	for(; a!=cmap->brush.end(); a++, cuta++)
	{
		std::list<Brush>::iterator b = cmap->brush.begin();
		std::list<CutBrush>::iterator cutb = cutbs.begin();

		while(&*b != &*a)
		{
			b++;
			cutb++;
		}

		b++;
		cutb++;

		for(; b!=cmap->brush.end(); b++)
		{
			if(!BrushTouch(&*a, &*b) && !BrushTouch(&*b, &*a))
				continue;

			RemoveHiddenFrags(&*cuta, &*b);
			RemoveHiddenFrags(&*cutb, &*a);
		}
	}
}

void MakeFinalBrushes()
{
	finalbs.clear();

	//Copy the map brushes to final brushes
	//and substitute its drawable vertex array
	//of triangles to the cut-down list.
	std::list<Brush>::iterator brushitr = cmap->brush.begin();
	std::list<CutBrush>::iterator cutbrushitr = cutbs.begin();
	for(; brushitr!=cmap->brush.end(); brushitr++, cutbrushitr++)
	{
		Brush finalb = *brushitr;
		std::list<CutBrushSide>::iterator cutsideitr = cutbrushitr->side.begin();

		for(int sideindex = 0; sideindex < finalb.nsides; cutsideitr++, sideindex++)
		{
			BrushSide* s = &finalb.sides[sideindex];
			CutBrushSide* cuts = &*cutsideitr;
			s->vafromcut(cuts);
		}

		finalbs.push_back(finalb);
	}
}

void CleanUpMapCompile()
{
	cutbs.clear();
	finalbs.clear();
}

void CompileMap(const char* full, EdMap* map)
{
	cmap = map;

//	Log("Compiling map "<<full<<std::endl;
//	Log("num brushes: "<<map->brush.size()<<std::endl;

	CutBrushes();

//	Log("frags removed: "<<g_fragerased<<std::endl;

	MakeFinalBrushes();

	SaveMap(full, finalbs);

	CleanUpMapCompile();
}

void ResetView(ecbool checkupscale)
{
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_projtype = PROJ_ORTHO;
	g_cam.position(0, 0, 10000.0f, 0, 0, 0, 0, 1, 0);
	g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
	g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);

	g_zoom = 1;

	Vec3f topleft(-g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomleft(-g_tilesize/2, 0, g_tilesize/2);
	Vec3f topright(g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomright(g_tilesize/2, 0, g_tilesize/2);

	int width;
	int height;

	if(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME)
	{
		width = g_width;
		height = g_height;
	}
	//if(g_appmode == APPMODE_EDITOR)
	else
	{
		width = g_width;
		height = g_height;
#if 0
		Widget *gui = (Widget*)&g_gui;
		//ViewLayer* edview = (ViewLayer*)g_gui.get("editor");
		Widget* viewportsframe = gui->get("viewports frame");
		Widget* toprightviewport = viewportsframe->get("top right viewport");
		width = toprightviewport->pos[2] - toprightviewport->pos[0];
		height = toprightviewport->pos[3] - toprightviewport->pos[1];
#endif
		//width = g_viewport[3].[2] - g_viewport[3].pos[0];
		//height = g_viewport[3].pos[3] - g_viewport[3].pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

#ifdef DEBUG
	{
		Log("rv"<<aspect<<","<<width<<","<<height<<" r-l:"<<(PROJ_RIGHT*aspect/g_zoom)<<","<<(-PROJ_RIGHT*aspect/g_zoom)<<" gwh:"<<g_width<<","<<g_height<<std::endl;
	}
#endif

	ecbool persp = ecfalse;

	if(g_appmode == APPMODE_EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" rv pmat0:"<<projection.m[0]<<","<<projection.m[1]<<","<<projection.m[2]<<","<<projection.m[3]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat1:"<<projection.m[4]<<","<<projection.m[5]<<","<<projection.m[6]<<","<<projection.m[7]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat2:"<<projection.m[8]<<","<<projection.m[9]<<","<<projection.m[10]<<","<<projection.m[11]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat3:"<<projection.m[12]<<","<<projection.m[13]<<","<<projection.m[14]<<","<<projection.m[15]<<std::endl;
	}
#endif

	VpWrap* v = &g_viewport[3];
	//Vec3f viewvec = g_focus; //g_cam.view;
	//Vec3f viewvec = g_cam.view;
	Vec3f viewvec = v->focus();
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.view + t->offset;
		//viewvec = posvec + Normalize(g_cam.view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->offset;
    //Vec3f upvec = t->up;
    //Vec3f upvec = g_cam.up;
	Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m);
	mvpmat.postmult(viewmat);

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" rv vmat0:"<<viewmat.m[0]<<","<<viewmat.m[1]<<","<<viewmat.m[2]<<","<<viewmat.m[3]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat1:"<<viewmat.m[4]<<","<<viewmat.m[5]<<","<<viewmat.m[6]<<","<<viewmat.m[7]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat2:"<<viewmat.m[8]<<","<<viewmat.m[9]<<","<<viewmat.m[10]<<","<<viewmat.m[11]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat3:"<<viewmat.m[12]<<","<<viewmat.m[13]<<","<<viewmat.m[14]<<","<<viewmat.m[15]<<std::endl;
	}
#endif

	persp = ecfalse;

	Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	float minx = fmin(topleft4.x, fmin(topright4.x, fmin(bottomleft4.x, bottomright4.x)));
	float maxx = fmax(topleft4.x, fmax(topright4.x, fmax(bottomleft4.x, bottomright4.x)));
	//float miny = min(topleft4.y, min(topright4.y, min(bottomleft4.y, bottomright4.y)));
	//float maxy = max(topleft4.y, max(topright4.y, max(bottomleft4.y, bottomright4.y)));

	float xrange = (float)maxx - (float)minx;

	if(xrange <= 0.0f)
		xrange = g_1tilewidth;

	float zoomscale = (float)g_1tilewidth / xrange;

	g_zoom *= zoomscale;

	if(checkupscale && g_antialias)
		g_zoom *= (float)ANTIALIAS_UPSCALE;

#ifdef DEBUG
	Log("zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<std::endl;
#endif
}
