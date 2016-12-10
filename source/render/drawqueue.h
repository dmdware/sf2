












#ifndef DRAWQUEUE_H
#define DRAWQUEUE_H

#include "../platform.h"

extern int rendtrees;

void DrawQueue(unsigned int renderdepthtex, unsigned int renderfb);
void DrawQueueDepth(unsigned int renderdepthtex, unsigned int renderfb);

#endif
