












#ifndef CLIENT_H
#define CLIENT_H

#include "../gui/richtext.h"
#include "../sim/player.h"
#include "netconn.h"
#include "../sim/simflow.h"
#include "../sim/simstate.h"
#include "packets.h"
#include "lockstep.h"

//a client is like a player, but concerns networking.
//a client must be a human player.
//a client controls a player slot.
struct Client
{
public:
	ecbool on;
	int player;
	RichText name;
	//unsigned char color;
	NetConn* nc;
	ecbool unresp;	//unresponsive?
	ecbool ready;
	short ping;	//for client use; server keeps it in NetConn's
	unsigned char speed;
	unsigned __int64 curnetfr;	//not so much the frame as the net turn
	int downin;	//download file byte index
	unsigned char clplat;	//client platform

	//information to send to join mid-session
	unsigned __int64 joinsimf;	//join sim frame
	SimState* joinstate;	//simstate at the start of joinsimf
	std::list<NetTurn> afterjoin;	//buffer of NetTurn commands on and after joinsimf

	Client()
	{
		nc = NULL;
		speed = SPEED_PLAY;
		joinstate = NULL;
	}
};

#define CLIENTS	PLAYERS

extern Client g_cl[CLIENTS];
extern int g_localC;
extern char g_name[GENNAMELEN];

void ResetCls();
void ResetCl(Client* c);
ecbool AddClient(NetConn* nc, RichText name, int* retci, unsigned char clplati);
void AddClient(NetConn* nc, RichText name, int ci, unsigned char clplati);

#endif
