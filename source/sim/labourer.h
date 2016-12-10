











#ifndef LABOURER_H
#define LABOURER_H

#include "unit.h"

#define LABSND_WORK		0
#define LABSND_SHOP		1
#define LABSND_REST		2
#define LAB_SOUNDS		3

extern short g_labsnd[LAB_SOUNDS];

void UpdLab(Mv* mv);
void UpdLab2(Mv* mv);
void Evict(Bl* b);
void Evict(Mv* mv, ecbool silent);
void Disembark(Mv* op);
int ConsumProp(int satiety, int incomerate, int bprice);
int ConsumProp(Mv* mv, int bprice);
void BestFood(Mv* mv, int* bestbi, int* bestutil);

#endif
