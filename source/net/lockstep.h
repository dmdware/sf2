












#ifndef LOCKSTEP_H
#define LOCKSTEP_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

#define NETTURN	(200/SIM_FRAME_RATE)	//200 ms delay between net turns
//#define NETTURN		(1000/SIM_FRAME_RATE)	//1000 ms delay between net turns
#define FASTTURN	(NETTURN*10)			//a fast-forward net turn is 10x normal net turn

struct NetTurn 
{
public:
	unsigned __int64 startnetfr;
	std::list<PacketHeader*> cmds;
	ecbool canturn;	//for client use only

	NetTurn()
	{
		canturn = ecfalse;
	}
};

#if 0
extern std::list<PacketHeader*> g_nextcmd;
extern std::list<PacketHeader*> g_nextnextcmd;
extern ecbool g_canturn;
extern ecbool g_canturn2;
#else
extern NetTurn g_next;
extern NetTurn g_next2;
extern std::list<NetTurn> g_next3;
#endif

void Cl_StepTurn();
void Sv_StepTurn();
void SP_StepTurn();
void FreeCmds(std::list<PacketHeader*>* q);
void AppendCmds(std::list<PacketHeader*>* q, NetTurnPacket* ntp);
void AppendCmd(std::list<PacketHeader*>* q, PacketHeader* p, short sz);
ecbool CanStep();
void UpdStep();
void LockCmd(PacketHeader* pack);
unsigned int TurnFrames();
void FillNetTurnPacket(NetTurnPacket** pp, std::list<PacketHeader*>* q);

#endif