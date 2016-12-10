












#ifndef SENDPACKETS_H
#define SENDPACKETS_H

#include "../platform.h"
#include "packets.h"

struct NetConn;

void SendAll(char* data, int size, ecbool reliable, ecbool expires, IPaddress* exaddr, unsigned short exdock);
void SendData(char* data, int size, IPaddress * paddr, ecbool reliable, ecbool expires, NetConn* nc, UDPsocket* sock, int msdelay, void (*onackfunc)(OldPacket* p, NetConn* nc));
void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, char* buffer, int bytes);
void ResendPacks();
void Register(char* username, char* password, char* email);
void Login(char* username, char* password);

#endif	//SENDPACKETS_H