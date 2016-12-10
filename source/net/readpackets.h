











#ifndef READPACKETS_H
#define READPACKETS_H

#include "../platform.h"
#include "packets.h"

struct NetConn;

void TranslatePacket(char* buffer, int bytes, ecbool checkprev, UDPsocket* sock, IPaddress* from);
void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadPlaceBlPacket(PlaceBlPacket* pbp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadMoveOrderPacket(MoveOrderPacket* mop, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadPlaceCdPacket(PlaceCdPacket* pcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadOrderManPacket(OrderManPacket* omp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSetSalePropPacket(SetSalePropPacket* sspp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadBuyPropPacket(BuyPropPacket* bpp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDemolPropPacket(DemolPropPacket* dpp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadChatPacket(ChatPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAckPacket(AckPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNoConnPacket(NoConnectionPacket* ncp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDoneTurnPacket(DoneTurnPacket* dtp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNetTurnPacket(NetTurnPacket* ntp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadJoinPacket(JoinPacket* jp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadWrongVersionPacket(WrongVersionPacket* wvp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAddSvPacket(AddSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAddedSvPacket(AddedSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSvAddrPacket(SvAddrPacket* sap, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGetSvListPacket(GetSvListPacket* gslp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSendNextHostPacket(SendNextHostPacket* snhp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNoMoreHostsPacket(NoMoreHostsPacket* nmhp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSvInfoPacket(SvInfoPacket* sip, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGetSvInfoPacket(GetSvInfoPacket* gsip, NetConn* nc, IPaddress* from, UDPsocket* sock);
#ifndef MATCHMAKER
void ReadAddClPacket(AddClientPacket* acp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSelfClPacket(SelfClientPacket* scp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSetClNamePacket(SetClNamePacket* scnp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClientLeftPacket(ClientLeftPacket* clp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClientRolePacket(ClientRolePacket* crp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDoneJoinPacket(DoneJoinPacket* djp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadTooManyClPacket(TooManyClPacket* tmcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadMapChPacket(MapChangePacket* mcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadMapStartPacket(MapStartPacket* msp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGameStartedPacket(GameStartedPacket* gsp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadChValPacket(ChValPacket* cvp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClDisconnectedPacket(ClDisconnectedPacket* cdp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClStatePacket(ClStatePacket* csp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadLANCallPacket(LANCallPacket* lcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadLANAnswerPacket(LANAnswerPacket* lap, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDownMapPacket(DownMapPacket* dmp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadUpMapPacket(UpMapPacket* ump, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadFolPacket(FolPacket* fp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadBlPacket(BlPacket* bp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadCdPacket(CdPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadPyPacket(PyPacket* pp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadUnitPacket(UnitPacket* up, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadJamPacket(JamPacket* jp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadVisPacket(VisPacket* vp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadBordPacket(BordPacket* bp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGraphHeadPacket(GraphHeadPacket* ghp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGraphPtPacket(GraphPtPacket* gpp, NetConn* nc, IPaddress* from, UDPsocket* sock);
#endif
void OnAck_Connect(OldPacket* p, NetConn* nc);
void OnAck_Disconnect(OldPacket* p, NetConn* nc);

#endif	//READPACKETS_H
