
#include "buyprop.h"
#include "selection.h"
#include "player.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "mvtype.h"
#include "conduit.h"
#include "../gui/gui.h"
#include "../platform.h"
#include "../gui/layouts/chattext.h"
#include "../net/lockstep.h"
#include "../language.h"

void Click_BuyProp()
{
	Selection* sel = &g_sel;
	Py* py = &g_py[g_localP];
	ecbool owned = ecfalse;	//owned by current player?
	Py* opy;
	int propi = -1;
	int propprice;
	int proptype;
	int8_t tx = -1;
	int8_t ty = -1;

	//TODO mv
	//TODO conduits

	if(sel->bl.size())
	{
		BlType* bt = NULL;
		Bl* b = NULL;

		propi = *sel->bl.begin();
		b = &g_bl[propi];
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = ectrue;

		if(!b->forsale)
			return;

		proptype = PROP_BL_BEG + b->type;
		propprice = b->propprice;

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n mv->cargotype=%d",
				(int)b->transporter[RES_URANIUM],
				(int)g_mv[b->transporter[RES_URANIUM]].mode,
				(int)g_mv[b->transporter[RES_URANIUM]].target,
				bi,
				(int)g_mv[b->transporter[RES_URANIUM]].targtype,
				(int)g_mv[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_py[b->owner];
	}
	else if(sel->mv.size())
	{
		Mv* mv = NULL;

		propi = *sel->mv.begin();
		u = &g_mv[propi];

		if(mv->owner == g_localP)
			owned = ectrue;

		if(!mv->forsale)
			return;

		proptype = PROP_U_BEG + mv->type;
		propprice = mv->price;
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		Vec2i t = *sel->roads.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_ROAD];

		CdTile* ctile = GetCd(CD_ROAD, tx, ty, ecfalse);

		if(ctile->owner == g_localP)
			owned = ectrue;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_ROAD;
		propprice = 0;
	}
	else if(sel->powls.size() > 0)
	{
		Vec2i t = *sel->powls.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_POWL];

		CdTile* ctile = GetCd(CD_POWL, tx, ty, ecfalse);

		if(ctile->owner == g_localP)
			owned = ectrue;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_POWL;
		propprice = 0;
	}
	else if(sel->crpipes.size() > 0)
	{
		Vec2i t = *sel->crpipes.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_CRPIPE];

		CdTile* ctile = GetCd(CD_CRPIPE, tx, ty, ecfalse);

		if(ctile->owner == g_localP)
			owned = ectrue;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_CRPIPE;
		propprice = 0;
	}
#endif

	if(py->global[RES_DOLLARS] < propprice)
	{
		RichText textr = STRTABLE[STR_INSUFBUY];
		AddNotif(&textr);
		return;
	}

	BuyPropPacket bpp;
	bpp.header.type = PACKET_BUYPROP;
	bpp.proptype = proptype;
	bpp.propi = propi;
	bpp.pi = g_localP;
	bpp.tx = tx;
	bpp.ty = ty;
	LockCmd((PacketHeader*)&bpp);
}