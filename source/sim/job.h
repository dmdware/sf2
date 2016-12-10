













#ifndef JOB_H
#define JOB_H

#include "labourer.h"

struct Mv;
struct Bl;

//job opportunity
struct JobOpp
{
public:
	int jobutil;
	int jobtype;
	int target;
	int target2;
	//float bestDistWage = -1;
	//float distWage;
	//ecbool fullquota;
	int ctype;	//conduit type
	Vec2i goal;
	int targtype;
	Mv* targu;
	Bl* targb;
	int owner;
};

ecbool FindJob(Mv* mv);
void NewJob(int jobtype, int target, int target2, int cdtype);


#endif