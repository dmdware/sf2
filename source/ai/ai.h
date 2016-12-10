










#ifndef AI_H
#define AI_H

#include "../sim/simdef.h"

struct Py;
struct Bl;

#define AI_FRAMES	(CYCLE_FRAMES/30+1)

void UpdAI();
void UpdAI2(Py* py);
void AdjProd(Py* py, Bl* b);
void AdjTrPrWg(Py* py, Mv* mv);
void AdjPrWg(Py* py, Bl* b);
void BuyProps(Py* py);
void AIBuild(Py* py);
void PlotCd(char pyi, char ctype, Vec2uc from, Vec2uc to);
void AIManuf(Py* py);
void AdjCs(Py* py);
void ConnectCd(Py* py, char ctype, Vec2uc tplace);

#endif