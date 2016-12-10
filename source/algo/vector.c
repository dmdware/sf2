

#include "vector.h"
#include "ecbool.h"

void Vector_init(Vector *v, int unitsz)
{
	v->items = NULL;
	v->total = 0;
	v->capacity = 0;
	v->unitsz = unitsz;
}

void Vector_free(Vector *v)
{
	free(v->items);
	v->items = NULL;
	//is it the responsibility of the vector to free the item contents?
	
	v->total = 0;
	v->capacity = 0;
	v->unitsz = 0;
}

void Vector_pushback(Vector *v, void *item)
{
	if(v->total == v->capacity)
		Vector_resize(v, v->capacity * 2);
	
	memcpy(Vector_get(v, v->total), item, v->unitsz);
	++v->total;
}

void *Vector_get(Vector *v, int i)
{
	return v->items + i*v->unitsz;
}

void Vector_popback(Vector *v, void *dest)
{
	void *item;
	
	item = Vector_get(v, v->total-1);
	
	memcpy(dest, item, v->unitsz);
	memset(item, 0, v->unitsz);
	
	--v->total;
}

void Vector_erase(Vector *v, int index)
{
	char *last, *ati;
	
	ati = Vector_get(v, index);
	last = Vector_get(v, v->total-1);

	for(; ati<last; ati+=v->unitsz)
		memcpy(ati, ati+v->unitsz, v->unitsz);
	
	memset(last, 0, v->unitsz);
	
	--v->total;
	
	if(v->total > 0 && v->total == v->capacity / 4)
		Vector_resize(v, v->capacity / 2);
}

void Vector_resize(Vector *v, int capacity)
{
	char *items;
 
	items = realloc(v->items, v->unitsz * capacity);
	
	if(items)
	{
		v->items = items;
		v->capacity = capacity;
	}
	else
		OUTOFMEM();
}
