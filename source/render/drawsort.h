












#ifndef DRAWSORT_H
#define DRAWSORT_H

#include "../platform.h"
#include "depthable.h"

extern std::list<Dl> g_drawlist;
extern std::list<Dl*> g_subdrawq;
ecbool CompareDepth(const Dl* a, const Dl* b);
void DrawSort(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue);
void DrawSort2(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue);
void DrawSort3(std::list<Dl*>& drawlist, std::list<Dl*>& drawqueue);

#endif
