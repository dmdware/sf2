

#include "ecbool.h"
#include "list.h"

void List_init(List *l)
{
	l->size = 0;
	l->head = NULL;
	l->tail = NULL;
}

void List_free(List *l)
{
	Node *it;
	Node *next;

	for(it=l->head; it; it=next)
	{
		next = it->next;
		free(it);
	}

	l->size = 0;
	l->head = NULL;
	l->tail = NULL;
}

void List_pushback(List *l, int size)
{
	Node *link = (Node*)malloc(sizeof(Node)+size);
	link->next = NULL;
	link->prev = l->tail;

	if(!l->head)
		l->head = link;
	
	if(l->tail)
		l->tail->next = link;

	l->tail = link;

	++l->size;
}

void List_pushback2(List *l, int size, void *data)
{
	Node *link = (Node*)malloc(sizeof(Node)+size);
	link->next = NULL;
	link->prev = l->tail;
	
	if(!l->head)
		l->head = link;
	
	if(l->tail)
		l->tail->next = link;
	
	l->tail = link;
	
	++l->size;
	
	memcpy(link->data, data, size);
}

void List_erase(List *l, Node *link)
{
	if(!link->prev)
		l->head = link->next;
	else
		link->prev->next = link->next;

	if(!link->next)
		l->tail = link->prev;
	else
		link->next->prev = link->prev;

	free(link);

	--l->size;
}

void List_unlink(List *l, Node *link)
{
	if(!link->prev)
		l->head = link->next;
	else
		link->prev->next = link->next;

	if(!link->next)
		l->tail = link->prev;
	else
		link->next->prev = link->prev;

	/* free(link); */

	--l->size;
}

void List_linkback(List *l, Node *link);
{
	link->next = NULL;
	link->prev = l->tail;
	
	if(!l->head)
		l->head = link;
	
	if(l->tail)
		l->tail->next = link;
	
	l->tail = link;
	
	++l->size;
}

void List_swap(List *l, Node *a, Node *b)
{
    Node *anext;
    Node *aprev;
    Node *bnext;
    Node *bprev;
    
    anext = a->next;
    aprev = a->prev;
    bnext = b->next;
    bprev = b->prev;
    
    a->next = bnext;
    a->prev = bprev;
    b->next = anext;
    b->prev = aprev;
    
    if(anext)
        anext->prev = b;
    if(aprev)
        aprev->next = b;
    if(bnext)
        bnext->prev = a;
    if(bprev)
        bprev->next = a;
    
    if(l->head == a)
        l->head = b;
    else if(l->head == b)
        l->head = a;
    
    if(l->tail == a)
        l->tail = b;
    else if(l->tail == b)
        l->tail = a;
}

//TODO make more efficient
//compare func answers the question "does a go before b?"
//IMPORTANT: MUST return ectrue (1) for values that are equal with regards to order in compare func
//otherwise, infinite loop will occur!
void List_sort(List *l, ecbool (*comparefunc)(void *a, void *b))
{
	Node *it, *returnit;
	
	it = l->head;
	returnit = NULL;
	
	while(ectrue)
	{
		if(!returnit)
		{
			if(!it)
				break;
			if(!it->next)
				break;
			
			//is "it" and "it->next" in the correct order?
			if(comparefunc(&it->data, &it->next->data))
			{
				//if so, then continue to next
				it = it->next;
				continue;
			}
			
			//otherwise, record this place to return to later,
			//and start moving it->next backward
			returnit = it;
			List_swap(l, it, it->next);
			it = it->prev->prev;	//the next "it" to check
			continue;
		}
		else if(!it)
		{
			it = returnit;
			returnit = NULL;
			continue;
		}
		//at this point, since we're backtracking, we know there's an "it->next"
		//does "it->next" go after "it"?
		else if(comparefunc(&it->data, &it->next->data))
		{
			//then we can stop back and return to old spot
			it = returnit;
			returnit = NULL;
			continue;
		}
		//otherwise, move "lowit" up one spot and continue back
		else
		{
			List_swap(l, it, it->next);	//keep moving backward
			it = it->prev->prev;	//the next "it" to check
			continue;
		}
	}
}

