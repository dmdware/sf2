












#ifndef NETCONN_H
#define NETCONN_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

extern UDPsocket g_sock;
extern ecbool g_sentsvinfo;
//extern ecbool g_needsvlist;
//extern ecbool g_reqdsvlist;

extern std::list<OldPacket> g_outgo;
extern std::list<OldPacket> g_recv;

struct NetConn
{
public:
	unsigned short nextsendack;
	unsigned short lastrecvack;
	ecbool handshook;
	IPaddress addr;
	//TODO change these to flags
	ecbool isclient;	//is this a hosted game's client? or for MATCHMAKER, is this somebody requesting sv list?
	ecbool isourhost;	//is this the currently joined game's host? cannot be a host from a server list or something. for MATCHMAKER, it can be a host getting added to sv list.
	ecbool ismatch;	//matchmaker?
	ecbool ishostinfo;	//is this a host we're just getting info from for our sv list?
	//ecbool isunresponsive;
	unsigned __int64 lastsent;
	unsigned __int64 lastrecv;
	short client;
	float ping;
	ecbool closed;
	ecbool disconnecting;
	unsigned short dock;

	void expirein(int millis);

#ifdef MATCHMAKER
	int svlistoff;	//offset in server list, sending a few at a time
	SendSvInfo svinfo;
#endif
	//void (*chcallback)(NetConn* nc, ecbool success);	//connection state change callback - did we connect successfully or time out?

	NetConn()
	{
		client = -1;
		handshook = ecfalse;
		nextsendack = 0;
		//important - reply ConnectPacket with ack=0 will be
		//ignored as copy (even though it is original) if new NetConn's lastrecvack=0.
		lastrecvack = USHRT_MAX;
		isclient = ecfalse;
		isourhost = ecfalse;
		ismatch = ecfalse;
		ishostinfo = ecfalse;
		//isunresponsive = ecfalse;
		lastrecv = GetTicks();
		lastsent = GetTicks();
		//chcallback = NULL;
#ifdef MATCHMAKER
		svlistoff = -1;
#endif
		ping = 1;
		closed = ecfalse;
		dock = 0;
	}
};

extern std::list<NetConn> g_cn;
extern NetConn* g_svconn;
extern NetConn* g_mmconn;	//matchmaker
extern unsigned short g_dock;

NetConn* Match(IPaddress* addr, unsigned short dock);
void EndSess(ecbool switchmode=ectrue);	//used to clear cmd queues
void BegSess(ecbool switchmode=ectrue);
NetConn* Connect(const char* addrstr, unsigned short port, ecbool ismatch, ecbool isourhost, ecbool isclient, ecbool ishostinfo);
NetConn* Connect(IPaddress* ip, ecbool ismatch, ecbool isourhost, ecbool isclient, ecbool ishostinfo);
void CheckConns();
void FlushPrev(IPaddress* from, unsigned short dock);
void KeepAlive();
void OpenSock();
void Disconnect(NetConn* nc);

#endif
