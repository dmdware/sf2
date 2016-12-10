

#include "../platform.h"
#include "brushside.h"
#include "../math/plane3f.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "brush.h"
#include "../save/compilemap.h"

BrushSide& BrushSide::operator=(const BrushSide &original)
{
	//g_log<<"edbrushside assignment operator "<<endl;
	//g_log.flush();

	plane = original.plane;
	diffusem = original.diffusem;
	specularm = original.specularm;
	normalm = original.normalm;
	ownerm = original.ownerm;
	drawva = original.drawva;

	ntris = original.ntris;
	tceq[0] = original.tceq[0];
	tceq[1] = original.tceq[1];
	tris = NULL;
	if(ntris > 0)
	{
		tris = new Triangle2[ntris];
		if(original.tris)
			memcpy(tris, original.tris, sizeof(Triangle2)*ntris);
	}
	outline = original.outline;

	//g_log<<"copy vindices ntris="<<ntris<<endl;
	//g_log.flush();

	vindices = NULL;
	if(ntris > 0)
	{
		vindices = new int[ntris+2];

		if(!original.vindices)
		{
			//g_log<<"!! copy vindices"<<endl;
			//g_log.flush();
			memset(vindices, 0, sizeof(int)*(ntris+2));
		}
		else
			memcpy(vindices, original.vindices, sizeof(int)*(ntris+2));
	}
	
	//g_log<<"end copy vindices"<<endl;
	//g_log.flush();

	centroid = original.centroid;
	sideverts = original.sideverts;

	return *this;
}

BrushSide::BrushSide(const BrushSide& original)
{
	//g_log<<"edbrushside copy constructor"<<endl;
	
	ntris = 0;
	tris = NULL;
	tceq[0] = Plane3f(0.1f,0.1f,0.1f,0);
	tceq[1] = Plane3f(0.1f,0.1f,0.1f,0);
	diffusem = 0;
	vindices = NULL;
	centroid = Vec3f(0,0,0);
	*this = original;

	/*
	plane = original.plane;
	diffusem = original.diffusem;
	drawva = original.drawva;
	*/
	/*
	int ntris;
	Triangle2* tris;
	Plane3f tceq[2];	//tex coord uv equations
	*/
	/*
	ntris = original.ntris;
	tceq[0] = original.tceq[0];
	tceq[1] = original.tceq[1];
	tris = new Triangle2[ntris];
	memcpy(tris, original.tris, sizeof(Triangle2)*ntris);
	//for(int i=0; i<ntris; i++)
	//	tris[i] = original.tris[i];

	
	g_log<<"copy edbrushside plane=n("<<plane.normal.x<<","<<plane.normal.y<<","<<plane.normal.z<<")d="<<plane.d<<endl;
	g_log<<"\tueq=n("<<tceq[0].normal.x<<","<<tceq[0].normal.y<<","<<tceq[0].normal.z<<endl;
	g_log<<"\tveq=n("<<tceq[1].normal.x<<","<<tceq[1].normal.y<<","<<tceq[1].normal.z<<endl;
	g_log.flush();
	*/
}

BrushSide::BrushSide()
{
	//g_log<<"edbrushside constructor default "<<endl;
	//g_log.flush();

	ntris = 0;
	tris = NULL;
	tceq[0] = Plane3f(0.1f,0.1f,0.1f,0);
	tceq[1] = Plane3f(0.1f,0.1f,0.1f,0);
	diffusem = 0;
	vindices = NULL;
	centroid = Vec3f(0,0,0);
}

BrushSide::~BrushSide()
{
	//g_log<<"edbrushsid destructor "<<endl;

	if(tris)
	{
		delete [] tris;
		tris = NULL;
	}

	if(vindices)
	{
		delete [] vindices;
		vindices = NULL;
	}

	ntris = 0;
}

void BrushSide::usedifftex()
{
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[diffusem].texname);
	glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
#ifdef DEBUG
	CHECKGLERROR();
#endif
}

void BrushSide::usespectex()
{
	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[specularm].texname);
	glUniform1i(g_sh[g_curS].slot[SSLOT_SPECULARMAP], 1);
#ifdef DEBUG
	CHECKGLERROR();
#endif
}


void BrushSide::usenormtex()
{
	glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[normalm].texname);
	glUniform1i(g_sh[g_curS].slot[SSLOT_NORMALMAP], 2);
#ifdef DEBUG
	CHECKGLERROR();
#endif
}

void BrushSide::useteamtex()
{
	glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[ownerm].texname);
	glUniform1i(g_sh[g_curS].slot[SSLOT_OWNERMAP], 3);
#ifdef DEBUG
	CHECKGLERROR();
#endif
}

Vec3f PlaneCrossAxis(Vec3f normal)
{
	float mag[6];
	mag[0] = MAG_VEC3F(normal - Vec3f(0,1,0));
	mag[1] = MAG_VEC3F(normal - Vec3f(0,-1,0));
	mag[2] = MAG_VEC3F(normal - Vec3f(1,0,0));
	mag[3] = MAG_VEC3F(normal - Vec3f(-1,0,0));
	mag[4] = MAG_VEC3F(normal - Vec3f(0,0,1));
	mag[5] = MAG_VEC3F(normal - Vec3f(0,0,-1));

	int match = 0;

	for(int i=0; i<6; i++)
	{
		if(mag[i] < mag[match])
			match = i;
	}

	//Vec3f vCross = Cross(view - pos, up);

	Vec3f crossaxis[6];
	crossaxis[0] = Vec3f( 0, 0, -1 );
	crossaxis[1] = Vec3f( 0, 0, 1 );
	crossaxis[2] = Vec3f( 0, 1, 0 );
	crossaxis[3] = Vec3f( 0, 1, 0 );
	crossaxis[4] = Vec3f( 0, 1, 0 );
	crossaxis[5] = Vec3f( 0, 1, 0 );

	return crossaxis[match];
}

void BrushSide::gentexeq()
{	
	Vec3f uaxis = Normalize(Cross(PlaneCrossAxis(plane.normal), plane.normal)) / STOREY_HEIGHT;
	Vec3f vaxis = Normalize(Cross(uaxis, plane.normal)) / STOREY_HEIGHT;
	
	tceq[0] = Plane3f(uaxis.x, uaxis.y, uaxis.z, 0);
	tceq[1] = Plane3f(vaxis.x, vaxis.y, vaxis.z, 0);
}

//#define FITTEX_DEBUG

void BrushSide::fittex()
{
	//Vec3f uaxis = Normalize(Cross(PlaneCrossAxis(plane.normal), plane.normal)) / STOREY_HEIGHT;
	//Vec3f vaxis = Normalize(Cross(uaxis, plane.normal)) / STOREY_HEIGHT;
	
#ifdef FITTEX_DEBUG
	g_log<<"fittex 1"<<endl;
	g_log.flush();
#endif

	Vec3f texaxis[2];
	texaxis[0] = Normalize(tceq[0].normal);
	texaxis[1] = Normalize(tceq[1].normal);

#ifdef FITTEX_DEBUG
	g_log<<"fittex 2"<<endl;
	g_log.flush();
#endif

	Vec3f newaxis[2];
	float closestmag[] = {-1, -1};
	Vec3f startv[2];

	//get the longest side edges and use those as axises for aligning the texture.

	//for(int i=0; i<outline.edv.size(); i++)
	//for(auto viter=sideverts.begin(); viter!=sideverts.end(); viter++)
	for(int i=0; i<drawva.numverts; i = (i==0) ? 1 : (i+2))
	{
#ifdef FITTEX_DEBUG
	//g_log<<"fittex 3 vertex="<<i<<"/"<<outline.edv.size()<<endl;
	g_log<<"fittex 3 vertex="<<i<<"/"<<drawva.numverts<<endl;
	g_log.flush();
#endif
	
		//Vec3f thisv = outline.drawoutva[i];
		Vec3f thisv = drawva.vertices[i];
		Vec3f nextv;
		
		//if(i+1 < outline.edv.size())
		//if(i+1 < outline.edv.size())
		//	nextv = outline.drawoutva[i+1];

		int nexti = (i==0) ? 1 : (i+2);

		if(nexti < drawva.numverts)
			nextv = drawva.vertices[nexti];
		else
			//nextv = outline.drawoutva[0];
			nextv = drawva.vertices[0];

		for(int j=0; j<2; j++)
		{
			Vec3f thisaxis = Normalize( nextv - thisv );
			float mag = MAG_VEC3F(thisaxis - texaxis[j]);

			if(mag < closestmag[j] || closestmag[j] < 0)
			{
				closestmag[j] = mag;
				newaxis[j] = thisaxis;
				startv[j] = thisv;
			}

			thisaxis = Vec3f(0,0,0) - thisaxis;
			mag = MAG_VEC3F(thisaxis - texaxis[j]);

			if(mag < closestmag[j] || closestmag[j] < 0)
			{
				closestmag[j] = mag;
				newaxis[j] = thisaxis;
				startv[j] = nextv;
			}
		}
	}
	
#ifdef FITTEX_DEBUG
	g_log<<"fittex 4"<<endl;
	g_log.flush();
#endif

	float mind[2];
	float maxd[2];

	//for(int i=0; i<outline.edv.size(); i++)
	for(int i=0; i<drawva.numverts; i = (i==0) ? 1 : (i+2))
	{
		Vec3f thisv = drawva.vertices[i];

		for(int j=0; j<2; j++)
		{
			//float thisd = Dot( outline.drawoutva[i], newaxis[j] );
			float thisd = Dot( thisv, newaxis[j] );

			if(thisd < mind[j] || i == 0)
			{
				mind[j] = thisd;
			}

			if(thisd > maxd[j] || i == 0)
			{
				maxd[j] = thisd;
			}
		}
	}

#ifdef FITTEX_DEBUG
	g_log<<"fittex 5"<<endl;
	g_log.flush();
#endif

	for(int i=0; i<2; i++)
	{
		float span = maxd[i] - mind[i];
		tceq[i].normal = newaxis[i] / span;
		tceq[i].d = PlaneDistance(tceq[i].normal, startv[i]);
	}

#ifdef FITTEX_DEBUG
	g_log<<"fittex 6"<<endl;
	g_log.flush();
#endif

	remaptex();

#ifdef FITTEX_DEBUG
	g_log<<"fittex 7"<<endl;
	g_log.flush();
#endif
}

void BrushSide::remaptex()
{
#if 0
	Vec3f* un = &tceq[0].normal;
	Vec3f* vn = &tceq[1].normal;
	float ud = tceq[0].d;
	float vd = tceq[1].d;

	for(int j=0; j<ntris; j++)
	{
		Triangle2* t = &tris[j];

		for(int k=0; k<3; k++)
		{
			Vec3f* v = &t->vertex[k];
			Vec2f* tc = &t->texcoord[k];
				
			tc->x = un->x*v->x + un->y*v->y + un->z*v->z + ud;
			tc->y = vn->x*v->x + vn->y*v->y + vn->z*v->z + vd;
				
			//g_log<<"-----------rebldg tex side"<<i<<" tri"<<j<<" vert"<<k<<"------------"<<endl;
			//g_log<<"remaptex u = "<<un->x<<"*"<<v->x<<" + "<<un->y<<"*"<<v->y<<" + "<<un->z<<"*"<<v->z<<" + "<<ud<<" = "<<tc->x<<endl;
			//g_log<<"remaptex v = "<<vn->x<<"*"<<v->x<<" + "<<vn->y<<"*"<<v->y<<" + "<<vn->z<<"*"<<v->z<<" + "<<vd<<" = "<<tc->y<<endl;
		}

		//for(int j=0; j<va->numverts; j++)
		for(int j=0; j<3; j++)
		{
			Vec2f* tc = &t->texcoord[j];
			//g_log<<"u "<<va->texcoords[j].x<<"\t	v "<<va->texcoords[j].y<<endl;
			//g_log<<"u "<<tc->x<<"\t	v "<<tc->y<<endl;
			//g_log.flush();
		}
	}

	makeva();
#elif 0

	auto piter = wind->points.begin();
		Vec3f first = *piter;
		Vec2f firsttc;
		firsttc.x = first.x * s->tceq[0].normal.x + first.y * s->tceq[0].normal.y + first.z * s->tceq[0].normal.z + s->tceq[0].d;
		firsttc.y = first.x * s->tceq[1].normal.x + first.y * s->tceq[1].normal.y + first.z * s->tceq[1].normal.z + s->tceq[1].d;

		piter++;
		Vec3f prev = *piter;
		Vec2f prevtc;
		prevtc.x = prev.x * s->tceq[0].normal.x + prev.y * s->tceq[0].normal.y + prev.z * s->tceq[0].normal.z + s->tceq[0].d;
		prevtc.y = prev.x * s->tceq[1].normal.x + prev.y * s->tceq[1].normal.y + prev.z * s->tceq[1].normal.z + s->tceq[1].d;

		int triindex = 0;
		piter++;

		do
		{
			Vec3f curr = *piter;
			Vec2f currtc;
			currtc.x = curr.x * s->tceq[0].normal.x + curr.y * s->tceq[0].normal.y + curr.z * s->tceq[0].normal.z + s->tceq[0].d;
			currtc.y = curr.x * s->tceq[1].normal.x + curr.y * s->tceq[1].normal.y + curr.z * s->tceq[1].normal.z + s->tceq[1].d;
			
			s->drawva.vertices[triindex*3 + 0] = first;
			s->drawva.vertices[triindex*3 + 1] = prev;
			s->drawva.vertices[triindex*3 + 2] = curr;
			
#if 1
			//texcoords will be applied in remaptex
			s->drawva.texcoords[triindex*3 + 0] = firsttc;
			s->drawva.texcoords[triindex*3 + 1] = prevtc;
			s->drawva.texcoords[triindex*3 + 2] = currtc;
#endif

			s->drawva.normals[triindex*3 + 0] = s->plane.normal;
			s->drawva.normals[triindex*3 + 1] = s->plane.normal;
			s->drawva.normals[triindex*3 + 2] = s->plane.normal;
			
			prev = curr;
			prevtc = currtc;

			triindex++;
			piter++;
		}while(piter != wind->points.end());
#else
	Vec3f* un = &tceq[0].normal;
	Vec3f* vn = &tceq[1].normal;
	float ud = tceq[0].d;
	float vd = tceq[1].d;

	for(int i=0; i<drawva.numverts; i++)
	{
		Vec3f* v = &drawva.vertices[i];
		Vec2f* tc = &drawva.texcoords[i];
				
		tc->x = un->x*v->x + un->y*v->y + un->z*v->z + ud;
		tc->y = vn->x*v->x + vn->y*v->y + vn->z*v->z + vd;
	}

#endif
}

BrushSide::BrushSide(Vec3f normal, Vec3f point)
{
	ntris = 0;
	tris = NULL;
	//tceq[0] = Plane3f(1,1,1,0);
	//tceq[1] = Plane3f(1,1,1,0);
	diffusem = 0;
	plane = Plane3f(normal.x, normal.y, normal.z, PlaneDistance(normal, point));

	gentexeq();

	//g_log<<"new edbrushside plane=n("<<plane.normal.x<<","<<plane.normal.y<<","<<plane.normal.z<<")d="<<plane.d<<endl;
	//g_log.flush();
	
	CreateTex(diffusem, "textures/notex.jpg", ecfalse, ecfalse);
	specularm = diffusem;
	normalm = diffusem;
	ownerm = diffusem;
	vindices = NULL;
	//centroid = Vec3f(0,0,0);
}

void BrushSide::usetex()
{
	glActiveTextureARB(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[diffusem].texname);
	glUniform1iARB(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
	
	glActiveTextureARB(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[specularm].texname);
	glUniform1iARB(g_sh[g_curS].slot[SSLOT_SPECULARMAP], 1);
	
	glActiveTextureARB(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[normalm].texname);
	glUniform1iARB(g_sh[g_curS].slot[SSLOT_NORMALMAP], 2);
	
	glActiveTextureARB(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[ownerm].texname);
	glUniform1iARB(g_sh[g_curS].slot[SSLOT_OWNERMAP], 3);
}
/*
void BrushSide::usetex()
{
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[diffusem].tex);
	glUniform1iARB(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
}*/

void BrushSide::makeva()
{
	drawva.alloc(ntris * 3);

	//Vec3f tangent = Normalize( Cross(plane.normal, Normalize(tris[0].vertex[0] - tris[0].vertex[1])) );

	for(int i=0; i<ntris; i++)
	{
		for(int j=0; j<3; j++)
		{
			drawva.normals[i*3+j] = plane.normal;
			drawva.vertices[i*3+j] = tris[i].vertex[j];
			drawva.texcoords[i*3+j] = tris[i].texcoord[j];
			//drawva.tangents[i*3+j] = Normalize(tceq[0].normal);
			//drawva.tangents[i*3+j] = tangent;

			//g_log<<"makeva uv="<<drawva.texcoords[i*3+j].x<<","<<drawva.texcoords[i*3+j].y<<endl;
		}
	}

	outline.makeva();
}

//Remake the drawable vertex array from a list of
//triangles in the CutBrushSide struct
void BrushSide::vafromcut(CutBrushSide* cutside)
{
	drawva.free();

	int ntris = cutside->frag.size();
	drawva.alloc(ntris*3);

	int triidx = 0;
	for(std::list<Triangle>::iterator triitr=cutside->frag.begin(); triitr!=cutside->frag.end(); triitr++, triidx++)
	{
		for(int vertidx=0; vertidx<3; vertidx++)
		{
			drawva.vertices[triidx*3+vertidx] = triitr->vertex[vertidx];
			drawva.normals[triidx*3+vertidx] = plane.normal;

			//Reconstruct the texture coordinate according 
			//to the plane equation of the brush side

			Vec3f vert = drawva.vertices[triidx*3+vertidx];

			drawva.texcoords[triidx*3+vertidx].x 
				= tceq[0].normal.x * vert.x
				+ tceq[0].normal.y * vert.y
				+ tceq[0].normal.z * vert.z
				+ tceq[0].d;
			
			drawva.texcoords[triidx*3+vertidx].y
				= tceq[1].normal.x * vert.x
				+ tceq[1].normal.y * vert.y
				+ tceq[1].normal.z * vert.z
				+ tceq[1].d;
		}
	}
}