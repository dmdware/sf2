











#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"

#define VBO_POSITION		0
#define VBO_TEXCOORD		1
#define VBO_NORMAL			2
#define VBOS				3

/* TODO swirly shader effects for aliens team */

struct VertexArray
{
public:
	int numverts;
	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;
	//Vec3f* tangents;
	unsigned int vbo[VBOS];

	VertexArray(const VertexArray& original);
	VertexArray& operator=(VertexArray const &original);
	VertexArray()
	{
		numverts = 0;
		for(int i=0; i<VBOS; i++)
			vbo[i] = -1;
	}

	~VertexArray()
	{
		free();
	}

	void genvbo();
	void delvbo();
	void alloc(int numv);
	void free();
};



void CopyVA(VertexArray* to, const VertexArray* from);
void CopyVAs(VertexArray** toframes, int* tonframes, VertexArray* const* fromframes, int fromnframes);

#endif
