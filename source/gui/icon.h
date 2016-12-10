










#ifndef ICON_H
#define ICON_H

#include "../platform.h"
#include "../ustring.h"

struct Icon
{
public:
	unsigned int tex;
	int width;
	int height;
	UStr tag;

	Icon()
	{
		tag = UStr("\\not");
	}
};

#define ICON_DOLLARS		0
#define ICON_PESOS			1
#define ICON_EUROS			2
#define ICON_POUNDS			3
#define ICON_FRANCS			4
#define ICON_YENS			5
#define ICON_RUPEES			6
#define ICON_ROUBLES		7
#define ICON_LABOUR			8
#define ICON_HOUSING		9
#define ICON_FARMPRODUCT	10
#define ICON_WSFOOD			11
#define ICON_RETFOOD		12
#define ICON_CHEMICALS		13
#define ICON_ELECTRONICS	14
#define ICON_RESEARCH		15
#define ICON_PRODUCTION		16
#define ICON_IRONORE		17
#define ICON_URANIUMORE		18
#define ICON_STEEL			19
#define ICON_CRUDEOIL		20
#define ICON_WSFUEL			21
#define ICON_STONE			22
#define ICON_CEMENT			23
#define ICON_ENERGY			24
#define ICON_ENUR			25
#define ICON_COAL			26
#define ICON_TIME			27
#define ICON_RETFUEL		28
#define ICON_LOGS			29
#define ICON_LUMBER			30
#define ICON_WATER			31
#define ICON_EXCLAMATION	32
#define ICON_CENTS			33
#define ICON_SMILEY			34
#define ICON_FIRM			35
#define ICON_GOV			36
#define ICONS				37
//TODO hardcode ICON_FIRM and _GOV for modding

extern Icon g_icon[ICONS];

#ifndef MATCHMAKER
void DefI(int type, const char* relative, const char* tag);
#endif

#endif
