











#include "../platform.h"
#include "net.h"
#include "netconn.h"
#include "readpackets.h"
#include "sendpackets.h"
#include "packets.h"
#include "../utils.h"
#include "parity.h"
#ifndef MATCHMAKER
#include "../sim/player.h"
#include "../gui/gui.h"
#include "../gui/widgets/spez/svlist.h"
#endif

//unsigned __int64 g_lastS;  //last sent
//unsigned __int64 g_lastR;  //last recieved

#ifndef MATCHMAKER
int g_netmode = NETM_SINGLE;
#else
int g_netmode = NETM_HOST;
unsigned int g_transmitted = 0;
#endif

char g_mapname[MAPNAME_LEN+1] = "";
char g_svname[SVNAME_LEN+1] = "";
unsigned int g_mapcheck;
int g_mapfsz = 0;
ecbool g_lanonly = ecfalse;
double g_sendrate = 0;

#ifndef MATCHMAKER
unsigned int g_transmitted = 0;
#endif

unsigned short NextAck(unsigned short ack)
{
	//if(ack == UINT_MAX)
	//	ack = 0;
	//else
	//	ack ++;

	ack++;

	return ack;
}

unsigned short PrevAck(unsigned short ack)
{
	//if(ack == 0)
	//	ack = UINT_MAX;
	//else
	//	ack --;

	ack--;

	return ack;
}

//PastAck(header->ack, nc->lastrecvack)

ecbool PastAck(unsigned short test, unsigned short current)
{
	return ((current >= test) && (current - test <= USHRT_MAX/2))
	       || ((test > current) && (test - current > USHRT_MAX/2));
}

ecbool PastFr(unsigned __int64 test, unsigned __int64 current)
{
	return ((current >= test) && (current - test <= (0xffffffffffffffff)/2))
	       || ((test > current) && (test - current > (0xffffffffffffffff)/2));
}


#ifndef MATCHMAKER
//add us to sv list of matchmaker,
//check sv hosts for info
//(two different things, one for hosts, one for clients).
void CheckAddSv()
{
	//check sv hosts for info
#if 0
	for(std::list<Widget*>::iterator cit=g_cn.begin(); cit!=g_cn.end(); cit++)
	{
		NetConn* nc = &*cit;

		if(!nc->ishostinfo)
			continue;
	}
#else
	//check sv hosts for info
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	SvList* v = (SvList*)gui->get("sv list");

	if(v)
	{
		std::list<SvInfo>::iterator sit = v->svlist.begin();
		while(sit != v->svlist.end())
		{
			if(sit->replied)
			{
				sit++;
				continue;
			}

			NetConn* nc = Match(&sit->addr, sit->dock);

			if(!nc)
			{
				//timeout?
				if(v->selsv == &*sit)
					v->selsv = NULL;

				sit = v->svlist.erase(sit);
				continue;
			}
			//update ping
			else
			{
				char pingstr[16];
				sprintf(pingstr, "%d", (int)nc->ping);
				sit->ping = nc->ping;
				sit->pingrt = RichText(pingstr);
			}

			sit++;
		}
	}
#endif

	//add us to sv list of matchmaker
	if(g_netmode == NETM_HOST && ( !g_sentsvinfo || !g_mmconn || !g_mmconn->handshook) && !g_lanonly)
	//if(ecfalse)	//disabled, since no matchmaker server up
	{
#if 0
		Connect(SV_ADDR, PORT, ectrue, ecfalse, ecfalse, ecfalse);
		AddSvPacket asp;
		asp.header.type = PACKET_ADDSV;
		SendData((char*)&asp, sizeof(AddSvPacket), &g_mmconn->addr, ectrue, ecfalse, g_mmconn, &g_sock, 1, NULL);
		g_sentsvinfo = ectrue;
#else
		if(g_mmconn)
		{
			//InfoMess("hsse?", "hs asv?");

			//check if we've got an outgoing packet in queue
			if(g_mmconn->handshook)
			{
				//InfoMess("hsse", "hs asv");

				ecbool outgoing = ecfalse;

				for(std::list<OldPacket>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
				{
					OldPacket* op = (OldPacket*)&*pi;
					PacketHeader* ph = (PacketHeader*)op->buffer;

					//if(memcmp(&op->addr, &g_mmconn->addr, sizeof(IPaddress)) != 0)
					if(!Same(&op->addr, ph->recvdock, &g_mmconn->addr, g_mmconn->dock) &&
						!Same(&op->addr, ph->recvdock, &g_mmconn->addr, 0))
						continue;

					if(ph->type != PACKET_ADDSV)
						continue;

					op->last -= 6000;	//send now!
					outgoing = ectrue;

					Log("outgoing");

					break;
				}

				if(!outgoing)
				{
					
					Log("!outgoing");

					//InfoMess("se", "sen asv");
					AddSvPacket asp;
					asp.header.type = PACKET_ADDSV;
#if 0
					//actually, this goes into a SvInfoPacket
					memcpy(asp.svinfo.mapname, g_mapname, sizeof(char)*MAPNAME_LEN);
					memcpy(asp.svinfo.svname, g_svname, sizeof(char)*SVNAME_LEN);
					asp.svinfo.nplayers = 1;	//TO DO
					memset(&asp.svinfo.addr, 0, sizeof(IPaddress));
#endif
					SendData((char*)&asp, sizeof(AddSvPacket), &g_mmconn->addr, ectrue, ecfalse, g_mmconn, &g_sock, 1, NULL);
					g_sentsvinfo = ectrue;
				}
			}
			// !g_mmconn->handshook
			else
			{
#if 1	//we've got this connection so we've already sent a connect packet. code below only creates
		//problems as multiple connect packets get ack'd and resent with new acks before the connectpacket back arrives.
				ecbool connecting = ecfalse;

				for(std::list<OldPacket>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
				{
					OldPacket* op = (OldPacket*)&*pi;
					PacketHeader* ph = (PacketHeader*)op->buffer;

					//if(memcmp(&op->addr, &g_mmconn->addr, sizeof(IPaddress)) != 0)
					if(!Same(&op->addr, ph->recvdock, &g_mmconn->addr, g_mmconn->dock) &&
						!Same(&op->addr, ph->recvdock, &g_mmconn->addr, 0))
						continue;

					if(ph->type != PACKET_CONNECT)
						continue;
					
					//op->last -= 6000;	//send now!
					
					Log("connecting lastsentd=%llu", (GetTicks()-op->last));

					connecting = ectrue;
					break;
				}

#if 0
				//disconnect sometimes, i find that sometimes it gets stuck here when another instance on the same computer is involved (might effect people behind nat)
				//what is that? why is it only one port effected? is it the VPS host blocking that machine from flooding?
				if(connecting && rand()%600 == 1)
				{
					Log("rand disc");
					Disconnect(g_mmconn);
				}
#endif

				if(!connecting)
				{
					Log("!connecting");

					Connect(SV_ADDR, PORT, ectrue, ecfalse, ecfalse, ecfalse);
					Log("con mm 1 g_outgo.size() = %d\r\n", (int)g_outgo.size());
				}
#endif
			}
		}
		// !g_mmconn
		else
		{
			Log("!g_mmconn");

			Connect(SV_ADDR, PORT, ectrue, ecfalse, ecfalse, ecfalse);
			Log("con mm 2");
		}
#endif
	}
}
#endif

#ifdef MATCHMAKER
#define SV_LIST_RATE		5	//limit 127

//send out server lists
void SendSvs()
{
	//check which clients need to send to
	for(std::list<Widget*>::iterator ci=g_cn.begin(); ci!=g_cn.end(); ci++)
	{

	//Log("send sv l 1");
	//

		//if we're not sending to this client, or
		//if we've already sent all our sv's.
		if(ci->svlistoff < 0)
			continue;

		//if it's not a client.
		if(!ci->isclient)
			continue;

		//how many we're already sending (outgoing) to this cl.
		int sending = 0;

		//have to go through all outgoing packets to match them up.
		for(std::list<Widget*>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
		{
			PacketHeader* ph = (PacketHeader*)pi->buffer;

			//same address as this client?
			if(!Same(&pi->addr, ph->recvdock, &ci->addr, ci->dock) &&
				!Same(&pi->addr, ph->recvdock, &ci->addr, 0))
				continue;

			//an sv addr packet?
			if(ph->type != PACKET_SVADDR)
				continue;

			sending++;
		}

		signed char tosend = SV_LIST_RATE - sending;

		if(tosend <= 0)
			continue;

		std::list<Widget*>::iterator ait = g_cn.begin();	//address iterator
		int ain = 0;	//address index

		while(ectrue)
		{
			//if we've reached the end of our connections,
			//we've sent all of our sv list
			if(ait == g_cn.end())
			{
				ci->svlistoff = -1;
				tosend = 0;
				break;
			}

			//if this isn't a game host, increment the iterator
			//and go to next.
			if(!ait->ishostinfo)
			{
				ait++;
				continue;
			}

			//if we've reached where we've left off on last send,
			//send this sv addr.
			if(ain >= ci->svlistoff)
			{
				SvAddrPacket sap;
				sap.header.type = PACKET_SVADDR;
				memcpy(&sap.addr, &ait->addr, sizeof(IPaddress));
				SendData((char*)&sap, sizeof(SvAddrPacket), &ci->addr, ectrue, ecfalse, &*ci, &g_sock, 0, NULL);
				tosend--;
				ci->svlistoff = ain;
#if 0
	Log("send sv l 2 g_cn.size()="<<g_cn.size());
	

	unsigned int svipaddr = SDL_SwapBE32(ait->addr.host);
	unsigned short svport = SDL_SwapBE16(ait->addr.port);
	unsigned int svipaddr2 = SDL_SwapBE32(ci->addr.host);
	unsigned short svport2 = SDL_SwapBE16(ci->addr.port);

	Log("send to "<<svipaddr2<<":"<<svport2<<" about sv "<<svipaddr<<":"<<svport);
#endif
			}

			//increment index and iterator, because this is a game host.
			ain++;
			ait++;
		}
	}
}
#endif

#ifndef MATCHMAKER
//check getting sv list info's
void CheckGetSvs()
{
	if(!g_reqsvlist)
		return;

	//if already getting next host
	if(g_reqdnexthost)
		return;

	//check if it's time to queue up next sv addr
	//if the last reply from any of the prev was 200 ms
	//and we have < NETCONN_TIMEOUT/RESEND_DELAY or <50 connections
	const int maxconn = imax(NETCONN_TIMEOUT/RESEND_DELAY, 50);
	int numconn = 0;
	unsigned __int64 latest = 0;

	for(std::list<NetConn>::iterator sit=g_cn.begin(); sit!=g_cn.end(); sit++)
	{
		if(!sit->ishostinfo)
			continue;

		numconn ++;

		NetConn* nc = Match(&sit->addr, sit->dock);

		if(!nc)
			continue;

		if(nc->lastrecv > latest)
			latest = nc->lastrecv;
	}

	//if there's more hosts than limit
	//or if latest reply was <200ms ago
	if(numconn >= maxconn &&
		GetTicks() - latest <= RESEND_DELAY)
		return;

	if(!g_mmconn)
	{
		//need to be connected to matchmaker
		Connect(SV_ADDR, PORT, ectrue, ecfalse, ecfalse, ecfalse);
		return;
	}

	if(!g_mmconn->handshook)
		return;

	SendNextHostPacket snhp;
	snhp.header.type = PACKET_SENDNEXTHOST;
	SendData((char*)&snhp, sizeof(SendNextHostPacket), &g_mmconn->addr, ectrue, ecfalse, g_mmconn, &g_sock, 0, NULL);

	//prevent us from sending out more nexthost requests
	//until we get the current one.
	g_reqdnexthost = ectrue;
}

#if 0	//Done in CheckConns and TranslatePacket now
//check for unresponsive clients
void CheckCls()
{
	if(g_netmode != NETM_HOST)
		return;

	const unsigned __int64 now = GetTicks();

	for(std::list<Widget*>::iterator cit=g_cn.begin(); cit!=g_cn.end(); cit++)
	{
		if(!cit->isclient)
			continue;

		if(now - cit->lastrecv < NETCONN_UNRESP)
			continue;

		//unresponsive, what to do?

		if(cit->isunresponsive)
			continue;

		cit->isunresponsive = ectrue;

		ClStatePacket csp;
		csp.header.type = PACKET_CLSTATE;
		csp.chtype = CLCH_UNRESP;
		csp.client = cit->client;
		SendAll((char*)&csp, sizeof(ClStatePacket), ectrue, ecfalse, NULL);
	}

	//unresponsive becoming responsive again
	for(std::list<Widget*>::iterator cit=g_cn.begin(); cit!=g_cn.end(); cit++)
	{
		if(!cit->isclient)
			continue;

		if(now - cit->lastrecv >= NETCONN_UNRESP)
			continue;

		if(!cit->isunresponsive)
			continue;

		cit->isunresponsive = ecfalse;

		ClStatePacket csp;
		csp.header.type = PACKET_CLSTATE;
		csp.chtype = CLCH_RESP;
		csp.client = cit->client;
		SendAll((char*)&csp, sizeof(ClStatePacket), ectrue, ecfalse, NULL);
	}

	//TODO send this as well to newly joined clients on join?
}
#endif
#endif

//Check if we can quit. There's also a countdown to
//make sure we quit eventually either way.
ecbool NetQuit()
{
#ifndef MATCHMAKER
	for(std::list<OldPacket>::iterator oit=g_outgo.begin(); oit!=g_outgo.end(); oit++)
	{
		PacketHeader* ph = (PacketHeader*)oit->buffer;

		//if(ph->type != PACKET_DISCONNECT)
		//	continue;

		//Sending out disconnect packet, so can't quit yet.
		return ecfalse;
	}
#endif

	return ectrue;
}

void EndConns()
{
	std::list<NetConn>::iterator cit = g_cn.begin();
	unsigned __int64 now = GetTicks();

	while(cit != g_cn.end())
	{
		if(!cit->closed
			|| now - cit->lastrecv < NETCONN_TIMEOUT)
		{
			cit++;
			continue;
		}

		FlushPrev(&cit->addr, cit->dock);
		cit = g_cn.erase(cit);
	}
}

//Net input
void UpdNet()
{
	//struct sockaddr_in from;
	//socklen_t fromlen = sizeof(struct sockaddr_in);
	//char buffer[1024];
	int bytes;

	UDPpacket *in;

	//IPaddress* addr = &ns->addr;

	//unsigned int svipaddr = SDL_SwapBE32(addr->host);
	//unsigned short svport = SDL_SwapBE16(addr->port);

	//g_sendrate -= SENDRATE * g_updfrinterval;
	g_sendrate = 0;
	if(g_sendrate < 0)
		g_sendrate = 0;

	UDPsocket* sock = &g_sock;

	if(!sock)
		return;

	in = SDLNet_AllocPacket(65535);

	if(!in)
	{
		OutOfMem(__FILE__,__LINE__);
		Log("SDLNet_AllocPacket out of mem");
	}

	do
	{
		in->data[0] = 0;
		//bytes = recvfrom(g_socket, buffer, 1024, 0, (struct addr *)&from, &fromlen);
		bytes = SDLNet_UDP_Recv(*sock, in);

		IPaddress ip;

		memcpy(&ip, &in->address, sizeof(IPaddress));
		//const char* host = SDLNet_ResolveIP(&ip);
		//unsigned int ipaddr = SDL_SwapBE32(ip.host);
		unsigned short port = SDL_SwapBE16(ip.port);

		//if(port < PORT || port > PORT+10)
		//	continue;

		if(bytes > 0)
		{
			bytes = in->len;	//corpd fix
//#ifdef MATCHMAKER
#if 1
			g_transmitted += bytes;
#endif
			//if(memcmp((void*)&from, (void*)&g_sockaddr, sizeof(struct sockaddr_in)) != 0)
			//if(ipaddr != svipaddr)
			//	continue;

			//Log("r a"<<((PacketHeader*)in->data)->ack<<" t"<<((PacketHeader*)in->data)->type);
	
			//Log("re a%d t%d", (int)((PacketHeader*)(in->data))->ack, (int)((PacketHeader*)(in->data))->type);
			//

			//for(int i=0; i<bytes; i++)
			//for(int i=0; i<4; i++)
			//{
			//	Log("r"<<i<<": "<<(unsigned int)(in->data[i]));
			//	
			//}

#if 0
			if(((PacketHeader*)in->data)->type == PACKET_NETTURN)
			{
				char msg[128];
				sprintf(msg, "recv ack%u t%d", (unsigned int)((PacketHeader*)in->data)->ack, ((PacketHeader*)in->data)->type);
				InfoMess("recv", msg);
			}
#endif

#if 1
			unsigned char *in2;
			int bytes2 = Unparify((unsigned char*)in->data, &in2, bytes);
			if(!bytes2)
			{
				//Log("in parity fail by=%d in->len=%d", (int) bytes, (int)(in->len));

				//for(int i=0; i<bytes; ++i)
				//	Log("byte[%d]=%d", i, (int)(in->data[i]));
				//
				continue;
			}
			else
			{
				//Log("in parity success by=%d", (int)bytes);
				//
			}
			
			if(/* ((PacketHeader*)in2)->type == PACKET_ACKNOWLEDGMENT && */
				((PacketHeader*)in2)->recvdock != 0 && 
				((PacketHeader*)in2)->recvdock != g_dock)
				continue;

			//Log("read in %d", bytes2);

			TranslatePacket((char*)in2, bytes2, ectrue, &g_sock, &ip);
			free(in2);
#endif
			//TranslatePacket(buffer, bytes, ectrue);
			//TranslatePacket((char*)in->data, bytes, ectrue, &g_sock, &ip);
			
		}
	} while(bytes > 0);

	SDLNet_FreePacket(in);

	//KeepAlive();
	//CheckConns();
	
	CheckConns();
	KeepAlive();
	ResendPacks();

#ifndef MATCHMAKER
	CheckAddSv();
	CheckGetSvs();
	//CheckCls();
#else
	SendSvs();
#endif

	//EndConns();

#if 1
	//renew sockets
	// http://stackoverflow.com/questions/18429320/udp-socket-stops-receiving-data

	static unsigned __int64 lastrenew = GetTicks();

#ifdef MATCHMAKER
	if(GetTicks() - lastrenew > (1000 * 60 * 5))
#else
	if(GetTicks() - lastrenew > (1000 * 60 * 5))
#endif
	{
		lastrenew = GetTicks();
		OpenSock();
	}
#endif

#if 1
	static unsigned __int64 lastlog = GetTicks();

#ifdef MATCHMAKER
	if(GetTicks() - lastlog > (1000 * 60 * 60))
#else
	if(GetTicks() - lastlog > (1000 * 60))
#endif
	{
		lastlog = GetTicks();
		Log("transmitted %s = %d bytes\r\n", DateTime().c_str(), g_transmitted);
		
		g_transmitted = 0;
	}
#endif
}

ecbool Same(IPaddress* a, unsigned short ad, IPaddress* b, unsigned short bd)
{
	if(a->host != b->host)
		return ecfalse;

	if(a->port != b->port)
		return ecfalse;

	if(ad != bd)
		return ecfalse;

	return ectrue;
}

#if 1

void ClearPackets()
{
#if 0
	for(std::list<Widget*>::iterator i=g_outgo.begin(); i!=g_outgo.end(); i++)
		i->freemem();

	for(std::list<Widget*>::iterator i=g_recv.begin(); i!=g_recv.end(); i++)
		i->freemem();
#else
	g_outgo.clear();
	g_recv.clear();
#endif
}

#endif
