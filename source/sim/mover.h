











#ifndef MV_H
#define MV_H

#include "../math/camera.h"
#include "../platform.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "resources.h"
#include "../render/depthable.h"

#define BODY_LOWER	0
#define BODY_UPPER	1

struct Demand;

struct Mv
{
public:

	Dl* depth;

	ecbool on;
	int type;
	int owner;

	/*
	The draw (floating-point) position vector is used for drawing.
	*/
	Vec2f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec2i cmpos;
	Vec3f facing;	//used for tank turret
	Vec3f rotation;
	float frame[2];	//BODY_LOWER and BODY_UPPER

	std::list<Vec2i> path;
	Vec2i goal;

	int target;
	int target2;
	ecbool targetu;
	ecbool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	ecbool passive;
	Vec2i prevpos;
	int taskframe;
	ecbool pathblocked;
	int jobframes;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;
	int cargoamt;
	int cargotype;
	int cargoreq;	//req amt

	Vec2i subgoal;

	unsigned char mode;
	int pathdelay;
	unsigned __int64 lastpath;

	ecbool threadwait;

	// used for debugging - don't save to file
	ecbool collided;
	ecbool mazeavoid;
	int8_t winding;	//to avoid falling into an infinite loop/circle in path, and to know when the unit has made it around an obstacle

	signed char cdtype;	//conduit type for mode (going to construction)
	int driver;
	//short framesleft;
	int cyframes;	//cycle frames (unit cycle of consumption and work)
	int opwage;	//transport driver wage	//edit: NOT USED, set globally in Py struct
	//std::list<TransportJob> bids;	//for trucks

	std::list<Vec2s> tpath;	//tile path

	int exputil;	//expected utility on arrival

	ecbool forsale;
	int price;

	int incomerate;
	std::list<TrCycleHist> cyclehist;

	//ecbool filled;

	Mv();
	virtual ~Mv();
	void destroy();
	void fillcollider();
	void freecollider();
	void resetpath();
	ecbool hidden() const;
};

#define MOVERS	(4096)
//#define MOVERS	256

extern Mv g_mv[MOVERS];

#define UMODE_NONE					0
#define UMODE_GOBLJOB				1
#define UMODE_BLJOB					2
#define UMODE_GOCSTJOB				3	//going to construction job
#define UMODE_CSTJOB				4
#define UMODE_GOCDJOB				5	//going to conduit (construction) job
#define UMODE_CDJOB					6	//conduit (construction) job
#define UMODE_GOSHOP				7
#define UMODE_SHOPPING				8
#define UMODE_GOREST				9
#define UMODE_RESTING				10
#define	UMODE_GODRIVE				11	//going to transport job
#define UMODE_DRIVE					12	//driving transport
#define UMODE_GOSUP					13	//transporter going to supplier
#define UMODE_GODEMB				14	//transporter going to demander bl
#define UMODE_GOREFUEL				15
#define UMODE_REFUELING				16
#define UMODE_ATDEMB				17	//at demanber bl, unloading load
#define UMODE_ATSUP					18	//at supplier loading resources
#define UMODE_GODEMCD				19	//going to demander conduit
#define UMODE_ATDEMCD				20	//at demander conduit, unloading

#define TARG_NONE		-1
#define TARG_BL			1	//building
#define TARG_U			2	//unit
#define TARG_CD			3	//conduit

#ifdef RANDOM8DEBUG
extern int thatunit;
#endif

void DrawUnits();
void DrawUnit(Mv* mv, float rendz, unsigned int renderdepthtex, unsigned int renderfb);
ecbool PlaceUnit(int type, Vec2i cmpos, int owner, int *reti);
void FreeUnits();
void UpdMvs();
void ResetPath(Mv* mv);
void ResetGoal(Mv* mv);
void ResetMode(Mv* mv, ecbool chcollider = ectrue);
void ResetTarget(Mv* mv);
void StartBel(Mv* mv);
int CntMv(int mvtype);
int CntMv(int mvtype, int owner);
void CalcRot(Mv* mv);

#endif
