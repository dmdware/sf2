














#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "simdef.h"

struct Mv;
struct Bl;

#if 0
struct TransportJob
{
public:
	int targtype;
	int target;
	int target2;
	std::list<Vec2i> path;
	int pathlen;
	int restype;
	int resamt;
	int fuelcost;
	int driverwagepaid;
	int netprofit;	// truck profit
	int transporter;
	int totalcosttoclient;	// transport fee + resource cost
	int clientcharge;	// transport fee
	int supplier;
    
	TransportJob()
	{
		pathlen = 0;
		totalcosttoclient = 0;
		clientcharge = 0;
		driverwagepaid = 0;
		netprofit = 0;
		fuelcost = 0;
		resamt = 0;
	}
};
#else

//job opportunity
struct TransportJob
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
	Mv* thisu;
	Mv* targu;
	Bl* targb;
	int suputil;
	int supbi;
	Vec2i supcmpos;
	short truckui;
	Bl* potsupb;
};

#endif

struct ResSource
{
public:
	int supplier;
	int amt;
};

//extern vector<int> g_freetrucks;

////#define TRANSPORT_DEBUG	//chat output debug messages

void MgTrips();

#endif
