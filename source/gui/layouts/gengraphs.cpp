










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
#include "gengraphs.h"
#include "blgraphs.h"
#include "blview.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"
#include "../../../net/lockstep.h"
#include "../../../sim/manuf.h"
#include "../../../net/client.h"
#include "../../../sim/simdef.h"

const char *GENGRAPHSTR[GENGRAPH_TYPES] = {
		"Total satiety", 
		"Average individual satiety",
		"Total household funds",
		"Average individual funds",
		"Population" };

Stat g_genstats[GENGRAPH_TYPES];

//TODO Need a better way to organize buttons, text, etc.

void RecStats()
{
	if(g_simframe % CYCLE_FRAMES != 0)
		return;
}

void DrawGenGraphs()
{
	Selection *sel = &g_sel;
	
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
	
	GUI *gui = &g_gui;
	GenGraphs *gg = (GenGraphs*)gui->get("gen graphs");
	
	int32_t ri = 0;
	DropList *rsel = &gg->m_rsel;
	ri = rsel->m_selected;
	
	if(ri < 0)
		return;
	
	float *frame = gg->m_pos;
	
	int32_t nhists = b->cyclehist.size();
	
	float wspan = frame[2] - frame[0];
	float wdiv = wspan / (float)(nhists + 2);
	
	int32_t greatest = 0;
	
	EndS();
	UseS(SHADER_COLOR2D);
	Shader *s = &g_shader[g_curS];
	
	int32_t chin = 1;
	for(std::list<CycleHist>::iterator chit=b->cyclehist.begin(); chit!=b->cyclehist.end(); chit++, ++chin)
	{
		//for(int32_t ri=0; ri<RESOURCES; ++ri)
		{
			/*if(bt->input[ri] <= 0 &&
			   bt->output[ri] <= 0 &&
			   ri != RES_DOLLARS)
				continue;
			*/
			greatest = imax(greatest, chit->cons[ri]);
			greatest = imax(greatest, chit->prod[ri]);
			
			DrawLine(0, 0, 0, 0.5f, frame[0] + wdiv * chin, frame[1], frame[0] + wdiv * chin, frame[3], gg->m_crop);
		}
	}
	
	float hspan = frame[3] - frame[1];
	float hscale = hspan / 2.0f / (1.0f + (float)greatest);
	
	int32_t zero[RESOURCES];
	Zero(zero);
	int32_t *previn = zero;
	int32_t *prevout = zero;
	
	float midy = (frame[3]+frame[1])/2.0f;
	
	//TODO cache graph on a backbuffer
	
	DrawLine(0, 0, 0, 1, frame[0], midy, frame[2], midy, gg->m_crop);
	
	chin = 1;
	for(std::list<CycleHist>::iterator chit=b->cyclehist.begin(); chit!=b->cyclehist.end(); chit++, ++chin)
	{
		//for(int32_t ri=0; ri<RESOURCES; ++ri)
		{/*
			if(bt->input[ri] <= 0 &&
			   bt->output[ri] <= 0 &&
			   ri != RES_DOLLARS)
				continue;
			*/
			Resource *r = &g_resource[ri];
			float *color = r->rgba;
			
			Vec2f linein[2], lineout[2];
			
			linein[0].y = midy + hscale * previn[ri];
			linein[0].x = frame[0] + wdiv * chin;
			linein[1].y = midy + hscale * chit->cons[ri];
			linein[1].x = frame[0] + wdiv * (chin + 1);
			
			lineout[0].y = midy - hscale * prevout[ri];
			lineout[0].x = frame[0] + wdiv * chin;
			lineout[1].y = midy - hscale * chit->prod[ri];
			lineout[1].x = frame[0] + wdiv * (chin + 1);
			
			DrawLine(color[0], color[1], color[2], color[3], linein[0].x, linein[0].y, linein[1].x, linein[1].y, gg->m_crop);
			DrawLine(color[0], color[1], color[2], color[3], lineout[0].x, lineout[0].y, lineout[1].x, lineout[1].y, gg->m_crop);
		}
		
		previn = chit->cons;
		prevout = chit->prod;
	}
			 
	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
	
	BmpFont *f = &g_font[MAINFONT16];
	
	float y = frame[1];
	
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(bt->input[ri] <= 0 &&
		   bt->output[ri] <= 0 &&
		   ri != RES_DOLLARS)
			continue;
		
		Resource *r = &g_resource[ri];
		RichText line = RichText(r->name.c_str());
		DrawShadowedText(MAINFONT16, frame[0]+f->gheight * 3, y, &line, r->rgba, -1);
		y+= f->gheight;
	}
	
	float yspace = midy - frame[1];
	float ylinesfit = yspace / f->gheight;
	float yincrement = (float)greatest / ylinesfit;
	
	float yval = 0;
	float color[4] = {1,1,1,1};
	for(y=midy - f->gheight; y>frame[1]; y-=f->gheight, yval+=yincrement)
	{
		char ct[32];
		sprintf(ct, "%+d", (int32_t)yval);
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT16, frame[0], y, &rt, color, -1);
	}
	
	yval = 0;
	for(y=midy; y<frame[3]; y+=f->gheight, yval+=yincrement)
	{
		char ct[32];
		sprintf(ct, "-%d", (int32_t)yval);
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT16, frame[0], y, &rt, color, -1);
	}
	
	float xspace = chin * wdiv;
	float xlinesfit = xspace / (f->gheight * 4);
	float xincrement = (float)b->cyclehist.size() / xlinesfit;
	
	float xval = 0;
	for(float x=frame[0]+wdiv; xval<b->cyclehist.size(); xval+=xincrement, x+=xincrement*wdiv)
	{
		char ct[32];
		sprintf(ct, "%d:00", (int32_t)xval);
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT16, x, frame[3] - f->gheight, &rt, color, -1);
	}
	
	RichText rtin = RichText("Earnings and production");
	RichText rtout = RichText("Expenses and consumption");
	
	DrawShadowedText(MAINFONT16, frame[0] + f->gheight * 10, midy - f->gheight, &rtin, color, -1);
	DrawShadowedText(MAINFONT16, frame[0] + f->gheight * 10, midy - 1, &rtout, color, -1);
	
	s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
}

void Click_GG_Close()
{
	GUI *gui = &g_gui;
	gui->hide("gen graphs");
}

//TODO mouseover manuf button resource costs
GenGraphs::GenGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_GENGRAPHS;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	
	m_close = Button(this, "hide", "gui/transp.png", RichText("Close"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BG_Close, Click_GG_Close, NULL, NULL, NULL, NULL, -1, NULL);
	m_graphs = InsDraw(this, DrawBlGraphs);
	m_rsel = DropList(this, "rsel", MAINFONT16, Resize_BG_RSel, NULL);
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}

void GenGraphs::regvals(Selection* sel)
{

}

void GenGraphs::draw()
{
	Win::draw();

	m_graphs.draw();
	m_close.draw();
	m_rsel.draw();
}

void GenGraphs::drawover()
{
	Win::drawover();

	m_close.drawover();
	m_rsel.drawover();
}

void GenGraphs::reframe()
{
	Win::reframe();

	m_close.reframe();
	m_rsel.reframe();
}

void GenGraphs::inev(InEv* ie)
{
	m_close.inev(ie);
	m_rsel.inev(ie);

	Win::inev(ie);
}

void GenGraphs::regen(Selection* sel)
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
	
	m_rsel.m_options.clear();
	
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		Resource *r = &g_resource[ri];
		RichText rt = RichText(r->name.c_str());
		m_rsel.m_options.push_back(rt);
	}

	reframe();
}
