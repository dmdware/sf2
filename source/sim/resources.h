











#ifndef RESOURCES_H
#define RESOURCES_H

#include "../platform.h"

//TODO MAX_ICONS 256 mod
struct Resource
{
public:
	char icon;
	ecbool physical;
	ecbool capacity;
	ecbool global;
	std::string name;	//TODO change to char[]?
	float rgba[4];
	std::string depositn;	//TODO change to char[]?
	int conduit;
	std::string unit;	//measuring unit
	unsigned int halflife;
};

//conduit
#define CD_NONE		-1
#define CD_ROAD		0
#define CD_POWL		1
#define	CD_CRPIPE		2

#define RES_NONE			-1
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_IRONORE			6
#define RES_METAL			7
#define RES_STONE			8
#define RES_CEMENT			9
#define RES_COAL			10
#define RES_URANIUM			11
#define RES_PRODUCTION		12
#define RES_CRUDEOIL		13
#define RES_WSFUEL			14
#define RES_RETFUEL			15
#define RES_ENERGY			16
#define RESOURCES			17
//#define RES_ELECTRONICS		6
extern Resource g_resource[RESOURCES];

//TODO remove electronics plant to avoid crash

//TODO make Basket allocate *r = x RESOURCES for modding
//MAX_RESOURCES 256
struct Basket
{
public:
	int r[RESOURCES];

	int& operator[](const int i)
	{
		return r[i];
	}
};

//TODO implement Bundle
struct Bundle
{
public:
	unsigned char res;
	int amt;
};

//capacity supply (e.g. electricity, water pressure)
struct CapSup
{
public:
	unsigned char rtype;
	int amt;
	int src;
	int dst;
};

void DefR(int resi, 
		  const char* n, 
		  const char* depn, 
		  int iconindex, 
		  ecbool phys, ecbool cap, ecbool glob, 
		  float r, float g, float b, float a, 
		  int conduit, const char* unit, unsigned int halflife);
void Zero(int *b);
void UpdDecay();
ecbool ResB(int building, int res);
ecbool TrySub(const int* cost, int* universal, int* stock, int* local, int* netch, int* insufres);

struct CycleHist
{
public:
	int prod[RESOURCES];	//earnings and production
	int cons[RESOURCES];	//expenses and consumption
	int price[RESOURCES];
	int wage;
	
	CycleHist()
	{
		reset();
	}
	
	void reset()
	{
		Zero(prod);
		Zero(cons);
		Zero(price);
		wage = -1;
	}
};

struct TrCycleHist
{
public:
	int earn;	//fees collected
	int pay;	//wage paid
	int trprice;	//transport fee
	int opwage;	//driver wage
	int jobsgiven;
	int jobsfini;
	
	TrCycleHist()
	{
		reset();
	}
	
	void reset()
	{
		earn = 0;
		pay = 0;
		trprice = 0;
		opwage = 0;
		jobsgiven = 0;
		jobsfini = 0;
	}
};

#endif
