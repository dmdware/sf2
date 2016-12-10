












#include "lockstep.h"
#include "packets.h"
#include "readpackets.h"
#include "sendpackets.h"
#include "../sim/simdef.h"
#include "../sim/simflow.h"
#include "client.h"

#ifndef MATCHMAKER
#include "../gui/layouts/messbox.h"
#endif

#if 0
std::list<PacketHeader*> g_nextcmd;
std::list<PacketHeader*> g_nextnextcmd;
ecbool g_canturn;	//only for client's use
ecbool g_canturn2;	//ditto
#else
NetTurn g_next;
NetTurn g_next2;
std::list<NetTurn> g_next3;
#endif

/*

assume 6 net frames per net turn for this example

netfr 2:	cl sends pl bl packet
netfr 4:	sv recvs pl bl packet
netfr 5:	sv checks all outgoing OldPacket's, finds no NetTurnPacket that hasn't been ack'd. this means next turn can be sent.
			sv sends out NetTurnPacket for netfr 12.
netfr 5:	cl has g_canturn==ectrue, so execs g_nextcmd, sets g_canturn=ecfalse
netfr 8:	cl recv's NetTurnPacket for netfr 12, sets g_canturn=ectrue, sends ack to sv

netfr 12:	sv hasn't recv'd ack, can't send out next NetTurnPacket for netfr 18
netfr 17:	cl can't step, the counter is at 18 but the sim state won't be updated until NetTurnPacket arrives

netfr 17:	sv finally recv's ack for NetTurnPacket for netfr 12, sends out for netfr 18


edit: what is important is not that the cl's ack the NetTurnPacket's, but that the cl's are done the prev(?) turn.
so it should be...


netfr 2:	cl sends pl bl packet
netfr 4:	sv recvs pl bl packet
netfr 5->6:	the frame before turn 1 (the next turn after turn 0): checks done if ready to advance to next turn.
			sv checks all cl's curnetfr's, finds all are at turn 0 (but also acceptable next, turn 1). 
			this means next next turn commands (2) can be sent.
			sv sends out NetTurnPacket for netfr 12 (start of turn 2).
netfr 5->6:	cl has starting set to g_canturn==ectrue, so execs g_nextcmd (empty), sets g_canturn=ecfalse
			send NetDonePacket with curnetfr=6 (turn 1)
netfr 8:	cl recv's NetTurnPacket for netfr 12 (turn 2), sets g_canturn=ectrue because it is at turn 1

netfr 11->12:	sv checks if it can advance to turn 2 (netfr 12). finds that cl is at curnetfr=6 (turn 1), so can't.
				stuck at netfr 11.
				sv is always in the same turn as all the other cl's, or behind 1 turn (because cl's have next NetTurnPacket commands).

				when advancing to the next turn, sv will exec next NetTurnPacket, send out next-next NetTurnPacket for next turn 
				(the one after advanced one), and set its next NetTurnPacket (local) to be what is in next-next, and empty next-next.

				sv always appends recv'd commands to next-next command queue.
				the next (not next-next) command queue is set in stone and has already been sent out and is just waiting to be exec'd.

			cl appends commands to next command queue if recv'd NetTurnPacket is for (current turn + 1), otherwise, next-next command
			queue. when executing, next is exec'd, emtied, and filled with next-next, as it is in turn emptied.

			what to do with variable turn length? 
			if cl exec's start of turn 1 fast forward command while sv at turn 0 (adv to 1) sends out next-next (netfr 12) NetTurnPacket,
			it won't be recognized by cl as next turn (6+FASTTURN).
			if sv exec's adv to turn 1 before filling next-next NetTurnPacket with fallacious fornetfr=12, all is okay.

*/

void UpdStep()
{
	DoneTurnPacket dtp;

	g_cmdframe ++;

	/* Not the start of a new net turn? */
	if(g_cmdframe % TurnFrames() != 0)
		return;

	if(g_netmode == NETM_SINGLE)
	{
		SP_StepTurn();
	}
	else if(g_netmode == NETM_HOST)
	{
		Sv_StepTurn();
	}
	else if(g_netmode == NETM_CLIENT)
	{
		Cl_StepTurn();

		dtp.header.type = PACKET_DONETURN;
		dtp.client = g_localC;
		dtp.curnetfr = g_cmdframe;
		SendData((char*)&dtp, sizeof(DoneTurnPacket), &g_svconn->addr, ectrue, ecfalse, g_svconn, &g_sock, 0, NULL);

		for (int ci = 0; ci < CLIENTS; ci++)
		{
			if (ci == g_localC)
				continue;

			Client* c = &g_cl[ci];

			if (!c->on)
				continue;

			c->curnetfr = dtp.curnetfr;
			break;
		}
	}
}

//how many net frames are in a net turn?
unsigned int TurnFrames()
{
	short fast = 0;
	short play = 0;
	short pause = 0;
	
	/*
	We need a separate counter for net frames and sim frames,
	because net frames might continue while sim frames are paused.
	*/

	//If anybody's paused, we can't continue.
	for(int ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_cl[ci];

		if(!c->on)
			continue;

		switch(c->speed)
		{
		case SPEED_FAST:
			fast++;
			break;
		case SPEED_PLAY:
			play++;
			break;
		case SPEED_PAUSE:
			pause++;
			break;
		default:
			break;
		}
	}

	//We can return NETTURN frames for pause, because we check for pause elsewhere.
	if(pause)
		return NETTURN;
	else if(play)
		return NETTURN;
	//EDIT: necessary for AI build commands to be executed consistently
	//if the wait is too int, the workers die
	//else if(fast)
	//	return FASTTURN;

	return NETTURN;
}

//check for net lockstep, lagging clients, etc.
//this is only for the net protocol involving packet sending;
//we check if somebody's paused (sim frame) later on in UpdSim().
ecbool CanStep()
{
	//if(g_netmode == NETM_SINGLE)
	//	return ectrue;

	unsigned int turnframes = TurnFrames();

	//Is it a decisive frame? (The start of a new net turn?) 
	//If not, we can keep going without further consideration.
	if((g_cmdframe+1) % turnframes != 0)
		return ectrue;

	//TODO notification sound of chat
	//TODO sound of mv and bl's only when in view

	/*
	We need a separate counter for net frames and sim frames,
	because net frames might continue while sim frames are paused.
	*/

	if(g_netmode == NETM_SINGLE)
		return ectrue;
	else if(g_netmode == NETM_CLIENT)
	{
		//recv'd NetTurnPacket for next turn?
		//if(g_canturn)
		//	return ectrue;

		if(g_next.canturn)
		{
			if(g_cmdframe+1 == g_next.startnetfr)
				return ectrue;
			else
			{
				char cm[256];
				sprintf(cm, "ERROR: next received net-turn is for net-frame %llu, but local client is at %llu.", g_next.startnetfr, g_cmdframe);
				RichText m = RichText(cm);
				Mess(&m);
				return ecfalse;
			}
		}

		//buggable?
		//if(g_cmdframe == 0)
		//	return ectrue;

		return ecfalse;
	}
	else if(g_netmode == NETM_HOST)
	{

#if 0
		char msg[128];
		sprintf(msg, "host check netfr%u finale sv", g_cmdframe);
		InfoMess("dt", msg);
#endif

#if 0
		//temp check for cl's
		if(g_cn.size() <= 0)
			return ecfalse;

		ecbool havecl = ecfalse;

		for(std::list<Widget*>::iterator ci=g_cn.begin(); ci!=g_cn.end(); ci++)
		{
			if(!ci->isclient)
				continue;

			if(!ci->handshook)
				continue;

			havecl = ectrue;
			break;
		}

		if(!havecl)
			return ecfalse;
#endif

		/*
		TODO
		MAINFONT16 for most of bl graphs
		fix 1011 elev road tile tricfg to not blend with terrain
		more accurate unit z/elev draw pos, accheight instead of bilerp
		accurate terrain intersection in map editor for high elevation
		maybe not C90 but C++ without constructors, or member methods, or classes mostly, and custom list and vector structs
		fixed button lines etc and windows so that they are within frames of crop
		no more crashing on failed map load
		window title bar text
		better layout for window widget layouts specialized for trucks management, bl etc
		dont use system error boxes
		make editbox frame line-based
		make player stats list scrollable
		everything scrollable
		tabbed bl view, with stocked list and graphs etc as tabs
		balanced prices and wages using algorithm to prebalance
		better AI conduit placement
		select truck on top of bl instead of bl behind it
		use distance to supplier or store based on path once sim speed reaches x600+ with ev queue
		calc movement up slope accurately and affect speed
		up-down incline for vehicles, sprite angles
		DrawOv in map ed doesn't work with borders showing
		get it loading in under 5 seconds
		borders stopped working any other py besides 0 in map ed fix fixed
		clean up map ed code
		permanent employment?
		std::list<Widget*>::iterator-generate all four sides of road tiles combos from model/3d asset and render tiles sprites in game
		pay to demolish bl, labour
		*/

#if 0
		//check for non-ack'd NetTurnPackets in outgoing OldPacket list
		for(std::list<Widget*>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
		{
			NetConn* nc = Match(&pi->addr);

			if(!nc)
				continue;

			if(!nc->handshook)
				continue;

			//if(nc->ctype != NETM_CLIENT)
			//	continue;

			if(!nc->isclient)
				continue;

			//also check if client is spectator or progresssing through the history of the session TO DO

			if(((PacketHeader*)pi->buffer)->type != PACKET_NETTURN)
				continue;

			return ecfalse;	//ack not recv'd
		}
#endif

		// check for cl's being up to current turn
		for(int ci=0; ci<CLIENTS; ci++)
			//for(int pi=0; pi<1; pi++)
		{
			Client* c = &g_cl[ci];

			//InfoMess("a", "all py's passed netfr c0");

			if(!c->on)
				continue;

			//InfoMess("a", "all py's passed netfr c1");

			//if(py->ai)
			//	continue;

#if 0
			char msg1[128];
			sprintf(msg1, "all py's passed netfr c2, clnetfr%u", (py->fornetfr / NETTURN) * NETTURN);
			InfoMess("a", msg1);
#endif

			//If this the local player, update we don't get a packet for his net frame,
			//and must update it from g_cmdframe.
			if(ci == g_localC)
			{
				c->curnetfr = g_cmdframe;
				continue;
			}

			//Not in the same net turn?
			//if( (py->fornetfr / turnframes) != (g_cmdframe / turnframes) )
			//Is the next turn (that we are about to enter) not the same net turn?
			if( (c->curnetfr / turnframes) != ((g_cmdframe + 1) / turnframes) &&
				(c->curnetfr / turnframes) != ((g_cmdframe + 1) / turnframes) )
			{
#if 0
				char msg[128];
				sprintf(msg, "py%d (%u) not up to %u", pi, py->fornetfr, g_cmdframe);
				InfoMess(msg, msg);
#endif

				return ecfalse;
			}

#if 0
			InfoMess("a", "all py's passed netfr finale");

			char msg[128];
			sprintf(msg, "netfr%u finale sv", g_cmdframe);
			InfoMess("dt", msg);
#endif
		}

		return ectrue;
	}

	return ecfalse;
}

void ExecCmds(std::list<PacketHeader*>* q)
{
	//InfoMess("e", "e");

	for(std::list<PacketHeader*>::iterator pi=q->begin(); pi!=q->end(); pi++)
	{
#if 0
		if(((PacketHeader*)*pi)->type == PACKET_PLACEBL)
			InfoMess("plp", "plp");
#endif

#if 0
		char msg[128];
		sprintf(msg, "exec t%d", ((PacketHeader*)*pi)->type);
		InfoMess("exe", msg);
#endif

		//TranslatePacket((char*)*pi, -1, ecfalse, NULL, NULL);
		PacketSwitch((*pi)->type, (char*)*pi, -1, NULL, NULL, NULL);
	}
}

void AppendCmd(std::list<PacketHeader*>* q, PacketHeader* p, short sz)
{
	unsigned char* newp = (unsigned char*)malloc(sz);
	if(!newp) OUTOFMEM();
	memcpy(newp, p, sz);
	q->push_back((PacketHeader*)newp);
}

//lockstep packet size (only packets used in lockstep)
//must get a copy of the packet to calc size ("pack")
int LockPackSz(int packtype, PacketHeader* pack)
{
	int sz = 0;

	/*
	Must fill out all of these function switches with
	packets that will be placed in NetTurnPacket's.
	Some packets, like move orders, are of variable length,
	depending on number of unit selected etc.
	*/

	switch(pack->type)
	{
	//some packets don't get bundled in NetTurnPacket's, but they're here anyway
	case PACKET_ACKNOWLEDGMENT:
		return sizeof(AckPacket);
	case PACKET_CONNECT:
		return sizeof(ConnectPacket);
	case PACKET_DISCONNECT:
		return sizeof(DisconnectPacket);
	case PACKET_PLACEBL:
		return sizeof(PlaceBlPacket);
		//InfoMess("bundl", "placebl detected in AppendCmds");
	case PACKET_CHVAL:
		return sizeof(ChValPacket);
	case PACKET_ORDERMAN:
		return sizeof(OrderManPacket);
	case PACKET_CLSTATE:
		return sizeof(ClStatePacket);
	case PACKET_MOVEORDER:	//variable length!
		sz = sizeof(MoveOrderPacket) + 
			sizeof(unsigned short) * ((MoveOrderPacket*)pack)->nunits;
		return sz;
	case PACKET_PLACECD:	//variable length!
		sz = sizeof(PlaceCdPacket) +
			sizeof(Vec2uc) * ((PlaceCdPacket*)pack)->ntiles;
		return sz;
	case PACKET_SETSALEPROP:
		sz = sizeof(SetSalePropPacket);
		return sz;
	case PACKET_BUYPROP:
		sz = sizeof(BuyPropPacket);
		return sz;
	case PACKET_DEMOLPROP:
		sz = sizeof(DemolPropPacket);
		return sz;
	default:
		return 0;
	}

	return 0;
}

void AppendCmds(std::list<PacketHeader*>* q, NetTurnPacket* ntp)
{
	unsigned char* pstart = ((unsigned char*)ntp)+sizeof(NetTurnPacket);
	unsigned char* pload = pstart;
	unsigned char* newp;

#if 0
	if(ntp->loadsz > 0)
		InfoMess("app", "appcmd");
#endif

	//go through all packets, add each to queue
	while( (int)(pload-pstart) < ntp->loadsz )
	{
		PacketHeader* ph = (PacketHeader*)pload;

#if 0
		char msg[128];
		sprintf(msg, "load ph->type=%d", ph->type);
		InfoMess("appcmd", msg);
#endif

		int sz = LockPackSz(ph->type, ph);
		newp = (unsigned char*)malloc(sz);
		if(!newp) OUTOFMEM();
		memcpy(newp, ph, sz);
		q->push_back((PacketHeader*)newp);
		newp = NULL;
		pload += sz;
	}
}

void FillNetTurnPacket(NetTurnPacket** pp, std::list<PacketHeader*>* q)
{
	int loadsz = 0;

	//go through all packets, calc size
	for(std::list<PacketHeader*>::iterator pi=q->begin(); pi!=q->end(); pi++)
		loadsz += LockPackSz((*pi)->type, *pi);

	*pp = (NetTurnPacket*)malloc(sizeof(NetTurnPacket)+loadsz);

	NetTurnPacket* ntp = *pp;
	ntp->header.type = PACKET_NETTURN;
	unsigned int turnframes = TurnFrames();
	ntp->fornetfr = (g_cmdframe / turnframes + 1) * turnframes;

	//0 is a special case
	//iph fix 3
	if (ntp->fornetfr < g_cmdframe)
		ntp->fornetfr = 0;

	ntp->loadsz = loadsz;

	unsigned char* pload = ((unsigned char*)ntp)+sizeof(NetTurnPacket);

	//go through all packets, add each to load
	for(std::list<PacketHeader*>::iterator pi=q->begin(); pi!=q->end(); pi++)
	{
		int packsz = LockPackSz((*pi)->type, *pi);
		memcpy(pload, *pi, packsz);
		pload += packsz;
	}

	//TODO order mv moves lockstep
}

#if 0	//no more
//internal use
void Cl_StepTurn(NetTurnPacket* ntp)
{
	ExecCmds(&g_nextcmd);
	FreeCmds(&g_nextcmd);
	AppendCmds(&g_nextcmd, ntp);
	g_canturn = ecfalse;
}
#endif

void Cl_StepTurn()
{
	//InfoMess("c", "cl st");
	//ExecCmds(&g_nextcmd);
	//FreeCmds(&g_nextcmd);
	//g_nextcmd = g_nextnextcmd;	//pointers!
	//g_nextnextcmd.clear();	//pointers!
	//g_canturn = g_canturn2;
	//g_canturn2 = ecfalse;
	ExecCmds(&g_next.cmds);
	FreeCmds(&g_next.cmds);
	g_next = g_next2;
	g_next2.cmds.clear();
	g_next2.canturn = ecfalse;

	if(g_next3.size() > 0)
	{
		g_next2 = *g_next3.begin();
		g_next3.pop_front();
	}
}

//single player
void SP_StepTurn()
{
#if 0
	ExecCmds(&g_nextcmd);
	FreeCmds(&g_nextcmd);
	g_nextcmd = g_nextnextcmd;	//pointers!
	//FreeCmds(&g_nextnextcmd);
	g_nextnextcmd.clear();	//pointers!
	g_canturn = ectrue;
	g_canturn2 = ecfalse;
#else
	ExecCmds(&g_next.cmds);
	FreeCmds(&g_next.cmds);
	g_next = g_next2;
	g_next.canturn = ectrue;
	g_next2.cmds.clear();
	g_next2.canturn = ecfalse;

	if(g_next3.size() > 0)
	{
		g_next2 = *g_next3.begin();
		g_next3.pop_front();
	}
#endif
}

/*
prev NetTurnPacket must have been ack'd by all clients before proceeding
and all cl's must be up to prev turn.
Since this is called from UpdStep(), that means g_cmdframe % turnframes == 0,
i.e., this frame is the start of a new net turn. So, g_nextcmd is for the current
turn, and g_nextnextcmd is for the next turn.
Sv must be behind the clients or with them; if sv is ahead and sends NetTurnPacket
for next turn, that will be two turns ahead.
*/
void Sv_StepTurn()
{
	//Must exec g_nextcmd BEFORE filling next-next NetTurnPacket,
	//because turn length might vary. Do everything IN ORDER.
	//InfoMess("c", "sv st");
	ExecCmds(&g_next.cmds);
	FreeCmds(&g_next.cmds);
	NetTurnPacket *ntp;
	FillNetTurnPacket(&ntp, &g_next2.cmds);
	SendAll((char*)ntp, sizeof(NetTurnPacket) + ntp->loadsz, ectrue, ecfalse, NULL, 0);

#if 0
	if(g_nextnextcmd.size() > 0)
		InfoMess("sv", "sv has local cmds");

	if(ntp->loadsz > 0)
		InfoMess("sv", "sv has load");

	if(g_nextcmd.size() > 0)
		InfoMess("sv", "sv has cmds");
#endif

	free(ntp);

#if 0
	g_nextcmd = g_nextnextcmd;	//pointers!
	//FreeCmds(&g_nextnextcmd);
	g_nextnextcmd.clear();	//pointers!
#else
	g_next = g_next2;
	g_next.canturn = ectrue;
	g_next2.cmds.clear();
	g_next2.canturn = ecfalse;

	if(g_next3.size() > 0)
	{
		g_next2 = *g_next3.begin();
		g_next3.pop_front();
	}
#endif
}

void AppendCmd(PacketHeader* p, int sz, std::list<PacketHeader*>* q)
{
	PacketHeader* p2 = (PacketHeader*)malloc(sz);
	if(!p2) OUTOFMEM();
	memcpy(p2, p, sz);
	q->push_back(p2);
}

void FreeCmds(std::list<PacketHeader*>* q)
{
	std::list<PacketHeader*>::iterator it = q->begin();

	while(it != q->end())
	{
		free(*it);
		it = q->erase(it);
	}
}

/*
Use for all commands that are in lockstep.
E.g., move orders, place building action, changing values.
Note: this command only adds commands locally;
If this is the server, you need to also SendAll the command.
Edit: actually no, the g_nextnextcmd gets wrapped in a NetTurnPacket
and gets sent to all the clients.
*/
void LockCmd(PacketHeader* pack)
{
	short sz = LockPackSz(pack->type, pack);

#ifndef MATCHMAKER
	if(g_netmode == NETM_HOST)
	{
		std::list<PacketHeader*>* q;

		//if(!g_next.canturn)
		//	q = &g_next.cmds;
		//else 
		if(!g_next2.canturn)
			q = &g_next2.cmds;
		else
		{
			NetTurn nt;
			nt.canturn = ectrue;
			g_next3.push_back(nt);
			q = &g_next3.rbegin()->cmds;
		}

		//AppendCmd(&g_nextnextcmd, (PacketHeader*)pack, sz);
		AppendCmd(q, (PacketHeader*)pack, sz);

#if 0
		NetConn* nc = &*g_cn.begin();

		if(nc)
			SendData((char*)&pbp, sizeof(PlaceBlPacket), &nc->addr, ectrue, nc, &g_sock);
#endif
	}
	else if(g_netmode == NETM_CLIENT)
	{
		NetConn* nc = g_svconn;

		if(nc)
			SendData((char*)pack, sz, &nc->addr, ectrue, ecfalse, nc, &g_sock, 0, NULL);
	}
	else if(g_netmode == NETM_SINGLE)
	{
#if 0
		char msg[128];
		sprintf(msg, "send %d at %d,%d", pbp.btype, pbp.tpos.x, pbp.tpos.y);
		InfoMess("polk", msg);
#endif
		
		std::list<PacketHeader*>* q;

		//if(!g_next.canturn)
		//	q = &g_next.cmds;
		//else 
		if(!g_next2.canturn)
			q = &g_next2.cmds;
		else
		{
			NetTurn nt;
			nt.canturn = ectrue;
			g_next3.push_back(nt);
			q = &g_next3.rbegin()->cmds;
		}

		//AppendCmd(&g_nextcmd, (PacketHeader*)pack, sz);
		//AppendCmd(&g_nextnextcmd, (PacketHeader*)pack, sz);
		AppendCmd(q, (PacketHeader*)pack, sz);
	}
#endif
}

