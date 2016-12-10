











#include "vertexarray.h"
#include "../utils.h"

VertexArray::VertexArray(const VertexArray& original)
{
	//Log("vertex array copy constructor");
	/*
	alloc(original.numverts);
	memcpy(vertices, original.vertices, sizeof(Vec3f)*numverts);
	memcpy(texcoords, original.texcoords, sizeof(Vec2f)*numverts);
	memcpy(normals, original.normals, sizeof(Vec3f)*numverts);*/

	*this = original;
}


VertexArray& VertexArray::operator=(VertexArray const &original)
{
	//Log("vertex array assignment op");

	alloc(original.numverts);
	memcpy(vertices, original.vertices, sizeof(Vec3f)*numverts);
	memcpy(texcoords, original.texcoords, sizeof(Vec2f)*numverts);
	memcpy(normals, original.normals, sizeof(Vec3f)*numverts);
	//memcpy(tangents, original.tangents, sizeof(Vec3f)*numverts);

	return *this;
}

void VertexArray::alloc(int numv)
{
	free();
	numverts = numv;
	vertices = new Vec3f[numv];
	texcoords = new Vec2f[numv];
	normals = new Vec3f[numv];
	//tangents = new Vec3f[numv];
}

void VertexArray::free()
{
	if(numverts <= 0)
		return;

	delete [] vertices;
	delete [] texcoords;
	delete [] normals;
	//delete [] tangents;
	numverts = 0;

	delvbo();
}

void VertexArray::genvbo()
{
	delvbo();

	glGenBuffers(VBOS, vbo);

	//GL_ARRAY_BUFFER_ARB
	//GL_STATIC_DRAW_ARB
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f)*numverts, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f)*numverts, texcoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f)*numverts, normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexArray::delvbo()
{
	for(int i=0; i<VBOS; i++)
	{
		if(vbo[i] == -1)
			continue;
		glDeleteBuffers(1, &vbo[i]);
		vbo[i] = -1;
	}
}

void CopyVA(VertexArray* to, const VertexArray* from)
{
	to->alloc(from->numverts);

	for(int i=0; i<from->numverts; i++)
	{
		to->vertices[i] = from->vertices[i];
		to->texcoords[i] = from->texcoords[i];
		to->normals[i] = from->normals[i];
	}
}

void CopyVAs(VertexArray** toframes, int* tonframes, VertexArray* const* fromframes, int fromnframes)
{
	*tonframes = fromnframes;

	(*toframes) = new VertexArray[fromnframes];

	for(int i=0; i<fromnframes; i++)
	{
		CopyVA(&(*toframes)[i], &(*fromframes)[i]);
	}
}
