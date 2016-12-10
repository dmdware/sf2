











#ifndef UNITMOVE_H
#define UNITMOVE_H

struct Mv;

void MoveUnit(Mv* mv);
ecbool UnitCollides(Mv* mv, Vec2i cmpos, int mvtype);
ecbool CheckIfArrived(Mv* mv);
void OnArrived(Mv* mv);

#endif
