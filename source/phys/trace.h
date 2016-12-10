










#ifndef TRACE_H
#define TRACE_H

struct Vec2i;
struct Mv;
struct Bl;

int Trace(int mvtype, int umode,
          Vec2i vstart, Vec2i vend,
          Mv* thisu, Mv* targu, Bl* targb);

#endif
