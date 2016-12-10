

#ifndef VECTOR_H
#define VECTOR_H

#include "../platform.h"

struct Vector
{
	int capacity;
	int total;
	int unitsz;
	char *items;
};

typedef struct Vector Vector;

void Vector_init(Vector *v, int unitsz);
void Vector_free(Vector *v);
void Vector_pushback(Vector *v, void *item);
void *Vector_get(Vector *v, int i);
void Vector_popback(Vector *v, void *dest);
void Vector_erase(Vector *v, int index);
void Vector_resize(Vector *v, int capacity);

#endif