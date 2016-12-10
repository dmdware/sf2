










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
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "blview.h"
#include "blpreview.h"
#include "blgraphs.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"
#include "../../../net/lockstep.h"
#include "../../../sim/manuf.h"
#include "../../../net/client.h"
#include "../../../language.h"
#include "../../layouts/chattext.h"
#include "../../../sim/buyprop.h"
#include "../../../sim/demoprop.h"
#include "../../layouts/demogui.h"

int32_t g_ucost = -1;

//Need a better way to organize buttons, text, etc.

//resize bl view column item
void Resize_BV_Cl(Widget* w)
{
	Widget* parw = w->m_parent;
	int32_t row;
	int32_t col;
	sscanf(w->m_name.c_str(), "%d %d", &row, &col);

	BmpFont* f = &g_font[MAINFONT16];	//20

	w->m_pos[0] = parw->m_pos[0] + 64*(imax(col-1,0));
	w->m_pos[1] = parw->m_pos[1] + f->gheight*row + 90;
	w->m_pos[2] = imin(parw->m_pos[0] + 150 + 64*(col+1), parw->m_pos[2]);
	w->m_pos[3] = imin(parw->m_pos[1] + f->gheight*(row+1) + 90, parw->m_pos[3]);

	w->m_pos[0] = fmin(w->m_pos[0], parw->m_pos[2]);
	w->m_pos[0] = fmax(w->m_pos[0], parw->m_pos[0]);

	w->m_pos[2] = fmin(w->m_pos[2], parw->m_pos[2]);
	w->m_pos[2] = fmax(w->m_pos[2], parw->m_pos[0]);

	w->m_pos[1] = fmin(w->m_pos[1], parw->m_pos[3]);
	w->m_pos[1] = fmax(w->m_pos[1], parw->m_pos[1]);

	w->m_pos[3] = fmin(w->m_pos[3], parw->m_pos[3]);
	w->m_pos[3] = fmax(w->m_pos[3], parw->m_pos[1]);

	//first column is extra large (label text)
	if(col > 0)
		w->m_pos[0] += 200;
}

void Resize_BV_Cl_w(Widget* w)
{
	Widget* parw = w->m_parent;
	int32_t row;
	int32_t col;
	sscanf(w->m_name.c_str(), "%d %d", &row, &col);

	BmpFont* f = &g_font[MAINFONT16];	//20

	w->m_pos[0] = parw->m_pos[0] + 64*(imax(col-1,0));
	w->m_pos[1] = parw->m_pos[1] + f->gheight*row + 90;
	w->m_pos[2] = imin(parw->m_pos[0] + 150 + 64*(col+2), parw->m_pos[2]);
	w->m_pos[3] = imin(parw->m_pos[1] + f->gheight*(row+2) + 90, parw->m_pos[3]);

	w->m_pos[0] = fmin(w->m_pos[0], parw->m_pos[2]);
	w->m_pos[0] = fmax(w->m_pos[0], parw->m_pos[0]);

	w->m_pos[2] = fmin(w->m_pos[2], parw->m_pos[2]);
	w->m_pos[2] = fmax(w->m_pos[2], parw->m_pos[0]);

	w->m_pos[1] = fmin(w->m_pos[1], parw->m_pos[3]);
	w->m_pos[1] = fmax(w->m_pos[1], parw->m_pos[1]);

	w->m_pos[3] = fmin(w->m_pos[3], parw->m_pos[3]);
	w->m_pos[3] = fmax(w->m_pos[3], parw->m_pos[1]);

	//first column is extra large (label text)
	if(col > 0)
		w->m_pos[0] += 200;
}

//resize bl view stocked goods list
//TODO regenvals
void Resize_BV_SL(Widget* w)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int32_t row = 0;

	//for sale checkbox
	row ++;

	//property price edit field
	row ++;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int32_t ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#elif 0
	if(b->type == BL_TRFAC)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#else
	row += bt->manuf.size();
#endif

	//bank space
	row++;

	//set button
	row++;
	
	//demoltion checkbox
	row ++;

	//blank space
	row++;
	row++;

	//stocked goods list

	Widget* parw = w->m_parent;
	int32_t col = 0;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];
}

//resize bl view cancel button
void Resize_BV_Close(Widget* w)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int32_t row = 0;

	//for sale checkbox
	row ++;

	//property price edit field
	row ++;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int32_t ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#elif 0
	if(b->type == BL_TRFAC)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#else
	row += bt->manuf.size();
#endif

	//blank space
	row++;

	//set, cancel button
	row++;

	Widget* parw = w->m_parent;
	int32_t col = 1;

	w->m_pos[0] = parw->m_pos[0] + col * 80;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[0] + (col+1) * 80;
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
	
	//demoltion checkbox
	row ++;

	//blank space
	row++;
	row++;

	//stocked goods list
	//...
}

//resize bl view set button
void Resize_BV_Set(Widget* w)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int32_t row = 0;

	//for sale checkbox
	row ++;

	//property price edit field
	row ++;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int32_t ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#elif 0
	if(b->type == BL_TRFAC)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#else
	row += bt->manuf.size();
#endif

	//blank space
	row++;

	//set, cancel button
	row++;

	Widget* parw = w->m_parent;
	int32_t col = 2;

	w->m_pos[0] = parw->m_pos[0] + col * 80;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[0] + (col+1) * 80;
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
	
	//demoltion checkbox
	row ++;

	//blank space
	row++;
	row++;

	//stocked goods list
	//...
}

//resize bl view demolish button
void Resize_BV_Demol(Widget* w)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int32_t row = 0;

	//for sale checkbox
	row ++;

	//property price edit field
	row ++;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int32_t ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#elif 0
	if(b->type == BL_TRFAC)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#else
	row += bt->manuf.size();
#endif

	//blank space
	row++;

	//set, cancel button
	row++;
	
	//demoltion checkbox
	row ++;

	Widget* parw = w->m_parent;
	int32_t col = 2;

	w->m_pos[0] = parw->m_pos[0] + col * 80;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[0] + (col+1) * 80;
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);

	//blank space
	row++;
	row++;

	//stocked goods list
	//...
}

//resize bl view graphs button
void Resize_BV_Graphs(Widget* w)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;
	
	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];
	
	int32_t row = 0;

	//for sale checkbox
	row ++;

	//property price edit field
	row ++;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}
	
	if(bt->input[RES_LABOUR] > 0)
		row++;
	
#if 0
	for(int32_t ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#elif 0
	if(b->type == BL_TRFAC)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#else
	row += bt->manuf.size();
#endif
	
	//blank space
	row++;
	
	//set, cancel button
	row++;

	Widget* parw = w->m_parent;
	int32_t col = 3;
	
	w->m_pos[0] = parw->m_pos[0] + col * 80;
	w->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	w->m_pos[2] = parw->m_pos[0] + (col+1) * 80;
	w->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
	
	//demoltion checkbox
	row ++;
	
	//blank space
	row++;
	row++;
	
	//stocked goods list
	//...
}

void Click_BV_Order(int32_t param)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

#if 0
	//TO DO: multiplayer lockstep netturn packet
	ManufJob mj;
	mj.owner = g_localP;
	mj.utype = param;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();
#elif 0
	OrderMan(param, *sel->buildings.begin(), g_localP);
#else
	OrderManPacket omp;
	omp.header.type = PACKET_ORDERMAN;
	omp.bi = *sel->buildings.begin();
	omp.player = g_localP;
	omp.utype = param;
	LockCmd((PacketHeader*)&omp);
#endif

	//crash as button is erased while in use
	//py = &g_player[g_localP];
	//bv->regen(&g_sel);
}

void Click_BV_Close()
{
	GUI* gui = &g_gui;
	gui->hide("bl view");
}

void Click_BV_Set()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	int32_t bi = *sel->buildings.begin();
	Building* b = &g_building[ bi ];
	BlType* bt = &g_bltype[ b->type ];

	//Set button being shown for non-owner
	if(b->owner != g_localP)
		return;

	int32_t row = 0;
	char wn[32];
	bool forsale;

	//for sale checkbox
	{
		sprintf(wn, "%d 0", row);
		CheckBox* cb = (CheckBox*)bv->get(wn);
		forsale = (bool)cb->m_selected;

		sprintf(wn, "%d 1", row+1);
		EditBox* pp = (EditBox*)bv->get(wn);
		std::string sval = pp->m_value.rawstr();
		if(sval.length() <= 0)
			sval = "0";
		int32_t ival;
		sscanf(sval.c_str(), "%d", &ival);

		if(forsale != b->forsale ||
			ival != b->propprice)
		{
			SetSalePropPacket sspp;
			sspp.header.type = PACKET_SETSALEPROP;
			sspp.propi = bi;
			sspp.selling = forsale;
			sspp.price = ival;
			sspp.proptype = PROP_BL_BEG + b->type;
			sspp.tx = -1;
			sspp.ty = -1;
			LockCmd((PacketHeader*)&sspp);
		}
	}
	row ++;

	//property price edit field
	row ++;

	// change bl view prod price

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		sprintf(wn, "%d 1", row);
		EditBox* pp = (EditBox*)bv->get(wn);
		std::string sval = pp->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		int32_t ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->price[ri] = ival;

		if(b->price[ri] != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLPRICE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.res = ri;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}

		row++;
	}

	EditBox* cw;
	std::string sval;
	int32_t ival;

	if(bt->input[RES_LABOUR] > 0)
	{
		// change bl view (op) wage

		sprintf(wn, "%d 1", row);
		cw = (EditBox*)bv->get(wn);
		sval = cw->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->opwage = ival;

		if(b->opwage != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLWAGE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}

		row++;
	}

	// change bl view production level

	sprintf(wn, "%d 1", row);
	cw = (EditBox*)bv->get(wn);
	sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	//b->prodlevel = ival;

	//char msg[128];
	//sprintf(msg, "prodv %d", ival);
	//InfoMess(msg, msg);

	if(b->prodlevel != ival)
	{
		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_PRODLEV;
		cvp.bi = bi;
		cvp.player = g_localP;
		cvp.value = ival;
		LockCmd((PacketHeader*)&cvp);
	}

	row++;

	// change bl view manuf unit price
	
	for(std::list<uint8_t>::iterator mit=bt->manuf.begin(); mit!=bt->manuf.end(); mit++)
	{
		//buildables set in bltype
		int32_t ui = *mit;

		char wn[32];
		sprintf(wn, "%d 1", row);
		EditBox* cw = (EditBox*)bv->get(wn);
		std::string sval = cw->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		int32_t ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->manufprc[ui] = ival;

		if(b->manufprc[ui] != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_MANPRICE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.utype = ui;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}

		row++;
	}
	
	//demoltion checkbox
	row ++;
}

void Click_BV_OpenGraphs()
{
	GUI *gui = &g_gui;
	BlView *view = (BlView*)gui->get("bl view");
	//doesn't hide now 2016/12/08
	view->show();
	view->regen(&g_sel);
	BlGraphs *graphs = (BlGraphs*)gui->get("bl graphs");
	gui->show("bl graphs");
	graphs->regen(&g_sel);
}

//TODO mouseover manuf button resource costs
BlView::BlView(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BUILDINGVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	m_close = Button(this, "hide", "gui/transp.png", (STRTABLE[STR_CLOSE]), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Close, Click_BV_Close, NULL, NULL, NULL, NULL, -1, NULL);
	m_set = Button(this, "set", "gui/transp.png", (STRTABLE[STR_SET]), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Set, Click_BV_Set, NULL, NULL, NULL, NULL, -1, NULL);
	m_ucost = InsDraw(this, DrawUCost);
	m_graphs = Button(this, "graphs", "gui/transp.png", (STRTABLE[STR_GRAPHS]), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Graphs, Click_BV_OpenGraphs, NULL, NULL, NULL, NULL, -1, NULL);
	

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void BlView::regvals(Selection* sel)
{

}

void BlView::draw()
{
	Win::draw();

	m_close.draw();

	if(m_owned)
		m_set.draw();

	m_graphs.draw();
	m_ucost.draw();
}

void BlView::drawover()
{
	Win::drawover();

	m_close.drawover();
	
	if(m_owned)
		m_set.drawover();

	m_graphs.drawover();
}

void BlView::reframe()
{
	Win::reframe();

	m_close.reframe();
	m_set.reframe();
	m_graphs.reframe();
}

void BlView::inev(InEv* ie)
{
	m_close.inev(ie);
	
	if(m_owned)
		m_set.inev(ie);
	
	m_graphs.inev(ie);

	Win::inev(ie);
}

void DrawUCost()
{
	if(g_ucost < 0)
		return;

	UType* ut = &g_utype[g_ucost];

	RichText text;

	char add[1024];
	sprintf(add, "%s %s:\n", ut->name, STRTABLE[STR_COST].rawstr().c_str());

	text.m_part.push_back(RichPart(add));

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(ut->cost[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];

		//TODO cstr array for resource name

		text.m_part.push_back(RichPart(RICH_ICON, r->icon));
		sprintf(add, " %s: %s %s\n", r->name.c_str(), iform(ut->cost[ri]).c_str(), r->unit.c_str());
		text.m_part.push_back(RichPart(add));
	}

	float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
	DrawBoxShadText(MAINFONT16, g_mouse.x, g_mouse.y+40, 300, 900, &text, color, 0, -1);
}

void Over_UCost(int32_t param)
{
	g_ucost = param;
}

void Out_UCost()
{
	g_ucost = -1;
}


void BlView::regen(Selection* sel)
{
	RichText bname;
	int32_t* price;

	Player* py = &g_player[g_localP];
	BlType* bt = NULL;
	Building* b = NULL;
	bool owned = false;	//owned by current player?
	Player* opy;
	int32_t bi;

	if(sel->buildings.size() > 0)
	{
		bi = *sel->buildings.begin();
		b = &g_building[bi];
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = true;

		g_bptype = b->type;
		price = b->price;

		bname = RichText(UStr(bt->name));

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
				(int32_t)b->transporter[RES_URANIUM],
				(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
				bi,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_player[b->owner];
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i tpos = *sel->roads.begin();
		RoadTile* road = RoadAt(tpos.x, tpos.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i tpos = *sel->powls.begin();
		PowlTile* powl = PowlAt(tpos.x, tpos.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i tpos = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(tpos.x, tpos.y);
		maxcost = crpipe->maxcost;
	}
#endif

	freech();

	m_owned = owned;

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

	if(owned)
	{
		//for sale checkbox
		{
			char rowname[32];
			sprintf(rowname, "%d %d", row, 0);
			add(new CheckBox(this, rowname, STRTABLE[STR_FORSALE], MAINFONT16, Resize_BV_Cl, b->forsale ? 1 : 0));
		}
		row ++;

		//property price edit field
		{
			char rowname[32];
			sprintf(rowname, "%d %d", row, 0);
			Resource* r = &g_resource[RES_DOLLARS];
			add(new Text(this, rowname, STRTABLE[STR_PROPERTYPRICE] + RichText(" ") + RichPart(RICH_ICON, r->icon) + RichText(":"), MAINFONT16, Resize_BV_Cl, true, 1, 1, 1, 1));

			sprintf(rowname, "%d %d", row, 1);
			char val[32];
			sprintf(val, "%d", b->propprice);
			add(new EditBox(this, rowname, RichText(val), MAINFONT16, Resize_BV_Cl, false, 10, NULL, NULL, -1));
		}
		row ++;
	}
	else
	{
		if(b->forsale)
		{
			char rowname[32];
			sprintf(rowname, "%d %d", row, 0);
			add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_BUYPROP], STRTABLE[STR_BUYTHISBL], MAINFONT16, BUST_LINEBASED, Resize_BV_Cl, Click_BuyProp, NULL, NULL, NULL, NULL, -1, NULL));
			
			sprintf(rowname, "%d %d", row, 1);
			char val[32];
			strcpy(val, iform(b->propprice).c_str());
			Resource* r = &g_resource[RES_DOLLARS];
			add(new Text(this, rowname, RichText(RichPart(RICH_ICON, r->icon)) + RichText(val), MAINFONT16, Resize_BV_Cl, true, 1, 1, 1, 1));
		}
		else
		{
			//TODO make an offer?
		}

		row ++;
		row ++;
	}

	//resource outputs price fields
	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		add(new Text(this, rowname, STRTABLE[STR_PRICEOF] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr(r->name.c_str())) + RichText(UStr(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", b->price[ri]);

		if(owned)
			//add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, Change_BV_PP, NULL, ri));
			add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, NULL, NULL, ri));
		else
			add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl));
		
		sprintf(rowname, "%d %d", row, 3);
		add(new Text(this, rowname, RichText("/ ") + RichText(UStr(r->unit.c_str())), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//wage input field
	if(bt->input[RES_LABOUR] > 0)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];

		add(new Text(this, rowname, STRTABLE[STR_WAGE] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", b->opwage);

		if(owned)
			//add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, Change_BV_OW, NULL, RES_LABOUR));
			add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, NULL, NULL, RES_LABOUR));
		else
			add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl));
		
		//sprintf(rowname, "%d %d", row, 3);
		//add(new Text(this, rowname, RichText(UStr(r->unit.c_str())), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//production level input field
	//if(owned)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);

		char label[64];
		sprintf(label, "%s /%d:", STRTABLE[STR_PRODLEVEL].rawstr().c_str(), RATIO_DENOM);

		add(new Text(this, rowname, RichText(UStr(label)), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cplstr[32];
		sprintf(cplstr, "%d", b->prodlevel);

		if(owned)
			//add(new EditBox(this, rowname, RichText(UStr(cplstr)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_PL, NULL, RES_LABOUR));
			add(new EditBox(this, rowname, RichText(UStr(cplstr)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, RES_LABOUR));
		else
			add(new Text(this, rowname, RichText(UStr(cplstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	if(bt)
	{
		for(std::list<unsigned char>::iterator mit=bt->manuf.begin(); mit!=bt->manuf.end(); mit++)
		{
			char rowname[32];
			sprintf(rowname, "%d %d", row, 0);
			RichText label;

			UType* mut = &g_utype[*mit];
			Resource* r = &g_resource[RES_DOLLARS];
	#if 0
	Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,
	int32_t f, int32_t style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int32_t p), void (*overf)(), void (*overf2)(int32_t p), void (*out)(), int32_t parm);
	#endif
			char orderstr[128];
			sprintf(orderstr, "%s %s", STRTABLE[STR_BUY].rawstr().c_str(), mut->name);
			add(new Button(this, rowname, "gui/transp.png",
			RichText(UStr(orderstr)) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr(":")), RichText(),
			MAINFONT16, BUST_LINEBASED, Resize_BV_Cl, NULL, Click_BV_Order, NULL, Over_UCost, Out_UCost, *mit, NULL));

			sprintf(rowname, "%d %d", row, 1);
			char clabel[128];
			sprintf(clabel, "%d", b->manufprc[*mit]);

			if(owned)
				//add(new EditBox(this, rowname, RichText(UStr(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_TRUCK));
				add(new EditBox(this, rowname, RichText(UStr(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, *mit));
			else
				add(new Text(this, rowname, RichText(UStr(clabel)), MAINFONT16, Resize_BV_Cl));

			row++;
		}
	}

	if(owned)
	{
		if(!b->demolition)
		{
			char rowname[32];
			sprintf(rowname, "%d %d", row, 0);
			//TODO strtable RichText(" search
			add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_DEMOL], STRTABLE[STR_DEMOL], MAINFONT16, BUST_LINEBASED, Resize_BV_Demol, Click_DemoProp, NULL, NULL, NULL, NULL, -1, NULL));
		}
		else
		{
		}
	}

	row ++;

	//stocked goods list
	RichText sl;
	sl = STRTABLE[STR_STOCKLIST];
	sl.m_part.push_back(RichPart("\n"));
	bool somestock = false;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(ri == RES_LABOUR || !r->capacity)
		{
			if(b->stocked[ri] <= 0)
				continue;

			somestock = true;

			char num[32];
			std::string stocked = iform(b->stocked[ri]);
			sprintf(num, "%s ", stocked.c_str());

			Resource* r = &g_resource[ri];

			sl.m_part.push_back(RichPart("      "));
			sl.m_part.push_back(RichPart(r->name.c_str()));
			sl.m_part.push_back(RichPart(" "));
			sl.m_part.push_back(RichPart(RICH_ICON, r->icon));
			sl.m_part.push_back(RichPart(": "));
			sl.m_part.push_back(RichPart(num));
			sl.m_part.push_back(RichPart(r->unit.c_str()));
			sl.m_part.push_back(RichPart("\n"));
		}
		else
		{
			int32_t num = 0;
			int32_t denom = 0;

			if(bt->output[ri] > 0)
			{
				int32_t minlevel = b->maxprod();
				num = bt->output[ri] * b->cymet / RATIO_DENOM;
				num = imax(num, bt->output[ri] * minlevel / RATIO_DENOM);
				denom = num;
			}

			bool hasin = false;

			//2015/11/16
			for(int32_t ri2=0; ri2<RESOURCES; ++ri2)
			{
				if(bt->input[ri2] > 0)
				{
					hasin = true;
					break;
				}
			}

			if(!hasin)
			{
				num = bt->output[ri];
				denom = num;
			}

			if(ri == RES_HOUSING &&
				bt->output[ri] > 0)
			{
				//num -= b->occupier.size();
				num = b->occupier.size();
			}

			for(std::list<CapSup>::iterator csit=b->capsup.begin(); csit!=b->capsup.end(); csit++)
			{
				if(csit->rtype != ri)
					continue;

#if 0
				if(b->type == BL_CHEMPL)
				{
					char msg[128];
					sprintf(msg, ">>dst:%d src:%d r:%s this:%d", (int32_t)csit->dst, (int32_t)csit->src, g_resource[ri].name.c_str(), bi);
					InfoMess("...",msg);
				}
#endif

				if(csit->dst == bi)
				{
					num += csit->amt;
					denom += csit->amt;

#if 0
					char msg[128];
					sprintf(msg, "numdenum+=%d", csit->amt);
					if(b->type == BL_CHEMPL)
						InfoMess("...",msg);
#endif
				}

				if(csit->src == bi)
				{
					num -= csit->amt;
				}
			}

			if(num == 0 && denom == 0 && bt->output[ri] <= 0)
				continue;

			somestock = true;

			char str[32];
			if(bt->output[ri] <= 0)
			{
				std::string nums = iform(num);
				sprintf(str, "%s ", nums.c_str());
			}
			else
			{
				std::string nums = iform(num);
				std::string denoms = iform(denom);
				sprintf(str, "%s/%s ", nums.c_str(), denoms.c_str());
			}

			Resource* r = &g_resource[ri];

			sl.m_part.push_back(RichPart("      "));
			sl.m_part.push_back(RichPart(r->name.c_str()));
			sl.m_part.push_back(RichPart(" "));
			sl.m_part.push_back(RichPart(RICH_ICON, r->icon));
			sl.m_part.push_back(RichPart(": "));
			sl.m_part.push_back(RichPart(str));
			sl.m_part.push_back(RichPart(r->unit.c_str()));
			sl.m_part.push_back(RichPart("\n"));
		}
	}

	if(!somestock)
	{
		sl.m_part.push_back(RichPart("      ("));
		sl = sl + STRTABLE[STR_NONE];
		sl.m_part.push_back(RichPart(")"));
	}

	//InfoMess("sl", sl.rawstr().c_str());

	add(new TextBlock(this, "req list", sl, MAINFONT16, Resize_BV_SL, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	//add(new InsDraw(this, DrawUCost));

	reframe();
}
