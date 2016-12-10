










#include "../../app/appmain.h"
#include "../../gui/gui.h"
#include "../../gui/widgets/winw.h"
#include "../keymap.h"
#include "../../render/heightmap.h"
#include "../../render/transaction.h"
#include "../../math/camera.h"
#include "../../render/screenshot.h"
#include "../../save/savemap.h"
#include "appgui.h"
#include "playgui.h"
#include "../../gui/icon.h"
#include "../../gui/widgets/spez/resticker.h"
#include "../../gui/widgets/spez/botpan.h"
#include "../../gui/widgets/spez/blpreview.h"
#include "../../gui/widgets/spez/blview.h"
#include "../../gui/widgets/spez/blgraphs.h"
#include "../../gui/widgets/spez/gengraphs.h"
#include "../../gui/widgets/spez/pygraphs.h"
#include "../../gui/widgets/spez/cstrview.h"
#include "../../gui/widgets/spez/svlist.h"
#include "../../gui/widgets/spez/saveview.h"
#include "../../gui/widgets/spez/loadview.h"
#include "../../gui/widgets/spez/truckmgr.h"
#include "appviewport.h"
#include "../../sim/bltype.h"
#include "../../sim/building.h"
#include "../../sim/player.h"
#include "../../sim/conduit.h"
#include "../../sim/simflow.h"
#include "chattext.h"
#include "../../net/client.h"
#include "../../net/lockstep.h"
#include "../../net/sendpackets.h"
#include "messbox.h"
#include "../../math/hmapmath.h"
#include "../../sim/map.h"

Vec3i g_vdrag[2];

void Resize_ResNamesTextBlock(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = 0;
	w->pos[1] = 10;
	w->pos[2] = g_width;
	w->pos[3] = g_height;
}

void Resize_ResAmtsTextBlock(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = 150;
	w->pos[1] = 10;
	w->pos[2] = g_width;
	w->pos[3] = g_height;
}

void Resize_ResDeltasTextBlock(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = 250;
	w->pos[1] = 10;
	w->pos[2] = g_width;
	w->pos[3] = g_height;
}

void Out_Build()
{
}

void Resize_ResTicker(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = g_width;
	w->pos[3] = g_font[MAINFONT16].gheight+5;
	w->tpos[0] = 0;
	w->tpos[1] = 0;
}

void UpdResTicker()
{
	//return;

	static float tickerpos = 0;

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");
	ResTicker* restickerw = (ResTicker*)playview->get("res ticker");
	Widget* restickertw = &restickerw->restext;
	RichText restext;

#if 0
	restext.part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));
	restext.part.push_back(RichPart(" Funds: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_HOUSING));
	restext.part.push_back(RichPart(" Housing: 100/120"));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_FARMPRODUCT));
	restext.part.push_back(RichPart(" Farm Products: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_RETFOOD));
	restext.part.push_back(RichPart(" Retail Food: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_PRODUCTION));
	restext.part.push_back(RichPart(" Production: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_IRONORE));
	restext.part.push_back(RichPart(" Minerals: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_CRUDEOIL));
	restext.part.push_back(RichPart(" Crude Oil: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_WSFUEL));
	restext.part.push_back(RichPart(" Wholesale Fuel: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_RETFUEL));
	restext.part.push_back(RichPart(" Retail Fuel: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_ENERGY));
	restext.part.push_back(RichPart(" Energy: 100/120"));
	restext.part.push_back(RichPart("    "));
	restext.part.push_back(RichPart(RICH_ICON, ICON_ENUR));
	restext.part.push_back(RichPart(" Uranium: 100 +1/"));
	restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.part.push_back(RichPart("    "));
#else
	for(int i=0; i<RESOURCES; i++)
	{
		if(i == RES_LABOUR)
			continue;

		Resource* r = &g_resource[i];

		if(r->capacity && py->local[i] <= 0)
			continue;
		else if(!r->capacity && py->local[i] + py->global[i] <= 0)
			continue;

		restext.part.push_back(RichPart(RICH_ICON, r->icon));

		char cstr1[128];
		sprintf(cstr1, " %s: ", r->name.c_str());
		char cstr2[64];

#if 0	//with reschanges?
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d %+d/", py->local[i] + py->global[i], py->resch[i]);

		strcat(cstr1, cstr2);
		restext.part.push_back(RichPart(cstr1));

		if(!r->capacity)
			restext.part.push_back(RichPart(RICH_ICON, ICON_TIME));
#else
#if 0
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d", py->local[i] + py->global[i]);
#else
		if(r->capacity)
		{
			std::string local = iform(py->local[i]);
			std::string global = iform(py->global[i]);
			sprintf(cstr2, "%s/%s", local.c_str(), global.c_str());
		}
		else
		{
			std::string total = iform(py->local[i] + py->global[i]);
			sprintf(cstr2, "%s", total.c_str());
		}
#endif

		strcat(cstr1, cstr2);
		restext.part.push_back(RichPart(cstr1));
#endif

		restext.part.push_back(RichPart("    "));
	}
#endif

	int len = restext.texlen();

	tickerpos += 0.5f * g_drawfrinterval * 20;

	if((int)tickerpos > len)
		tickerpos = 0;

	int endx = EndX(&restext, restext.rawlen(), MAINFONT16, 0, 0);

	if(endx > g_width)
	{
		RichText restext2 = restext.substr((int)tickerpos, len-(int)tickerpos) + restext.substr(0, (int)tickerpos);
		restickertw->text = restext2;
	}
	else
		restickertw->text = restext;
}

void Resize_BottomPanel(Widget* w)
{
	Py* py = &g_py[g_localP];

	w->pos[0] = 0;
	w->pos[1] = (float)(g_height - MINIMAP_SIZE - 32);
	w->pos[2] = (float)g_width;
	w->pos[3] = (float)g_height;
}

void Out_BuildButton()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	gui->hide("bl preview");
}

void Click_BuildButton(int bwhat)
{
	Py* py = &g_py[g_localP];
	g_build = bwhat;
	//Log("b "<<g_build);
	//char msg[128];
	//sprintf(msg, "b %d", bwhat);
	//InfoMess("t", msg);

	//TODO for mobile, display placement confirmation and show planned/preview bl

#ifdef PLATFORM_MOBILE
	Vec3i cmpos;
	Vec3f intersect;
	Vec2i pixpos = g_scroll + Vec2i(g_width/2,g_height/2);
	Vec3f ray;
	Vec3f point;
	
	IsoToCart(pixpos, &ray, &point);
	
	//if(!MapInter(&g_hmap, ray, point, &cmpos))
	//	return;
	
	Vec3f line[2];
	Vec3f fint;
	line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
	line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	
	FastMapIntersect(&g_hmap, g_mapsz, line, &fint);
	
	cmpos.x = (int)fint.x;
	cmpos.y = (int)fint.y;
	cmpos.z = (int)fint.z;
	
	g_vdrag[0] = cmpos;
	g_vdrag[1] = cmpos;
	
	//g_placecnt = 0;
#endif
	
	Out_BuildButton();
}

void Over_BuildButton(int bwhat)
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	if(gui->get("cs view")->opened)
		return;

	if(gui->get("bl view")->opened)
		return;

	g_bptype = bwhat;

	gui->show("bl preview");
	//BlPreview* bp = (BlPreview*)gui->get("bl preview")->get("bl preview");
	BlPreview* bp = (BlPreview*)gui->get("bl preview");
	Text* tl = (Text*)bp->get("title");

	std::string bname;
	RichText cb;	//conmat block
	RichText ib;	//inputs block
	RichText ob;	//outputs block
	RichText db;	//description block

	TextBlock* cbw = (TextBlock*)bp->get("conmat block");
	TextBlock* ibw = (TextBlock*)bp->get("input block");
	TextBlock* obw = (TextBlock*)bp->get("output block");
	TextBlock* dbw = (TextBlock*)bp->get("desc block");

	//cb.part.push_back(RichPart(UStr("CONSTRUCTION REQUISITES:")));
	cb = cb + STRTABLE[STR_CONREQS];

	if(bwhat < 0)
		;
	else if(bwhat < BL_TYPES)
	{
		//ib.part.push_back(RichPart(UStr("INPUTS:")));
		//ob.part.push_back(RichPart(UStr("OUTPUTS:")));
		ib = ib + STRTABLE[STR_INPUTS];
		ob = ob + STRTABLE[STR_OUTPUTS];

		BlType* bt = &g_bltype[bwhat];
		bname = bt->name;

		db.part.push_back(RichPart(UStr(bt->desc.c_str())));

		int ns = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.part.push_back(RichPart(UStr("\n")));
			cb.part.push_back(RichPart(UStr(r->name.c_str())));
			cb.part.push_back(RichPart(UStr(" ")));
			cb.part.push_back(RichPart(RICH_ICON, r->icon));
			cb.part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->conmat[ri]);
			cb.part.push_back(RichPart(UStr(num)));
			//cb.part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ns <= 0)
			cb = cb + RichPart(UStr("\n")) + STRTABLE[STR_NONE];

		int ni = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			ni ++;
			Resource* r = &g_resource[ri];

			ib.part.push_back(RichPart(UStr("\n")));
			ib.part.push_back(RichPart(UStr(r->name.c_str())));
			ib.part.push_back(RichPart(UStr(" ")));
			ib.part.push_back(RichPart(RICH_ICON, r->icon));
			ib.part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->input[ri]);
			ib.part.push_back(RichPart(UStr(num)));
			//ib.part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ni <= 0)
			ib = ib + RichPart(UStr("\n")) + STRTABLE[STR_NONE];

		int no = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			no ++;
			Resource* r = &g_resource[ri];

			ob.part.push_back(RichPart(UStr("\n")));
			ob.part.push_back(RichPart(UStr(r->name.c_str())));
			ob.part.push_back(RichPart(UStr(" ")));
			ob.part.push_back(RichPart(RICH_ICON, r->icon));
			ob.part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->output[ri]);
			ob.part.push_back(RichPart(UStr(num)));
			//ob.part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(no <= 0)
			ob = ob + RichPart(UStr("\n")) + STRTABLE[STR_NONE];
	}
	else if(bwhat < BL_TYPES + CD_TYPES)
	{
		CdType* ct = &g_cdtype[bwhat - BL_TYPES];
		bname = ct->name;

		db.part.push_back(RichPart(UStr(ct->desc.c_str())));

		int ns = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(ct->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.part.push_back(RichPart(UStr("\n")));
			cb.part.push_back(RichPart(UStr(r->name.c_str())));
			cb.part.push_back(RichPart(UStr(" ")));
			cb.part.push_back(RichPart(RICH_ICON, r->icon));
			cb.part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", ct->conmat[ri]);
			cb.part.push_back(RichPart(UStr(num)));
			//cb.part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ns <= 0)
			cb = cb + RichPart(UStr("\n")) + STRTABLE[STR_NONE];
	}

	tl->text = RichText(UStr(bname.c_str()));
	cbw->text = cb;
	ibw->text = ib;
	obw->text = ob;
	dbw->text = db;

	g_bpcam.position(TILE_SIZE*3, TILE_SIZE*3, TILE_SIZE*3, 0, 0, 0, 0, 1, 0);
}

void Click_NextBuildButton(int nextpage)
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	for(int i=0; i<9; i++)
		bp->buto[i] = ecfalse;

#if 0
	if(nextpage == 1)
		BuildMenu_OpenPage1();
	else if(nextpage == 2)
		BuildMenu_OpenPage2();
	else if(nextpage == 3)
		BuildMenu_OpenPage3();
#else
	//9 - 1 forward button
	int skip = 8 * (nextpage - 0);

	//figure out last page skip?
	if(skip < 0)
	{
		skip = 0;

#if 0
		for(int i=0; i<BL_TYPES; ++i)
		{
			BlType* bt = &g_bltype[i];

			//if(!bt->on)
			//	continue;

			++skip;	
		}

		for(int i=0; i<CD_TYPES; ++i)
		{
			CdType* ct = &g_cdtype[i];

			//if(!ct->on)
			//	continue;

			++skip;
		}

		//divides evenly, i.e., empty after last page
		if(skip / 8 * 8 == skip)
		{
			skip -= 8;
		}
		else
		{
			//restrict to the beginning of page
			//if not enough items to fill last page
			skip = skip / 8 * 8;
		}

		//if there's not enough items to fill 
		//if(skip < 8)
		//	skip = 0;
#endif
	}

	for(int buti=0; buti<8; ++buti)
	{
		int skip2 = skip+buti;
		ecbool found = ecfalse;

		for(int i=0; i<BL_TYPES; ++i)
		{
			BlType* bt = &g_bltype[i];

			//if(!bt->on)
			//	continue;

			if(skip2 <= 0)
			{
				Texture* tex = &g_texture[ bt->icontex ];
				std::string texrel = MakeRelative(tex->fullpath.c_str());

				//InfoMess("b", texrel.c_str());

				bp->but[buti] = 
					Button(bp, "name", texrel.c_str(), RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, i, NULL);
	
				bp->buto[buti] = ectrue;

				found = ectrue;

				break;
			}
			else
				--skip2;

			//++skip;	
		}

		if(found)
			continue;

		for(int i=0; i<CD_TYPES; ++i)
		{
			CdType* ct = &g_cdtype[i];

			//if(!ct->on)
			//	continue;
			
			if(skip2 <= 0)
			{
				Texture* tex = &g_texture[ ct->icontex ];
				std::string texrel = MakeRelative(tex->fullpath.c_str());

				bp->but[buti] = 
					Button(bp, "name", texrel.c_str(), RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CD_BEG+i, NULL);

				bp->buto[buti] = ectrue;
				
				break;
			}
			else
				--skip2;

			//++skip;
		}

		if(skip2 > 0)
			break;
	}

	int maxskip = 0;

	for(int i=0; i<BL_TYPES; ++i)
	{
		BlType* bt = &g_bltype[i];

		//if(!bt->on)
		//	continue;

		++maxskip;	
	}

	for(int i=0; i<CD_TYPES; ++i)
	{
		CdType* ct = &g_cdtype[i];

		//if(!ct->on)
		//	continue;

		++maxskip;
	}
	
	int nextskip = nextpage*8+8;

	if(nextskip >= maxskip)
		nextskip = 0;

	bp->but[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, nextskip/8, NULL);
	bp->buto[8] = ectrue;
#endif

	bp->reframe();
}

#if 0

void BuildMenu_OpenPage1()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0
	Button(Widget* parent, const char* filepath, const RichText t, int f, int style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm)
#endif

#if 0	//with gas station
	bp->but[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE1);
	bp->buto[0] = ectrue;

	bp->but[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE);
	bp->buto[1] = ectrue;

	bp->but[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM);
	bp->buto[2] = ectrue;

	bp->but[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL);
	bp->buto[3] = ectrue;

	bp->but[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILREF);
	bp->buto[4] = ectrue;

	bp->but[5] = Button(bp, "name", "gui/brbut/gasstation2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_GASST);
	bp->buto[5] = ectrue;

	bp->but[6] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_SHMINE);
	bp->buto[6] = ectrue;

	bp->but[7] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_TRFAC);
	bp->buto[7] = ectrue;

	bp->but[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2);
	bp->buto[8] = ectrue;
#else
	bp->but[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE1, NULL);
	bp->buto[0] = ectrue;

	bp->but[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE, NULL);
	bp->buto[1] = ectrue;

	bp->but[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM, NULL);
	bp->buto[2] = ectrue;

	bp->but[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL, NULL);
	bp->buto[3] = ectrue;

	bp->but[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILREF, NULL);
	bp->buto[4] = ectrue;

	bp->but[5] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_SHMINE, NULL);
	bp->buto[5] = ectrue;

	bp->but[6] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_TRFAC, NULL);
	bp->buto[6] = ectrue;

	bp->but[7] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW, NULL);
	bp->buto[7] = ectrue;

	bp->but[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2, NULL);
	bp->buto[8] = ectrue;
#endif

	bp->reframe();
}


void BuildMenu_OpenPage2()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0 //with gas station , c1
	bp->but[0] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW);
	bp->buto[0] = ectrue;

	//bp->but[1] = Button(bp, "name", "gui/brbut/harbour2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HARBOUR);
	//bp->buto[1] = ectrue;

	bp->but[2] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD);
	bp->buto[2] = ectrue;

	bp->but[3] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE);
	bp->buto[3] = ectrue;

	bp->but[4] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL);
	bp->buto[4] = ectrue;

	bp->but[5] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1);
	bp->buto[5] = ectrue;
#else
	bp->but[0] = Button(bp, "name", "gui/brbut/coalpow.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_COALPOW, NULL);
	bp->buto[0] = ectrue;

	bp->but[1] = Button(bp, "name", "gui/brbut/chemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CHEMPL, NULL);
	bp->buto[1] = ectrue;

	bp->but[2] = Button(bp, "name", "gui/brbut/gasstation2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_GASST, NULL);
	bp->buto[2] = ectrue;

	bp->but[3] = Button(bp, "name", "gui/brbut/cemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CEMPL, NULL);
	bp->buto[3] = ectrue;

	//bp->but[4] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE2, NULL);
	//bp->buto[4] = ectrue;

	//bp->but[4] = Button(bp, "name", "gui/brbut/quarry.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_QUARRY, NULL);
	//bp->buto[4] = ectrue;

	bp->but[5] = Button(bp, "name", "gui/brbut/smelter.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_IRONSM, NULL);
	bp->buto[5] = ectrue;
	
	bp->but[6] = Button(bp, "name", "gui/brbut/barracks.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_BRK, NULL);
	//bp->buto[6] = ectrue;

	bp->but[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 3, NULL);
	bp->buto[8] = ectrue;
#endif

	bp->reframe();
}

void BuildMenu_OpenPage3()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	bp->but[0] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD, NULL);
	bp->buto[0] = ectrue;

	bp->but[1] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE, NULL);
	bp->buto[1] = ectrue;

	bp->but[2] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL, NULL);
	bp->buto[2] = ectrue;

	bp->but[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1, NULL);
	bp->buto[8] = ectrue;

	bp->reframe();
}

#endif

void Resize_BlGraphs(Widget* w)
{
	Py* py = &g_py[g_localP];
	
#ifndef PLATFORM_MOBILE
	int centerx = g_width/2;
	int centery = g_height/2;
	
	w->pos[0] = centerx-200;
	w->pos[1] = centery-200;
	w->pos[2] = centerx+200;
	w->pos[3] = centery+200;
#elif 1
	w->pos[0] = 0;
	w->pos[1] = g_height - 300;
	w->pos[2] = 400;
	w->pos[3] = g_height - 150;
#endif
}

void Resize_BlPreview(Widget* w)
{
	Py* py = &g_py[g_localP];

#ifndef PLATFORM_MOBILE
	int centerx = g_width/2;
	int centery = g_height/2;

	w->pos[0] = centerx-350;
	w->pos[1] = centery-200;
	w->pos[2] = centerx+200;
	w->pos[3] = centery+200;
#elif 1
	w->pos[0] = 0;
	w->pos[1] = g_height - 300;
	w->pos[2] = 550;
	w->pos[3] = g_height;
#endif
}

void Resize_ConstructionView(Widget* w)
{
	Py* py = &g_py[g_localP];

#ifndef PLATFORM_MOBILE
	w->pos[0] = g_width/2 - 200;
	w->pos[1] = g_height/2 - 200;
	w->pos[2] = g_width/2 + 200;
	w->pos[3] = g_height/2 + 200;
#else
	w->pos[0] = 0;
	w->pos[1] = g_height - 200;
	w->pos[2] = 400;
	w->pos[3] = g_height;
#endif
}

void Click_MoveConstruction()
{
	int alloced[RESOURCES];
	Zero(alloced);
	int totalloc = 0;

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	for(std::list<int>::iterator siter = g_sel.bl.begin(); siter != g_sel.bl.end(); siter++)
	{
		int bi = *siter;
		Bl* b = &g_bl[bi];

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += b->conmat[i];
			totalloc += b->conmat[i];
		}
	}

#if 0
	for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += r->conmat[i];
			totalloc += r->conmat[i];
		}
	}

	for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}

	for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}
#endif

	if(totalloc <= 0)
	{
		for(std::list<int>::iterator siter = g_sel.bl.begin(); siter != g_sel.bl.end(); siter++)
		{
			int bi = *siter;
			Bl* b = &g_bl[bi];
			b->on = ecfalse;
		}

#if 0
		for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
		{
			Vec2i tpos = *siter;
			RoadTile* r = RoadAt(tpos.x, tpos.y);
			r->on = ecfalse;
		}

		for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
		{
			Vec2i tpos = *siter;
			PowlTile* p = PowlAt(tpos.x, tpos.y);
			p->on = ecfalse;
		}

		for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
		{
			Vec2i tpos = *siter;
			CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
			p->on = ecfalse;
		}
#endif

		ClearSel(&g_sel);
		gui->hide("cs view");
	}
	else
	{
		ShowMessage(RichText("You've already invested resources in this project."));
	}
}

void Click_CancelConstruction()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	for(std::list<int>::iterator siter = g_sel.bl.begin(); siter != g_sel.bl.end(); siter++)
	{
		int bi = *siter;
		Bl* b = &g_bl[bi];
		b->on = ecfalse;
	}

#if 0
	for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);
		r->on = ecfalse;
	}

	for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);
		p->on = ecfalse;
	}

	for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
		p->on = ecfalse;
	}
#endif

	ClearSel(&g_sel);
	gui->hide("cs view");
}

void Click_ProceedConstruction()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	ClearSel(&g_sel);
	gui->hide("cs view");
}

void Click_EstimateConstruction()
{
	Py* py = &g_py[g_localP];
	ClearSel(&g_sel);
	Widget *gui = (Widget*)&g_gui;
	gui->hide("cs view");
}

void Resize_Message(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = (float)(0);
	w->pos[1] = (float)(g_height - 200);
	w->pos[2] = (float)(0 + 400);
	w->pos[3] = (float)(g_height);
}

void Resize_MessageContinue(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = (float)(200 - 80);
	w->pos[1] = (float)(g_height - 130);
	w->pos[2] = (float)(200 + 80);
	w->pos[3] = (float)(g_height - 10);
}

void Click_MessageContinue()
{
#if 0
	std::list<Widget*>::iterator viter = gui->view.begin();
	while(viter != gui->view.end())
	{
		if(stricmp(viter->name.c_str(), "message view") == 0)
		{
			InfoMess("f", "view found");

			viter = gui->view.erase(viter);

			continue;
		}

		viter++;
	}
#endif

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	gui->hide("message view");
}

void ShowMessage(const RichText& msg)
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");
	TextBlock* msgblock = (TextBlock*)msgview->get("message");
	msgblock->text = msg;
	gui->show("message view");
}

void Resize_Window(Widget* w)
{
	Py* py = &g_py[g_localP];
	w->pos[0] = (float)(g_width/2 - 200);
	w->pos[1] = (float)(g_height/2 - 200);
	w->pos[2] = (float)(g_width/2 + 200);
	w->pos[3] = (float)(g_height/2 + 200);
}

void Resize_DebugInfo(Widget* w)
{
	Py* py = &g_py[g_localP];
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Transx(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 1;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Save(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 2;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_QSave(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 3;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Load(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 4;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Pause(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 0;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Play(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 1;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Fast(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 2;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_TrMgr(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 3;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Esc(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 4;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Ranks(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 5;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Graphs(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 6;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_PyGraphs(Widget* w)
{
	Py* py = &g_py[g_localP];
	int i = 10;
	Font* f = &g_font[CHATFONT];
	w->pos[0] = (float)(0 + 40*i);
	w->pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->pos[2] = (float)(40 + 40*i);
	w->pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}


void Click_Pause()
{
	//g_speed = SPEED_PAUSE;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_PAUSE;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);

#if 0
	char cmess[128];
	sprintf(cmess, "pause locking simfr:%u netfr:%u", g_simframe, g_cmdframe);
	RichText mess = RichText(cmess);
	AddNotif(&mess);
#endif
}

void Click_Play()
{
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];

		if(c->downin != -1)
		{
			RichText r = STRTABLE[STR_STILLDOWN];
			Mess(&r);
			return;
		}
	}

	//g_speed = SPEED_PLAY;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_PLAY;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);
}

void Click_Fast()
{
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];

		if(c->downin != -1)
		{
			RichText r = STRTABLE[STR_STILLDOWN];
			Mess(&r);
			return;
		}
	}

	//g_speed = SPEED_FAST;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_FAST;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);
}

void Click_DebugLines()
{
	g_debuglines = !g_debuglines;
}

void Click_Transx()
{
	g_drawtransx = !g_drawtransx;
}

void Click_Graphs()
{
	RichText ms = RichText("Coming soon.");
	Mess(&ms);
	return;

	Widget *gui = (Widget*)&g_gui;
	gui->show("gen graphs");
}

void Click_PyGraphs()
{
	RichText ms = RichText("Coming soon.");
	Mess(&ms);
	return;

	Widget *gui = (Widget*)&g_gui;
	gui->show("py graphs");
}

void Click_TrMgr()
{
	Selection* sel = &g_sel;
	sel->mv.clear();
	Widget *gui = (Widget*)&g_gui;
	TruckMgr *trmgr = (TruckMgr*)gui->get("truck mgr");
	trmgr->regen(&g_sel);
	gui->show("truck mgr");
}

void Click_Ranks()
{
	g_keys[SDL_SCANCODE_TAB] = !g_keys[SDL_SCANCODE_TAB];
}

void Click_Save()
{
#if 0
	if(g_netmode != NETM_SINGLE)
	{
		Mess(&RichText("Coming soon to multiplayer."));
		return;
	}
#endif

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	gui->hide("save");
	gui->hide("load");
	gui->show("save");
	((SaveView*)gui->get("save"))->regen(SAVEMODE_SAVES);
}

void Click_QSave()
{
#if 0
	if(g_netmode != NETM_SINGLE)
	{
		Mess(&RichText("Coming soon to multiplayer."));
		return;
	}
#endif

	if(!g_lastsave[0])
	{
		Click_Save();
		return;
	}

	SaveMap(g_lastsave);

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	gui->hide("save");
	gui->hide("load");
}

void Click_Load()
{
	if(g_netmode != NETM_SINGLE)
	{
		//RichText mess = RichText("Coming soon to multiplayer.");
		RichText mess = STRTABLE[STR_MUSTBESINGLE];
		Mess(&mess);
		return;
	}

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	gui->hide("save");
	gui->hide("load");
	gui->show("load");
	((LoadView*)gui->get("load"))->regen(SAVEMODE_SAVES);
}

void Click_QuitToMenu()
{
	EndSess();
}

void Click_HideBlPrev(int dummy)
{
	Widget *gui = (Widget*)&g_gui;

	gui->hide("bl preview");
	//gui->hide("hide bl preview");
}

void FillPlay()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	g_vptype[VIEWPORT_MINIMAP] = VpType(Vec3f(0, 0, 0), Vec3f(0, 1, 0), "Minimap", ectrue);
	g_vptype[VIEWPORT_ENTVIEW] = VpType(Vec3f(0, MAX_DISTANCE/2, 0), Vec3f(0, 1, 0), "EntView", ecfalse);

	//for(int i=0; i<25; i++)
	//Sleep(6000);

	gui->add(new ViewLayer(gui, "play"));
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	//playview->add(new Image(NULL, "gui/backg/white.png", Resize_ResTicker));
	//playview->add(new Text(NULL, "res ticker", RichText(" "), MAINFONT16, Resize_ResTicker, ectrue, 1, 1, 1, 1));
	playview->add(new ResTicker(playview, "res ticker", Resize_ResTicker));
	playview->add(new BotPan(playview, "bottom panel", Resize_BottomPanel));

	playview->add(new Button(playview, "debug lines", "gui/debuglines.png", RichText(), (STRTABLE[STR_SHOWDEBUG]), MAINFONT16, BUST_LEFTIMAGE, Resize_DebugInfo, Click_DebugLines, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "show info", "gui/transx.png", RichText(), (STRTABLE[STR_SHOWTRANSX]), MAINFONT16, BUST_LEFTIMAGE, Resize_Transx, Click_Transx, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "save", "gui/edsave.png", RichText(), (STRTABLE[STR_SAVEGAME2]), MAINFONT16, BUST_LEFTIMAGE, Resize_Save, Click_Save, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "qsave", "gui/qsave.png", RichText(), (STRTABLE[STR_QSAVE]), MAINFONT16, BUST_LEFTIMAGE, Resize_QSave, Click_QSave, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "load", "gui/edload.png", RichText(), (STRTABLE[STR_LOADGAME2]), MAINFONT16, BUST_LEFTIMAGE, Resize_Load, Click_Load, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "pause", "gui/pause.png", RichText(), (STRTABLE[STR_PAUSEUPD]), MAINFONT16, BUST_LEFTIMAGE, Resize_Pause, Click_Pause, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "play", "gui/play.png", RichText(), (STRTABLE[STR_PLAYUNLOCK]), MAINFONT16, BUST_LEFTIMAGE, Resize_Play, Click_Play, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "fast", "gui/fastforward.png", RichText(), (STRTABLE[STR_FASTUNLOCK]), MAINFONT16, BUST_LEFTIMAGE, Resize_Fast, Click_Fast, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "truck mgr", "gui/brbut/truck.png", RichText(), (STRTABLE[STR_TRUCKMGR]), MAINFONT16, BUST_LEFTIMAGE, Resize_TrMgr, Click_TrMgr, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "esc", "gui/esc.png", RichText(), (STRTABLE[STR_ESC]), MAINFONT16, BUST_LEFTIMAGE, Resize_Esc, Esc, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "ranks", "gui/rank.png", RichText(), (STRTABLE[STR_RANKS]), MAINFONT16, BUST_LEFTIMAGE, Resize_Ranks, Click_Ranks, NULL, NULL, NULL, NULL, -1, NULL));
	//playview->add(new Button(playview, "gen graphs", "gui/graphs.png", RichText(), RichText("General graphs"), MAINFONT16, BUST_LEFTIMAGE, Resize_Graphs, Click_Graphs, NULL, NULL, NULL, NULL, -1));
	//playview->add(new Button(playview, "py graphs", "gui/graphs.png", RichText(), RichText("Py graphs"), MAINFONT16, BUST_LEFTIMAGE, Resize_PyGraphs, Click_PyGraphs, NULL, NULL, NULL, NULL, -1));

	//AddNotif(playview);

	//preload all the button images
	//BuildMenu_OpenPage3();
	//BuildMenu_OpenPage2();
	//BuildMenu_OpenPage1();
	Click_NextBuildButton(0);

	gui->add(new ViewLayer(gui, "chat"));
	ViewLayer* chat = (ViewLayer*)gui->get("chat");
	AddNotif(chat);

	//gui->add(new ViewLayer(gui, "cs view"));
	gui->add(new CstrView(gui, "cs view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));
	//ViewLayer* constrview = (ViewLayer*)gui->get("cs view");

	//constrview->add(new CstrView(NULL, "cs view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));

	//gui->add(new ViewLayer(gui, "bl preview"));
	//gui->add(new ViewLayer(gui, "bl preview"));
	//ViewLayer* blprev = (ViewLayer*)gui->get("bl preview");
	//blprev->add(new TouchListener(blprev, "hide bl preview", Resize_Fullscreen, Click_HideBlPrev, NULL, NULL, -1));
	//blprev->add(new BlPreview(blprev, "bl preview", Resize_BlPreview));
	gui->add(new BlPreview(gui, "bl preview", Resize_BlPreview));
	//ViewLayer* blpreview = (ViewLayer*)gui->get("bl preview");

	//blpreview->add(new TouchListener(NULL, Resize_Fullscreen, NULL, NULL, NULL, -1));
	//blpreview->add(new BlPreview(blpreview, "bl preview", Resize_BlPreview));
	//blpreview->add(new Win(blpreview, "bl preview", Resize_BlPreview));
	
	gui->add(new BlView(gui, "bl view", Resize_BlPreview));
	gui->add(new BlGraphs(gui, "bl graphs", Resize_BlGraphs));
	gui->add(new TruckMgr(gui, "truck mgr", Resize_BlPreview));
	gui->add(new GenGraphs(gui, "gen graphs", Resize_BlPreview));
	gui->add(new PyGraphs(gui, "py graphs", Resize_BlPreview));

	gui->add(new SaveView(gui, "save", Resize_BlPreview));
	gui->add(new LoadView(gui, "load", Resize_BlPreview));

#if 0
	gui->add(new ViewLayer(gui, "construction estimate view"));
	ViewLayer* cev = (ViewLayer*)gui->get("construction estimate view");
#endif

	gui->add(new ViewLayer(gui, "ingame"));
	ViewLayer* ingame = (ViewLayer*)gui->get("ingame");
	ingame->add(new Image(ingame, "", "gui/backg/white.jpg", ectrue, Resize_Fullscreen, 0, 0, 0, 0.5f));
	ingame->add(new TouchListener(ingame, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	ingame->add(new Link(ingame, "0", (STRTABLE[STR_QUITTOMENU]), MAINFONT16, Resize_MenuItem, Click_QuitToMenu));
	ingame->add(new Link(ingame, "1", (STRTABLE[STR_CANCEL]), MAINFONT16, Resize_MenuItem, Esc));

	gui->add(new ViewLayer(gui, "message view"));
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");

	msgview->add(new Image(msgview, "", "gui/backg/white.jpg", ectrue, Resize_Message));
	msgview->add(new TextBlock(msgview, "message", RichText(""), MAINFONT16, Resize_Message));
	msgview->add(new TouchListener(msgview, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	msgview->add(new Button(msgview, "continue button", "gui/transp.png", (STRTABLE[STR_CONTINUE]), RichText(""), MAINFONT16, BUST_LEFTIMAGE, Resize_MessageContinue, Click_MessageContinue, NULL, NULL, NULL, NULL, -1, NULL));
}
