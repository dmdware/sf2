

#ifndef LIST_H
#define LIST_H

#include "../platform.h"
#include "ecbool.h"

struct Node
{
    struct Node *next;
    struct Node *prev;
	char data[0];
};

typedef struct Node Node;

struct List
{
	int size;
	Node *head;
	Node *tail;
};

typedef struct List List;

void List_init(List *l);
void List_free(List *l);
void List_pushback(List *l, int size);
void List_pushback2(List *l, int size, void *data);
void List_erase(List *l, Node *link);
void List_unlink(List *l, Node *link);
void List_linkback(List *l, Node *link);
void List_swap(List *l, Node *a, Node *b);
void List_sort(List *l, ecbool (*comparefunc)(void *a, void *b));

#endif