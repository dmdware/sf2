











#ifndef CONDUIT_H
#define CONDUIT_H

#include "connectable.h"
#include "resources.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"
#include "../render/heightmap.h"
#include "../render/depthable.h"

#if 1
#define CD_ROAD		0
#define CD_POWL		1
#define CD_CRPIPE	2
#define CD_TYPES	3
#endif
//extern int g_ncdtypes;
//#define CD_TYPES	128

struct CdTile
{
public:

	Dl* depth;

	ecbool on;
	unsigned char conntype;
	ecbool finished;
	unsigned char owner;
	int conmat[RESOURCES];
	short netw;	//network
	//ecbool inaccessible;
	short transporter[RESOURCES];
	Vec2f drawpos;
	//int maxcost[RESOURCES];
	int conwage;

	ecbool selling;

	CdTile();
	~CdTile();

	//virtual unsigned char cdtype();
	int netreq(int res, unsigned char cdtype);
	void destroy();
	void allocate(unsigned char cdtype);
	ecbool checkconstruction(unsigned char cdtype);
	virtual void fillcollider();
	virtual void freecollider();
};

struct CdType
{
public:
	//ecbool on;
	unsigned int icontex;
	int conmat[RESOURCES];
	unsigned short netwoff;	//offset to network list in Bl struct
	unsigned short seloff;	//offset to selection list in Selection struct
	//TO DO elevation inclines
	//unsigned int sprite[CONNECTION_TYPES][2][INCLINES];	//0 = not finished, 1 = finished/constructed
	unsigned int splist[CONNECTION_TYPES][2];
	unsigned short maxforwincl;
	unsigned short maxsideincl;
	ecbool blconduct;	//do bl conduct this resource (also act as conduit in a network?)
	Vec2i physoff;	//offset in cm. physoff from tile(tx,ty) corner, where workers go to build it (collider).
	Vec3i drawoff;	//offset in cm. drawoff from tile(tx,ty) corner, where the depthable sorted box begins.
	CdTile* cdtiles[2];	//0 = actual placed, 1 = plan proposed
	ecbool cornerpl;	//is the conduit centered on corners or tile centers?
	char name[256];
	std::string desc;
	unsigned int lacktex;
	//reqsource indicates if being connected to the grid 
	//requires connection to a source building, or just the network.
	ecbool reqsource;
	unsigned char flags;

	CdType()
	{
		//on = ectrue;
		Zero(conmat);
		blconduct = ecfalse;
		cdtiles[0] = NULL;
		cdtiles[1] = NULL;
		cornerpl = ecfalse;
		name[0] = '\0';
		lacktex = 0;
		icontex = 0;
		reqsource = ecfalse;
	}

	~CdType()
	{
		for(int i=0; i<2; i++)
			if(cdtiles[i])
			{
				delete [] cdtiles[i];
				cdtiles[i] = NULL;
			}
	}
};

//extern CdType g_cdtype[CD_TYPES];
extern CdType g_cdtype[CD_TYPES];

inline CdTile* GetCd(unsigned char ctype, int tx, int ty, ecbool plan)
{
	CdType* ct = &g_cdtype[ctype];
	CdTile* tilesarr = ct->cdtiles[(int)plan];
	return &tilesarr[ tx + ty*(g_mapsz.x+0) ];
}

struct Bl;
struct PlaceCdPacket;

int CdWorkers(unsigned char ctype, int tx, int ty);
void DefCd(unsigned char ctype,
			const char* name,
		   const char* iconrel,
          /* */ unsigned short netwoff,
           unsigned short seloff, /* */
           ecbool blconduct,
           ecbool cornerpl,
           Vec2i physoff,
           Vec3i drawoff,
		   const char* lacktex,
		   unsigned short maxsideincl,
		   unsigned short maxforwincl,
		   ecbool reqsource,
		  unsigned char flags);
void CdDes(unsigned char ctype, const char* desc);
void CdMat(unsigned char ctype, unsigned char rtype, short ramt);
void UpdCdPlans(unsigned char ctype, char owner, Vec3i start, Vec3i end);
void ClearCdPlans(unsigned char ctype);
void ReNetw(unsigned char ctype);
void ResetNetw(unsigned char ctype);
ecbool ReNetwB(unsigned char ctype);
void MergeNetw(unsigned char ctype, int A, int B);
ecbool ReNetwTl(unsigned char ctype);
ecbool CompareCo(unsigned char ctype, CdTile* ctile, int tx, int ty);
ecbool BAdj(unsigned char ctype, int i, int tx, int ty);
ecbool CompareB(unsigned char ctype, Bl* b, CdTile* ctile);
ecbool CdLevel(unsigned char ctype, float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, ecbool plantoo);
void PlaceCd(unsigned char ctype, int ownerpy);
void PlaceCd(PlaceCdPacket* pcp);
void PlaceCd(unsigned char ctype, int tx, int ty, int owner, ecbool plan);
void Repossess(unsigned char ctype, int tx, int ty, int owner);
void DrawCd(CdTile* ctile, unsigned char x, unsigned char y, unsigned char cdtype, ecbool plan, float rendz,
			unsigned int renderdepthtex, unsigned int renderfb);
void DrawCd(unsigned char ctype);
void CdXY(unsigned char ctype, CdTile* ctile, ecbool plan, int& tx, int& ty);
void DefConn(unsigned char conduittype,
	unsigned char connectiontype,
	ecbool finished,
	const char* sprel,
	int rendtype);
void PruneCd(unsigned char ctype);
void ConnectCdAround(unsigned char ctype, int x, int y, ecbool plan);

#endif
