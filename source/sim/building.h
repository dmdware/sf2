











#ifndef BUILDING_H
#define BUILDING_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "bltype.h"
#include "../render/vertexarray.h"
#include "mvtype.h"
#include "../render/depthable.h"

struct VertexArray;

/* mv manufacturing job */
struct ManufJob
{
	char mvtype;
	int owner;
};

typedef union CdNetw
{
	List l; /* short */
	short i;
};

struct Bl
{
	Dl* depth;

	ecbool on;
	int type;
	int owner;

	Vec2i tpos;	//position in tiles
	Vec2f drawpos;	//drawing position in world pixels

	ecbool finished;
	float frame;

	CdNetw netw[CD_TYPES];

	/* TODO dynamic baskets/bundles */
	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmCntr emitterco[MAX_B_EMITTERS];

	int conmat[RESOURCES];
	ecbool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property
	ecbool forsale;	//is this property for sale?
	ecbool demolish;	//was a demolition ordered?
	//TODO only one stocked[] and no conmat[]. set stocked[] to 0 after construction finishes, and after demolition ordered.

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned __int64 lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int manufprc[MV_TYPES];
	std::list<ManufJob> manufjob;
	short transporter[RESOURCES];

	int hp;
	
	int varcost[RESOURCES];	//variable cost for input resource ri
	int fixcost;	/* fixed cost for transport */

	List cyclehist;	/* <CycleHist> */

	ecbool demolition;

	ecbool excin(int rtype);	//excess input resource right now?
	ecbool metout();	//met production target for now?
	int netreq(int rtype);	//how much of an input is still required to meet production target
	ecbool hasworker(int ui);
	ecbool tryprod();
	ecbool trymanuf();
	int maxprod();
	void adjcaps();
	void spawn(int mvtype, int uowner);
	void morecap(int rtype, int amt);
	void lesscap(int rtype, int amt);
	void getethereal();
	void getethereal(int rtype, int amt);

	void destroy();
	void fillcollider();
	void freecollider();
	void allocres();
	ecbool checkconstruction();
	Bl();
	~Bl();
};

//#define BUILDINGS	256
//#define BUILDINGS	64
//#define BUILDINGS (MAX_MAP*MAX_MAP)
#define BUILDINGS	1024

extern Bl g_bl[BUILDINGS];
struct Mv;

extern std::list<unsigned short> g_onbl;

int NewBl();
void FreeBls();
void DrawBl();
void DrawBl(Bl* b, float rendz, unsigned int renderdepthtex, unsigned int renderfb);
void DrawBl2(Bl* b, float rendz, unsigned int renderdepthtex, unsigned int renderfb);
void UpdBls();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Bl* b);
float CompletPct(int* cost, int* current);
void RemWorker(Mv* w);
void SupAdjCaps(unsigned char ri, int supbi);
void DemAdjCaps(unsigned char ri, int dembi);
void RemShopper(Mv* w);

#endif
