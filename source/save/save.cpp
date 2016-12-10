

#include "../sim/map.h"
#include "save.h"
#include "../platform.h"
#include "../math/polygon.h"
#include "../texture.h"
#include "../render/vertexarray.h"
#include "../utils.h"

//#define LOADMAP_DEBUG

void SavePolygon(FILE* fp, Polyg* p)
{
	/*
	std::list<Vec3f> vertex;	//used for constructing the polygon on-the-fly
	Vec3f* drawva;	//used for drawing outline
	*/

	int nvertex = p->edv.size();
	fwrite(&nvertex, sizeof(int), 1, fp);

	for(std::list<Vec3f>::iterator i=p->edv.begin(); i!=p->edv.end(); i++)
	{
		fwrite(&*i, sizeof(Vec3f), 1, fp);
	}

	fwrite(p->drawoutva, sizeof(Vec3f), nvertex, fp);
}

void ReadPolygon(FILE* fp, Polyg* p)
{
	int nvertex;
	fread(&nvertex, sizeof(int), 1, fp);

	for(int i=0; i<nvertex; i++)
	{
		Vec3f v;
		fread(&v, sizeof(Vec3f), 1, fp);
		p->edv.push_back(v);
	}

	p->drawoutva = new Vec3f[nvertex];
	fread(p->drawoutva, sizeof(Vec3f), nvertex, fp);
}

void SaveVertexArray(FILE* fp, VertexArray* va)
{
	/*
	int numverts;
	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;
	*/

	fwrite(&va->numverts, sizeof(int), 1, fp);
	fwrite(va->vertices, sizeof(Vec3f), va->numverts, fp);
	fwrite(va->texcoords, sizeof(Vec2f), va->numverts, fp);
	fwrite(va->normals, sizeof(Vec3f), va->numverts, fp);
	//fwrite(va->tangents, sizeof(Vec3f), va->numverts, fp);

#if 0
	Log("write VA");
	for(int vertindex = 0; vertindex < va->numverts; vertindex++)
	{
		Vec3f vert = va->vertices[vertindex];
		Log("\twrite vert: "<<vert.x<<","<<vert.y<<","<<vert.z<<std::endl;
	}
#endif
}

void ReadVertexArray(FILE* fp, VertexArray* va)
{
	int nverts;
	fread(&nverts, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nverts = "<<nverts<<std::endl;
	
#endif

	va->alloc(nverts);
	fread(va->vertices, sizeof(Vec3f), va->numverts, fp);
	fread(va->texcoords, sizeof(Vec2f), va->numverts, fp);
	fread(va->normals, sizeof(Vec3f), va->numverts, fp);
	//fread(va->tangents, sizeof(Vec3f), va->numverts, fp);
}

