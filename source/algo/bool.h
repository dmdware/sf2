

#ifndef BOOL_H
#define BOOL_H

typedef unsigned char ecbool;

/*
 Don't change these as these depend on
 statements like:
 
 return (ecbool)(heap->total > 0);
 if(found)
 */
#define ectrue	1
#define ecfalse	0

#endif