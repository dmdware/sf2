










#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/appviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/player.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../net/lockstep.h"
#include "../../../net/client.h"
#include "../../../language.h"
#include "../../../sim/buyprop.h"

//resize construction view column item
void Resize_CV_Cl(Widget* w)
{
	Widget* parw = w->m_parent;
	int32_t row;
	int32_t col;
	sscanf(w->m_name.c_str(), "%d %d", &row, &col);

	w->m_pos[0] = parw->m_pos[0] + 150*col;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = imin(parw->m_pos[0] + 150*(col+1), parw->m_pos[2]);
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
}

//resize construction view's requisite list
void Resize_CV_RL(Widget* w)
{
	Widget* parw = w->m_parent;
	int32_t row = 2;
	int32_t col = 0;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];
}

#if 0
// change construction view construction wage
void Change_CV_CW(uint32_t key, uint32_t scancode, bool down, int32_t parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	CstrView* cv = (CstrView*)gui->get("cs view");
	EditBox* cw = (EditBox*)cv->get("0 1");
	std::string sval = cw->m_value.rawstr();
	int32_t ival;
	sscanf(sval.c_str(), "%d", &ival);

	Selection* sel = &g_sel;

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	if(sel->buildings.size() > 0)
	{
		int32_t bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];
		b->conwage = ival;
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		CdType* ct = &g_cdtype[CD_ROAD];
		for(std::list<Widget*>::iterator coi=sel->roads.begin(); coi!=sel->roads.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_ROAD, tpos.x, tpos.y, false);
			ctile->conwage = ival;
		}
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		CdType* ct = &g_cdtype[CD_POWL];
		for(std::list<Widget*>::iterator coi=sel->powls.begin(); coi!=sel->powls.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_POWL, tpos.x, tpos.y, false);
			ctile->conwage = ival;
		}
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		CdType* ct = &g_cdtype[CD_CRPIPE];
		for(std::list<Widget*>::iterator coi=sel->crpipes.begin(); coi!=sel->crpipes.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_CRPIPE, tpos.x, tpos.y, false);
			ctile->conwage = ival;
		}
	}
#endif
}
#endif

void Click_CV_Close()
{
	GUI* gui = &g_gui;
	gui->hide("cs view");
}

void Click_CV_Set()
{

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	CstrView* cv = (CstrView*)gui->get("cs view");
	EditBox* cw = (EditBox*)cv->get("0 1");
	std::string sval = cw->m_value.rawstr();
	int32_t ival;
	sscanf(sval.c_str(), "%d", &ival);

	Selection* sel = &g_sel;

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	if(sel->buildings.size() > 0)
	{
		int32_t bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];
		//b->conwage = ival;

		if(b->conwage != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_CSTWAGE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		CdType* ct = &g_cdtype[CD_ROAD];
		for(std::list<Vec2i>::iterator coi=sel->roads.begin(); coi!=sel->roads.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_ROAD, tpos.x, tpos.y, false);
			//ctile->conwage = ival;

			if(ctile->conwage != ival)
			{
				ChValPacket cvp;
				cvp.header.type = PACKET_CHVAL;
				cvp.chtype = CHVAL_CDWAGE;
				//cvp.bi = bi;
				cvp.x = tpos.x;
				cvp.y = tpos.y;
				cvp.cdtype = CD_ROAD;
				cvp.player = g_localP;
				cvp.value = ival;
				LockCmd((PacketHeader*)&cvp);
			}
		}
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		CdType* ct = &g_cdtype[CD_POWL];
		for(std::list<Vec2i>::iterator coi=sel->powls.begin(); coi!=sel->powls.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_POWL, tpos.x, tpos.y, false);
			//ctile->conwage = ival;

			if(ctile->conwage != ival)
			{
				ChValPacket cvp;
				cvp.header.type = PACKET_CHVAL;
				cvp.chtype = CHVAL_CDWAGE;
				//cvp.bi = bi;
				cvp.x = tpos.x;
				cvp.y = tpos.y;
				cvp.cdtype = CD_POWL;
				cvp.player = g_localP;
				cvp.value = ival;
				LockCmd((PacketHeader*)&cvp);
			}
		}
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		CdType* ct = &g_cdtype[CD_CRPIPE];
		for(std::list<Vec2i>::iterator coi=sel->crpipes.begin(); coi!=sel->crpipes.end(); coi++)
		{
			Vec2i tpos = *coi;
			CdTile* ctile = GetCd(CD_CRPIPE, tpos.x, tpos.y, false);
			//ctile->conwage = ival;

			if(ctile->conwage != ival)
			{
				ChValPacket cvp;
				cvp.header.type = PACKET_CHVAL;
				cvp.chtype = CHVAL_CDWAGE;
				//cvp.bi = bi;
				cvp.x = tpos.x;
				cvp.y = tpos.y;
				cvp.cdtype = CD_CRPIPE;
				cvp.player = g_localP;
				cvp.value = ival;
				LockCmd((PacketHeader*)&cvp);
			}
		}
	}
#else
	else
	{
		for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
		{
			CdType* ct = &g_cdtype[ctype];

			//if(!ct->on)
			//	continue;

			if(!sel->cdtiles[ctype].size())
				continue;

			g_bptype = BL_CD_BEG + ctype;
			
			for(std::list<Widget*>::iterator coi=sel->cdtiles[ctype].begin; coi!=sel->cdtiles[ctype].end(); coi++)
			{
				Vec2i tpos = *coi;
				CdTile* ctile = GetCd(ctype, tpos.x, tpos.y, false);
				
				if(ctile->conwage != ival)
				{
					ChValPacket cvp;
					cvp.header.type = PACKET_CHVAL;
					cvp.chtype = CHVAL_CDWAGE;
					//cvp.bi = bi;
					cvp.x = tpos.x;
					cvp.y = tpos.y;
					cvp.cdtype = ctype;
					cvp.player = g_localP;
					cvp.value = ival;
					LockCmd((PacketHeader*)&cvp);
				}
			}

			break;
		}
	}
#endif
}

//resize set and hide buttons, more inward than table cells
void Resize_CV_Cl_2(Widget* w)
{
	Widget* parw = w->m_parent;
	int32_t row;
	int32_t col;
	sscanf(w->m_name.c_str(), "%d %d", &row, &col);

	w->m_pos[0] = parw->m_pos[0] + col * 180;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[0] + (col+1) * 180;
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
}

CstrView::CstrView(Widget* parent, const char* n, void (*reframef)(Widget* w), void (*movefunc)(), void (*cancelfunc)(), void (*proceedfunc)(), void (*estimatefunc)()) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_CONSTRUCTIONVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	this->movefunc = movefunc;
	this->cancelfunc = cancelfunc;
	this->proceedfunc = proceedfunc;
	this->estimatefunc = estimatefunc;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void CstrView::regen(Selection* sel)
{
	int32_t* conmat = NULL;
	int32_t qty = -1;
	RichText bname;
	int32_t conwage;
	CdType* ct;
	bool owned = false;

	Player* py = &g_player[g_localP];
	Player* opy;	//owner player

	//numerator,denominator for amount of requisite met
	int32_t num[RESOURCES];
	int32_t denom[RESOURCES];
	Zero(num);
	Zero(denom);

	bool forsale = false;

	if(sel->buildings.size() > 0)
	{
		int32_t bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];

		conmat = t->conmat;
		g_bptype = b->type;
		bname = RichText(UStr(t->name));
		conwage = b->conwage;
		opy = &g_player[b->owner];
		
		if(b->owner == g_localP)
			owned = true;

		if(b->forsale)
			forsale = true;

		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			Resource* r = &g_resource[ri];

			if(t->conmat[ri] <= 0)
				continue;

			denom[ri] += t->conmat[ri];
			num[ri] += b->conmat[ri];
		}
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		ct = &g_cdtype[CD_ROAD];
		conmat = ct->conmat;
		qty = sel->roads.size();
		Vec2i tpos = *sel->roads.begin();
		CdTile* ctile = GetCd(CD_ROAD, tpos.x, tpos.y, false);
		conwage = ctile->conwage;
		opy = &g_player[ctile->owner];
		bname = RichText(UStr(ct->name));
		
		if(ctile->owner == g_localP)
			owned = true;
		
		if(ctile->selling)
			forsale = true;

		for(std::list<Vec2i>::iterator cit=sel->roads.begin(); cit!=sel->roads.end(); cit++)
		{
			CdTile* ctile = GetCd(CD_ROAD, cit->x, cit->y, false);

			for(int32_t ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(ct->conmat[ri] <= 0)
					continue;

				denom[ri] += ct->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		ct = &g_cdtype[CD_POWL];
		conmat = ct->conmat;
		qty = sel->powls.size();
		Vec2i tpos = *sel->powls.begin();
		CdTile* ctile = GetCd(CD_POWL, tpos.x, tpos.y, false);
		conwage = ctile->conwage;
		opy = &g_player[ctile->owner];
		bname = RichText(UStr(ct->name));
		
		if(ctile->owner == g_localP)
			owned = true;
		
		if(ctile->selling)
			forsale = true;

		for(std::list<Vec2i>::iterator cit=sel->powls.begin(); cit!=sel->powls.end(); cit++)	//corpc fix
		{
			CdTile* ctile = GetCd(CD_POWL, cit->x, cit->y, false);

			for(int32_t ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(ct->conmat[ri] <= 0)
					continue;

				denom[ri] += ct->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		ct = &g_cdtype[CD_CRPIPE];
		conmat = ct->conmat;
		Vec2i tpos = *sel->crpipes.begin();
		CdTile* ctile = GetCd(CD_CRPIPE, tpos.x, tpos.y, false);
		conwage = ctile->conwage;
		opy = &g_player[ctile->owner];
		bname = RichText(UStr(ct->name));
		
		if(ctile->owner == g_localP)
			owned = true;
		
		if(ctile->selling)
			forsale = true;

		for(std::list<Vec2i>::iterator cit=sel->crpipes.begin(); cit!=sel->crpipes.end(); cit++)	//corpc fix
		{
			CdTile* ctile = GetCd(CD_CRPIPE, cit->x, cit->y, false);

			for(int32_t ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(ct->conmat[ri] <= 0)
					continue;

				denom[ri] += ct->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
#else
	else
	{
		for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
		{
			CdType* ct = &g_cdtype[ctype];

			//if(!ct->on)
			//	continue;

			if(!sel->cdtiles[ctype].size())
				continue;

			g_bptype = BL_CD_BEG + ctype;
			
			for(std::list<Widget*>::iterator coi=sel->cdtiles[ctype].begin; coi!=sel->cdtiles[ctype].end(); coi++)
			{
				Vec2i tpos = *coi;
				CdTile* ctile = GetCd(ctype, tpos.x, tpos.y, false);
				
				for(int32_t ri=0; ri<RESOURCES; ri++)
				{
					Resource* r = &g_resource[ri];

					if(ct->conmat[ri] <= 0)
						continue;

					denom[ri] += ct->conmat[ri];
					num[ri] += ctile->conmat[ri];
				}
			}

			break;
		}
	}
#endif

	freech();

	RichText ownname = opy->name;
	if(opy->client >= 0)
	{
		Client* c = &g_client[opy->client];
		ownname = c->name;
	}

	//add(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	add(new Text(this, "owner", ownname, MAINFONT16, Resize_BP_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	add(new Text(this, "title", bname, MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int32_t row = 0;

	char rowname[32];
	sprintf(rowname, "%d %d", row, 0);
	add(new Text(this, rowname, STRTABLE[STR_CONWAGE] + RichText(RichPart(RICH_ICON, ICON_DOLLARS)) + RichText(UStr(":")), MAINFONT16, Resize_CV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	sprintf(rowname, "%d %d", row, 1);
	char cwstr[32];
	sprintf(cwstr, "%d", conwage);
	//add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_CV_Cl, false, 6, Change_CV_CW, NULL, -1));
	if(owned)
		add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_CV_Cl, false, 6, NULL, NULL, -1));
	else
		add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_CV_Cl));

	row++;

	if(owned)
	{
		sprintf(rowname, "%d %d", row, 0);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_CLOSE], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_CV_Close, NULL, NULL, NULL, NULL, -1, NULL));
		sprintf(rowname, "%d %d", row, 1);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_SET], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_CV_Set, NULL, NULL, NULL, NULL, -1, NULL));

		row++;
	}
	else if(forsale)
	{	
		sprintf(rowname, "%d %d", row, 0);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_BUYPROP], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_BuyProp, NULL, NULL, NULL, NULL, -1, NULL));

		row++;
	}

	//requisite list
	RichText rl;
	rl = rl + STRTABLE[STR_CONREQS];
	rl.m_part.push_back(RichPart("\n"));

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(denom[ri] <= 0)
			continue;

		char numdenom[32];
		sprintf(numdenom, "%d/%d ", num[ri], denom[ri]);

		Resource* r = &g_resource[ri];

		rl.m_part.push_back(RichPart("      "));
		rl.m_part.push_back(RichPart(r->name.c_str()));
		rl.m_part.push_back(RichPart(" "));
		rl.m_part.push_back(RichPart(RICH_ICON, r->icon));
		rl.m_part.push_back(RichPart(": "));
		rl.m_part.push_back(RichPart(numdenom));
		rl.m_part.push_back(RichPart(r->unit.c_str()));
		rl.m_part.push_back(RichPart("\n"));
	}

	add(new TextBlock(this, "req list", rl, MAINFONT16, Resize_CV_RL, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	reframe();
}
