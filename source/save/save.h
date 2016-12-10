

#ifndef SAVE_H
#define SAVE_H

#include "../platform.h"
#include "../render/vertexarray.h"
#include "../math/polygon.h"

struct TexRef
{
public:
	std::string filepath;
	unsigned int diffindex;
	unsigned int texname;
	unsigned int specindex;
	unsigned int normindex;
	unsigned int ownindex;
};

void SaveVertexArray(FILE* fp, VertexArray* va);
void SavePolygon(FILE* fp, Polyg* p);
void ReadVertexArray(FILE* fp, VertexArray* va);
void ReadPolygon(FILE* fp, Polyg* p);

#endif