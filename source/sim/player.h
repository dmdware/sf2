











#ifndef PLAYER_H
#define PLAYER_H

#include "../platform.h"
//#include "../net/netconn.h"
#include "resources.h"
#include "../gui/richtext.h"
#include "../econ/institution.h"

#ifndef MATCHMAKER
#include "../gui/gui.h"
#include "../math/camera.h"
#include "selection.h"
//#include "../../script/objectscript.h"
#endif

struct PlayerColor
{
	unsigned char color[3];
	char name[32];
};

#define PLAYER_COLORS	48

extern PlayerColor g_pycols[PLAYER_COLORS];

#define PYNAME_LEN		63	//not counting null (=>64)

#ifndef MATCHMAKER

struct Py
{
public:
	ecbool on;
	ecbool ai;

	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second
	unsigned __int64 util;
	unsigned __int64 gnp;

	float color[4];
	RichText name;
	int client;	//for server

	signed char insttype;	//institution type
	signed char instin;	//institution index
	int parentst;	//parent state player index

	ecbool protectionism;
	int imtariffratio;	//import tariff ratio
	int extariffratio;	//export tariff ratio

	unsigned __int64 lastthink;	//AI simframe timer

	//TODO check if to include +1 or if thats part of pynamelen
	char username[PYNAME_LEN+1];
	char password[PYNAME_LEN+1];

#ifndef MATCHMAKER
	Py();
	~Py();
#endif
};

//#define PLAYERS 12
#define PLAYERS ARRSZ(g_pycols)
//#define PLAYERS	6	//small number of AI players so it doesn't freeze (as much)

extern Py g_py[PLAYERS];
extern int g_localP;
extern int g_playerm;
extern ecbool g_diplomacy[PLAYERS][PLAYERS];

void FreePys();
void AssocPy(int player, int client);
int NewPlayer();
int NewClPlayer();
void DefP(int ID, float red, float green, float blue, float alpha, RichText name);
void DrawPy();
void Bankrupt(int player, const char* reason);
void IniRes();

#endif

#endif
