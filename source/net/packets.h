












#ifndef PACKETS_H
#define PACKETS_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../math/vec2i.h"
#include "../math/vec2uc.h"
#include "../math/camera.h"
#include "../sim/simflow.h"
#include "../sim/player.h"

#ifndef MATCHMAKER
#include "../sim/resources.h"
#include "../sim/mvtype.h"
#include "../sim/bltype.h"
#include "../sim/conduit.h"
#include "../render/fogofwar.h"
#endif

struct NetConn;

struct OldPacket
{
public:
	char* buffer;
	int len;
	unsigned __int64 last;	//last time resent
	unsigned __int64 first;	//first time sent
	ecbool expires;
	ecbool acked;	//used for outgoing packets
	unsigned __int64 netfr;

	//sender/reciever
	IPaddress addr;
	void (*onackfunc)(OldPacket* op, NetConn* nc);

	void freemem()
	{
		if(len <= 0)
			return;

		if(buffer != NULL)
			delete [] buffer;
		buffer = NULL;
	}

	OldPacket()
	{
		len = 0;
		buffer = NULL;
		onackfunc = NULL;
		acked = ecfalse;
		netfr = g_cmdframe;
	}
	~OldPacket()
	{
		freemem();
	}

	OldPacket(const OldPacket& original)
	{
		len = 0;
		buffer = NULL;
		*this = original;
	}

	OldPacket& operator=(const OldPacket &original)
	{
		//corpdfix
		freemem();

		if(original.buffer && original.len > 0)
		{
			len = original.len;
			if(len > 0)
			{
				buffer = new char[len];
				memcpy((void*)buffer, (void*)original.buffer, len);
			}
			last = original.last;
			first = original.first;
			expires = original.expires;
			acked = original.acked;
			addr = original.addr;
			onackfunc = original.onackfunc;
			netfr = original.netfr;
#ifdef MATCHMAKER
			//ipaddr = original.ipaddr;
			//port = original.port;
			//memcpy((void*)&addr, (void*)&original.addr, sizeof(struct sockaddr_in));
#endif
		}
		else
		{
			buffer = NULL;
			len = 0;
			onackfunc = NULL;
			netfr = 0;
		}

		return *this;
	}
};

//TODO merge some of these into multi-purpose packet types
//TODO separate protocol/control packets from user/command packets

#define	PACKET_NULL						0
#define PACKET_DISCONNECT				1
#define PACKET_CONNECT					2
#define	PACKET_ACKNOWLEDGMENT			3
#define PACKET_PLACEBL					4
#define PACKET_NETTURN					5
#define PACKET_DONETURN					6
#define PACKET_JOIN						7
#define PACKET_ADDSV					8
#define PACKET_ADDEDSV					9
#define PACKET_KEEPALIVE				10
#define PACKET_GETSVLIST				11
#define PACKET_SVADDR					12
#define PACKET_SVINFO					13
#define PACKET_GETSVINFO				14
#define PACKET_SENDNEXTHOST				15
#define PACKET_NOMOREHOSTS				16
#define PACKET_ADDCLIENT				17
#define PACKET_SELFCLIENT				18
#define PACKET_SETCLNAME				19
#define PACKET_CLIENTLEFT				20
#define PACKET_CLIENTROLE				21
#define PACKET_DONEJOIN					22
#define PACKET_TOOMANYCL				23
#define PACKET_MAPCHANGE				24
#define PACKET_CHVAL					25
#define PACKET_CLDISCONNECTED			26
#define PACKET_CLSTATE					27
#define PACKET_NOCONN					28
#define PACKET_ORDERMAN					29
#define PACKET_CHAT						30
#define PACKET_MAPSTART					31
#define PACKET_GAMESTARTED				32
#define PACKET_WRONGVERSION				33
#define PACKET_MOVEORDER				34
#define PACKET_PLACECD					35
#define PACKET_NACK						36
#define PACKET_LANCALL					37
#define PACKET_LANANSWER				38
#define PACKET_SETSALEPROP				39
#define PACKET_BUYPROP					40
#define PACKET_DOWNMAP					41
#define PACKET_UPMAP					42
#define PACKET_DEMOLPROP				43
#define PACKET_UNIT						44
#define PACKET_BL						45
#define PACKET_CD						46
#define PACKET_FOL						47
#define PACKET_PY						48
#define PACKET_JAM						49
#define PACKET_VIS						50
#define PACKET_BORD						51
#define PACKET_GRAPHHEAD				52
#define PACKET_GRAPHPT					53
#define PACKET_TYPES					54

// byte-align structures
#pragma pack(push, 1)

struct PacketHeader
{
	unsigned short type;
	unsigned short ack;
	unsigned short senddock;
	unsigned short recvdock;
};

struct BasePacket
{
	PacketHeader header;
};

typedef BasePacket NoConnectionPacket;
typedef BasePacket DoneJoinPacket;
typedef BasePacket TooManyClPacket;
typedef BasePacket SendNextHostPacket;
typedef BasePacket NoMoreHostsPacket;
typedef BasePacket GetSvInfoPacket;
typedef BasePacket GetSvListPacket;
typedef BasePacket KeepAlivePacket;
typedef BasePacket AddSvPacket;
typedef BasePacket AddedSvPacket;
typedef BasePacket AckPacket;
typedef BasePacket NAckPacket;
typedef BasePacket MapStartPacket;
//typedef BasePacket GameStartedPacket;
typedef BasePacket LANCallPacket;
typedef BasePacket LANAnswerPacket;
typedef BasePacket DownMapPacket;

#ifndef MATCHMAKER

struct FolPacket
{
	PacketHeader header;
	int fi;

	ecbool on;
	unsigned char type;
	Vec2i cmpos;
	float yaw;

	//Updraw
};

struct GraphHeadPacket
{
	PacketHeader header;
	unsigned __int64 simframe;
	int cycles;
	int8_t series;
	int8_t row;

#define SERIES_PROTECT	0
#define SERIES_HEALTH	1
};

struct GraphPtPacket
{
	PacketHeader header;
	int8_t series;
	int8_t row;
	float pt;
};

struct BlPacket
{
	PacketHeader header;
	int bi;
	
	ecbool on;
	int type;
	int owner;

	Vec2i tpos;	//position in tiles
	Vec2f drawpos;	//drawing position in world pixels

	ecbool finished;

	int stocked[RESOURCES];
	int inuse[RESOURCES];

	int conmat[RESOURCES];
	ecbool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property
	ecbool forsale;	//is this property for sale?
	ecbool demolish;	//was a demolition ordered?
	//TODO only one stocked[] and no conmat[]. set stocked[] to 0 after construction finishes, and after demolition ordered.

	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned __int64 lastcy;	//last simframe of last production cycle
	
	int manufprc[MV_TYPES];
	short transporter[RESOURCES];

	int hp;
	
	int varcost[RESOURCES];	//variable cost for input resource ri
	int fixcost;	//fixed cost for transport

	ecbool demolition;

	short pownetw;
	short crpipenetw;

	int8_t nroadnetw;
	short ncapsup;
	int8_t noccup;
	int8_t nworker;
	char data[0];

	//std::list<short> roadnetw;
	//std::list<CapSup> capsup;	//capacity suppliers
	//std::list<CycleHist> cyclehist;
	//std::list<int> occupier;
	//std::list<int> worker;
	//std::list<ManufJob> manufjob;

	//UpDraw
};

struct CdPacket
{
	PacketHeader header;
	
	int8_t cdx;
	int8_t cdy;
	int8_t cdti;
	
	ecbool on;
	unsigned char conntype;
	ecbool finished;
	unsigned char owner;
	int conmat[RESOURCES];
	short netw;	//network
	//ecbool inaccessible;
	short transporter[RESOURCES];
	Vec2f drawpos;
	//int maxcost[RESOURCES];
	int conwage;

	ecbool selling;

	//Updraw
};

struct PyPacket
{
	PacketHeader header;

	int pyi;
	ecbool on;
	ecbool ai;
	
	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second
	unsigned __int64 util;
	unsigned __int64 gnp;
	
	unsigned __int64 lastthink;	//AI simframe timer

#if 0
	signed char insttype;	//institution type
	signed char instin;	//institution index
	int parentst;	//parent state player index
	
	ecbool protectionism;
	int imtariffratio;	//import tariff ratio
	int extariffratio;	//export tariff ratio
	
	//TODO check if to include +1 or if thats part of pynamelen
	char username[PYNAME_LEN+1];
	char password[PYNAME_LEN+1];
#endif
};

struct UnitPacket
{
	PacketHeader header;
	int ui;
	ecbool on;
	int type;
	int owner;
	Vec2i cmpos;
	Vec3f facing;	//used for tank turret
	Vec3f rotation;
	float frame[2];	//BODY_LOWER and BODY_UPPER
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
	int cargoamt;
	int cargotype;
	int cargoreq;	//req amt
	//std::vector<TransportJob> bids;
	Vec2i subgoal;
	unsigned char mode;
	int pathdelay;
	unsigned __int64 lastpath;

	ecbool threadwait;

	// used for debugging - don't save to file
	ecbool collided;

	signed char cdtype;	//conduit type for mode (going to construction)
	int driver;
	//short framesleft;
	int cyframes;	//cycle frames (unit cycle of consumption and work)
	int opwage;	//transport driver wage	//edit: NOT USED, set globally in Py struct
	//std::list<TransportJob> bids;	//for trucks

	int exputil;	//expected utility on arrival

	ecbool forsale;
	int price;

	int incomerate;

	unsigned short npath;
	unsigned short ntpath;
	char pathdata[0];

	//std::list<TrCycleHist> cyclehist;

	//UpDraw
};

struct JamPacket
{
	PacketHeader header;
	unsigned char jam;
	unsigned char tx;
	unsigned char ty;
};

struct VisPacket
{
	PacketHeader header;
	int8_t tx;
	int8_t ty;
	VisTile vt;
};

struct BordPacket
{
	PacketHeader header;
	int8_t tx;
	int8_t ty;
	int8_t owner;
};

#endif

struct GameStartedPacket
{
	PacketHeader header;
	unsigned __int64 simframe;
	unsigned __int64 cmdframe;
	Vec2uc mapsz;
};

struct UpMapPacket
{
	PacketHeader header;
	int paysz;
	char data[0];
};

//set property selling state
//TODO set selling conduits
struct SetSalePropPacket
{
	PacketHeader header;
	int propi;
	ecbool selling;	//TODO custom ecbool typedef
	int price;
	int proptype;
	signed char tx;
	signed char ty;
	//int pi;	//should match owner at the time. necessary?
};

#define PROP_BL_BEG		0
#define PROP_BL_END		(BL_TYPES)
#define PROP_CD_BEG		(PROP_BL_END)
#define PROP_CD_END		(PROP_CD_BEG+CD_TYPES)
#define PROP_U_BEG		(PROP_CD_END)
#define PROP_U_END		(PROP_U_BEG+MV_TYPES)

//TODO sell off and set for jobs for individual trucks and individual deals apart from std::list<Widget*>::iterator manager
//buy property
//TODO implement buying conduits
//TODO drop-down tree view for all bl and conduits
//TODO selection box for conduits
struct BuyPropPacket
{
	PacketHeader header;
	int propi;
	int pi;
	int proptype;
	signed char tx;
	signed char ty;
};

struct DemolPropPacket
{
	PacketHeader header;
	int propi;
	int pi;
	int proptype;
	signed char tx;
	signed char ty;
};

//TODO order demolish property and conduit

//variable length depending on number of tiles involved
struct PlaceCdPacket
{
	PacketHeader header;
	short player;
	unsigned char cdtype;
	short ntiles;
	Vec2uc place[0];
};

//variable length depending on number of mv ordered
struct MoveOrderPacket
{
	PacketHeader header;
	Vec2i mapgoal;
	unsigned short nunits;
	unsigned short mv[0];
};

//order unit manufacture at building
struct OrderManPacket
{
	PacketHeader header;
	int mvtype;
	int player;
	int bi;
};

#define MAX_CHAT			711

struct ChatPacket
{
	PacketHeader header;
	int client;
	char msg[MAX_CHAT+1];
};

#define CLCH_UNRESP			0	//client became unresponsive
#define CLCH_RESP			1	//became responsive again
#define CLCH_PING			2
#define CLCH_READY			3	//client became ready to start
#define CLCH_NOTREADY		4	//client became not ready
#define CLCH_PAUSE			5	//pause speed
#define CLCH_PLAY			6	//normal play speed
#define CLCH_FAST			7	//fast forward speed

struct ClStatePacket
{
	PacketHeader header;
	unsigned char chtype;
	short client;
	float ping;
	int downin;
};

/*
The difference between client left
and disconnected is that ClDisc*
means it was done by server (kicked? timed out?)
*/

struct ClientLeftPacket
{
	PacketHeader header;
	short client;
};

struct ClDisconnectedPacket
{
	PacketHeader header;
	short client;
	ecbool timeout;
};

#define CHVAL_BLPRICE					0
#define CHVAL_BLWAGE					1
#define CHVAL_TRWAGE					2
#define CHVAL_TRPRICE					3
#define CHVAL_CSTWAGE					4
#define CHVAL_PRODLEV					5
#define CHVAL_CDWAGE					6
#define CHVAL_MANPRICE					7

struct ChValPacket
{
	PacketHeader header;
	unsigned char chtype;
	int value;
	unsigned char player;
	unsigned char res;
	unsigned short bi;
	unsigned char x;
	unsigned char y;
	unsigned char cdtype;
	unsigned char mvtype;
};

//not counting null terminator
#define MAPNAME_LEN		127
#define SVNAME_LEN		63

struct MapChangePacket
{
	PacketHeader header;
	unsigned int checksum;
	int filesz;
	char map[MAPNAME_LEN+1];
	ecbool live;
};

#define CLPLAT_WIN	0	//windows
#define CLPLAT_MAC	1	//mac
#define CLPLAT_LIN	2	//linux
#define CLPLAT_IPH	3	//iphone
#define CLPLAT_IPA	4	//ipad
#define CLPLAT_AND	5	//android
#define CLPLAT_TYPES	6

#if defined( PLATFORM_WIN )
#define CLPLATI		CLPLAT_WIN
#elif defined( PLATFORM_IPHONE )
#define CLPLATI		CLPLAT_IPH
#elif defined( PLATFORM_IPAD )
#define CLPLATI		CLPLAT_IPA
#elif defined( PLATFORM_ANDROID )
#define CLPLATI		CLPLAT_AND
#elif defined( PLATFORM_MAC )
#define CLPLATI		CLPLAT_MAC
#elif defined( PLATFORM_LINUX )
#define CLPLATI		CLPLAT_LIN
#else
#define CLPLATI		-1
#endif

extern char* CLPLATSTR[CLPLAT_TYPES];

struct JoinPacket
{
	PacketHeader header;
	unsigned int version;
	unsigned char clplat;
	char name[PYNAME_LEN+1];
};

struct WrongVersionPacket
{
	PacketHeader header;
	unsigned int correct;
};

struct AddClientPacket
{
	PacketHeader header;
	signed char client;
	signed char player;
	char name[PYNAME_LEN+1];
	ecbool ishost;
	ecbool ready;
	unsigned char clplat;
	unsigned char speed;
};

struct SelfClientPacket
{
	PacketHeader header;
	int client;
};

struct SetClNamePacket
{
	PacketHeader header;
	int client;
	char name[PYNAME_LEN+1];
};

struct ClientRolePacket
{
	PacketHeader header;
	signed char client;
	signed char player;
};

struct SvAddrPacket
{
	PacketHeader header;
	IPaddress addr;
	unsigned short dock;
};

struct SendSvInfo	//sendable
{
public:
	IPaddress addr;
	char svname[SVNAME_LEN+1];
	short nplayers;
	short maxpys;
	char mapname[MAPNAME_LEN+1];
	ecbool started;
};

struct SvInfoPacket
{
	PacketHeader header;
	SendSvInfo svinfo;
};

struct NetTurnPacket
{
	PacketHeader header;
	/*
	Is this the net frame upon which it is delivered?
	Or the frame when this will be executed?
	Let's make it the frame that will be executed.
	*/
	unsigned __int64 fornetfr;	//for net fr #..
	unsigned short loadsz;
	//commands go after
};

struct DoneTurnPacket
{
	PacketHeader header;
	unsigned __int64 curnetfr;	//current net fr #.. (start of cl's current turn)
	short client;	//should match sender
};

struct PlaceBlPacket
{
	PacketHeader header;
	int btype;
	Vec2i tpos;
	int player;
};

struct ConnectPacket
{
	PacketHeader header;
	ecbool reconnect;
	unsigned short yourlastrecvack;
	unsigned short yournextrecvack;
	unsigned short yourlastsendack;
};

struct DisconnectPacket
{
	PacketHeader header;
	ecbool reply;
};

// Default alignment
#pragma pack(pop)

#endif



