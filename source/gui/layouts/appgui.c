









#include "../../tut/tut.h"
#include "../../app/appmain.h"
#include "../../gui/gui.h"
#include "../keymap.h"
#include "../../render/heightmap.h"
#include "../../math/hmapmath.h"
#include "../../math/camera.h"
#include "../../render/screenshot.h"
#include "../../save/savemap.h"
#include "../../sim/unit.h"
#include "../../sim/mvtype.h"
#include "../../sim/building.h"
#include "../../sim/bltype.h"
#include "../../sim/selection.h"
#include "../../render/water.h"
#include "../../sim/order.h"
#include "../../path/pathdebug.h"
#include "../../path/pathnode.h"
#include "../../sim/build.h"
#include "../../sim/player.h"
#include "../../sim/unit.h"
#include "../../sim/mvtype.h"
#include "../../gui/widgets/winw.h"
#include "../../debug.h"
#include "../../trigger/console.h"
#include "../../window.h"
#include "../../net/lockstep.h"
#include "../../net/sendpackets.h"
#include "../../gui/widgets/spez/svlist.h"
#include "../../gui/widgets/spez/newhost.h"
#include "../../gui/widgets/spez/loadview.h"
#include "../../gui/widgets/spez/lobby.h"
#include "../../sim/simdef.h"
#include "../../path/collidertile.h"
#include "../../sim/map.h"
#include "../../sim/bltype.h"
#include "../../net/client.h"
#include "../../algo/random.h"
#include "../../sim/umove.h"
#include "../widgets/spez/saveview.h"
#include "../widgets/spez/loadview.h"
#include "../../language.h"
#include "../../sim/border.h"
#include "../../sim/player.h"
#include "../../version.h"
#include "../../sim/player.h"
#include "../../net/packets.h"
#include "../../algo/random.h"
#include "../../tool/compilebl.h"
#include "../../app/seviewport.h"
#include "../../save/compilemap.h"
#include "../../app/segui.h"

//not engine
#include "edgui.h"
#include "playgui.h"
#include "appgui.h"
#include "messbox.h"
#include "../../app/appmain.h"
#include "../../tool/rendersprite.h"

//ecbool g_canselect = ectrue;

char g_lastsave[SFH_MAX_PATH+1];

void Resize_Fullscreen(Widget* w)
{
	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = (float)g_width;
	w->pos[3] = (float)g_height;
}

void Resize_FullscreenSq(Widget* w)
{
	float minsz = fmin((float)g_width-1, (float)g_height-1);

	w->pos[0] = g_width/2.0f - minsz/2.0f;
	w->pos[1] = g_height/2.0f - minsz/2.0f;
	w->pos[2] = g_width/2.0f + minsz/2.0f;
	w->pos[3] = g_height/2.0f + minsz/2.0f;
}

void Resize_AppLogo(Widget* w)
{
	w->pos[0] = 30;
	w->pos[1] = 30;
	w->pos[2] = 200;
	w->pos[3] = 200;
}

void Resize_AppTitle(Widget* w)
{
	w->pos[0] = 30;
	w->pos[1] = 30;
	w->pos[2] = (float)g_width-1;
	w->pos[3] = 100;
}

void Click_LoadMapButton()
{
	Widget *gui;
	LoadView *v;

	gui = (Widget*)&g_gui;
	v = (LoadView*)Widget_get(gui, "load");

	LoadView_regen(v, SAVEMODE_MAPS);
	Widget_show((Widget*)v);
}

void Click_SaveMapButton()
{
	Widget *gui;
	SaveView *v;

	gui = (Widget*)&g_gui;
	v = (SaveView*)gui->get("save");

	SaveView_regen(v, SAVEMODE_MAPS);
	Widget_show((Widget*)v);
}

void Click_QSaveMapButton()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveMapButton();
		return;
	}

	SaveMap(g_lastsave);
}

void Resize_MenuItem(Widget* w)
{
	int row;
	Font* f;

	sscanf(w->name, "%d", &row);
	f = g_font+w->font;

	w->pos[0] = f->gheight*4;
	w->pos[1] = g_height/2 - f->gheight*4 + f->gheight*1.5f*row;
	w->pos[2] = w->pos[0] + f->gheight * 10;
	w->pos[3] = w->pos[1] + f->gheight;
}

void Resize_Status(Widget* w)
{
	w->pos[0] = (float)g_width/2 - 64;
	w->pos[1] = (float)g_height/2;
	w->pos[2] = (float)g_width;
	w->pos[3] = (float)g_height;
	w->tpos[0] = (float)g_width/2;
	w->tpos[1] = (float)g_height/2;
}

void Resize_WinText(Widget* w)
{
	Widget* parent;

	parent = w->parent;

	w->pos[0] = parent->pos[0];
	w->pos[1] = parent->pos[1];
	w->pos[2] = w->pos[0] + 400;
	w->pos[3] = w->pos[1] + 2000;
}

void Click_HostGame()
{
	Widget *gui;
	NewHost* newhost;

	gui = (Widget*)&g_gui;
	Widget_show(gui, "new host");
	newhost = (NewHost*)Widget_get(gui, "new host");
	NewHost_regen((Widget*)newhost);
}

void Click_ListHosts()
{
	Widget *gui;
	gui = (Widget*)&g_gui;
	Widget_show(gui, "sv list");
}

void Click_EndGame()
{
	EndSess();
}

void Click_NewGame()
{
	unsigned char x, y;
	Py* py;
	unsigned char pyi;
	short tminx, tminy, tmaxx, tmaxy;
	Vec2i cmpos;
	Vec3i cmscroll;
	Widget *gui;

	SRAND(0);
	CHECKGLERROR();
	FreeMap();
	FreePys();

	Hmap_alloc(&g_hmap, MAX_MAP, MAX_MAP);

	for(x=0; x<=g_mapsz.x; ++x)
	{
		for(y=0; y<=g_mapsz.y; ++y)
		{
			Hmap_seth(&g_hmap, x, y, 1);
		}
	}

	Hmap_lowev(&g_hmap);
	Hmap_remesh(&g_hmap);

	for(x=0; x<=g_mapsz.x; ++x)
	{
		for(y=0; y<=g_mapsz.y; ++y)
		{
			Hmap_seth(&g_hmap, x, y, Hmap_geth(&g_hmap, x, y) / 2 + 1);
		}
	}

	Hmap_lowev(&g_hmap);
	Hmap_remesh(&g_hmap);

	AllocPathGrid((g_mapsz.x)*TILE_SIZE, (g_mapsz.y)*TILE_SIZE);
	AllocGrid(g_mapsz.x, g_mapsz.y);
	FillColliderGrid();

	FillForest(RAND());

	const int sqr = isqrt((int)Max2Pow((int)PLAYERS));

	//must be set because unit lastpath depends on this
	//2016/05/04 simframe now reset before placing mv
	g_simframe = 0;

	py = g_py;
	pyi = 0;
	for(; py<g_py+PLAYERS; ++pyi, ++py)
	{
		x = i % sqr;
		y = i / sqr;

		tminx = x * 10 / sqr;
		tminy = y * 10 / sqr;
		tmaxx = tminx + 10 / sqr;
		tmaxy = tminy + 10 / sqr;

		tmaxx = imin(g_mapsz.x, tmaxx);
		tmaxy = imin(g_mapsz.y, tmaxy);

		for(x=0; x < 2 * (1); x++)
		{
			for(y=0; y < 2 * (1); y++)
			{
				cmpos.x = tminx*TILE_SIZE + (x+2)*PATHNODE_SIZE*4;
				cmpos.y = tminy*TILE_SIZE + (y+2)*PATHNODE_SIZE*4;
				PlaceUnit(MV_LABOURER, cmpos, i, NULL);
				ClearFol(cmpos.x - TILE_SIZE, cmpos.y - TILE_SIZE, cmpos.x + TILE_SIZE, cmpos.y + TILE_SIZE);
			}
		}
	}

	cmscroll.x = 4 * TILE_SIZE;
	cmscroll.y = 4 * TILE_SIZE;
	cmscroll.z = (int)Bilerp(&g_hmap, (float)cmscroll.x, (float)cmscroll.y);
	g_scroll = CartToIso(cmscroll) - Vec2i(g_width,g_height)/2;

	ResetCls();
	g_localP = 0;
	py = g_py+g_localP;
	py->on = ectrue;
	py->ai = ecfalse;
	AddClient(NULL, g_name, &g_localC, CLPLATI);
	AssocPy(g_localP, g_localC);

	CHECKGLERROR();

	for(py=g_py; py<g_py+PLAYERS; ++py)
	{
		p->ai = (i == g_localP) ? ecfalse : ectrue;
	}

	IniRes();

	for(py=g_py; py<g_py+8; ++py)
	{
		p->on = ectrue;
	}

	CHECKGLERROR();

	g_appmode = APPMODE_PLAY;

	gui = (Widget*)&g_gui;

	Widget_hideall(gui);
	Widget_show(gui, "play");
	Click_NextBuildButton(0);
	Widget_show(gui, "chat");

	CHECKGLERROR();

	g_lastsave[0] = '\0';
	g_netmode = NETM_SINGLE;
	BegSess();
}

void Click_OpenTut()
{
	Click_NewGame();
	g_netmode = NETM_SINGLE;

	g_tutstep = 0;
	AdvTut();
}

void Click_OpenEd()
{
	unsigned char x, y;
	Vec3i cmscroll;
	Widget *gui;
	Vec2i scsz;

	FreeMap();
	ResetCls();
	FreePys();
	IniRes();

	Hmap_alloc(&g_hmap, MAX_MAP, MAX_MAP);

	for(x=0; x<MAX_MAP; ++x)
	{
		for(y=0; y<MAX_MAP; ++y)
		{
			Hmap_seth(&g_hmap, x, y, 1);
		}
	}

	Hmap_lowev(&g_hmap);
	Hmap_remesh(&g_hmap);

	AllocPathGrid(g_mapsz.x*TILE_SIZE, g_mapsz.y*TILE_SIZE);
	AllocGrid(g_mapsz.x, g_mapsz.y);
	FillColliderGrid();

	cmscroll.x = 4 * TILE_SIZE;
	cmscroll.y = 4 * TILE_SIZE;
	cmscroll.z = (int)Bilerp(&g_hmap, (float)cmscroll.x, (float)cmscroll.y);

	scsz.x = -g_width/2;
	scsz.y = -g_height/2;

	Vec2i_sub(&g_scroll, CartToIso(cmscroll), scsz);

	g_appmode = APPMODE_EDITOR;

	gui = (Widget*)&g_gui;

	Widget_hideall(gui);
	Widget_show(gui, "ed");

	Change_ToolCat();

	g_lastsave[0] = '\0';
}


/*TODO make window resize functions fit window to screen, 
no matter how small it is, but be more centered on larger screens
TODO scroll bars on smaller screens*/
void Resize_CenterWin2(Widget* w)
{
#ifdef PLATFORM_MOBILE
	w->pos[0] = g_width/2 - 250 + 60;
	w->pos[1] = g_height - 300;
	w->pos[2] = g_width/2 + 200 + 60;
	w->pos[3] = g_height;
#else
	w->pos[0] = (float)g_width/2 - 250 + 60;
	w->pos[1] = (float)g_height/2 - 150;
	w->pos[2] = (float)g_width/2 + 200 + 60;
	w->pos[3] = (float)g_height/2 + 150;
#endif
}

void Resize_CenterWin(Widget* w)
{
#ifdef PLATFORM_MOBILE
	w->pos[0] = 0;
	w->pos[1] = g_height - 200;
	w->pos[2] = 400;
	w->pos[3] = g_height;
#else
	w->pos[0] = (float)g_width/2 - 200;
	w->pos[1] = (float)g_height/2 - 100;
	w->pos[2] = (float)g_width/2 + 200;
	w->pos[3] = (float)g_height/2 + 100;
#endif
}

void Resize_SvList(Widget* w)
{
#ifndef PLATFORM_MOBILE
	w->pos[0] = (float)g_width/2 - 250;
	w->pos[1] = (float)g_height/2 - 100;
	w->pos[2] = (float)g_width/2 + 300;
	w->pos[3] = (float)g_height/2 + 200;
#else
	w->pos[0] = 0;
	w->pos[1] = g_height - 300;
	w->pos[2] = 550;
	w->pos[3] = g_height;
#endif
}

void Click_LoadGame()
{
	Widget *gui;
	LoadView* loadview;

	gui = (Widget*)&g_gui;
	loadview = (LoadView*)Widget_get(gui, "load");
	Widget_show((Widget*)loadview);
	LoadView_regen(loadview, SAVEMODE_SAVES);
}

void Click_Options()
{
	Widget *gui;
	Widget* options;

	gui = (Widget*)&g_gui;
	Widget_hideall(gui);
	options = Widget_get(gui, "options");
	Widget_show(options);
}

void Click_Quit()
{
	EndSess();
	FreeMap();
	g_quit = ectrue;
}

void Resize_JoinCancel(Widget* w)
{
	w->pos[0] = (float)g_width/2 - 75;
	w->pos[1] = (float)g_height/2 + 100 - 30;
	w->pos[2] = (float)g_width/2 + 75;
	w->pos[3] = (float)g_height/2 + 100 - 0;
	CenterLabel(w);
}

void Click_SetOptions()
{
	Widget *gui;
	ViewLayer* opview;
	DropList* winmodes;
	DropList* reslist;
	EditBox* namebox;
	DropList* vols;
	DropList* langs;
	ecbool restart;
	ecbool write;
	short w;
	short h;
	ecbool fs;
	char* lang;
	int vol;
	char langfull[SFH_MAX_PATH+1];
	char* resname;
	List langdirs;	/* char* */
	char lin;
	Node* lit;	/* char* */
	char* oldname;
	char* newname;

	gui = (Widget*)&g_gui;
	opview = (ViewLayer*)Widget_get(gui, "options");

#ifndef PLATFORM_MOBILE
	winmodes = (DropList*)Widget_get(opview, "0");
	reslist = (DropList*)Widget_get(opview, "1");
#endif
	namebox = (EditBox*)Widget_get(opview, "2");
	vols = (DropList*)Widget_get(opview, "3");
	langs = (DropList*)Widget_get(opview, "4");

	restart = ecfalse;
	write = ecfalse;

#ifndef PLATFORM_MOBILE
	resname = reslist->options[ reslist->selected ];
	fs = (winmodes->selected == 1) ? ectrue : ecfalse;
	sscanf(resname, "%hdx%hd", &w, &h);
#endif

	FullPath("lang/", langfull);
	List_init(&langdirs);
	ListDirs(langfull, langdirs);

	lin = 0;
	for(lit=langdirs.head; lit; lit=lit->next, ++lin)
	{
		if(lin != langs->selected)
			continue;

		lang = (char*)lit->data;
		break;
	}

	vol = vols->selected * 10;

#ifndef PLATFORM_MOBILE
	if(w != g_selres.width)
		restart = ectrue;

	if(h != g_selres.height)
		restart = ectrue;

	if(fs != g_fs)
		restart = ectrue;
#endif

	if(vol != g_volume)
		write = ectrue;

	if(stricmp(lang, g_lang) != 0)
	{
		SwitchLang(lang);
		write = ectrue;
		restart = ectrue;
	}

	Widget_hideall(gui);
	Widget_show(gui, "main");

	oldname = g_name;
	newname = namebox->value;

	/* name changed? */
	if(strcmp(oldname, newname) != 0)
	{
		strcpy(g_name, namebox->value);
		WriteName();
	}

	if(restart)
		write = ectrue;

	if(!write)
		return;

#ifdef PLATFORM_MOBILE
#else
	g_selres.width = w;
	g_selres.height = h;
	//Resize(w, h);
	g_width = w;
	g_height = h;
	g_fs = fs;
#endif
	g_lang = lang;
	//g_lang = LANG_ENG;
	g_volume = vol;
	SetVol(vol);

	WriteConfig();
	List_free(&langdirs);

	if(!restart)
		return;

	g_appmode = APPMODE_RELOADING;
	Resize(w, h);
}

void Resize_CopyInfo(Widget* w)
{
	w->pos[0] = (float)g_width - 260;
	w->pos[1] = (float)g_height - 60;
	w->pos[2] = (float)g_width;
	w->pos[3] = (float)g_height;
}

void EnumDisp()
{
	short i;
	SDL_DisplayMode mode;
	ecbool found;
	Node* rit;	/* Vec2i */
	Vec2i* rp;
	Vec2i r;

	List_free(&g_ress);

	for(i=0; i<SDL_GetNumDisplayModes(0); ++i)
	{
		SDL_GetDisplayMode(0, i, &mode);

		found = ecfalse;

		for(rit=g_ress.head; rit; rit=rit->next)
		{
			rp = (Vec2i*)rit->data;

			if(rp->width == mode.w &&
				rp->height == mode.h)
			{
				found = ectrue;
				break;
			}
		}

		if(found)
			continue;

#ifdef PLATFORM_MOBILE
		//landscape only
		if(mode.h > mode.w)
			continue;

		if(mode.h < 480)
			continue;
#endif

		r.width = mode.w;
		r.height = mode.h;
		List_pushback2(&g_ress, &r, sizeof(r));
	}
}

void FillMenu()
{
	Widget *gui;
	ViewLayer *mainview, *opview, *jview;
	Image *bg, *logo;
	Link *tutl, *newl, *loadl, *hostl, *joinl, *edl, *opl, *exl;
	Text *title;
	TextBlock *copyt;
	char infostr[128];
	char verstr[32];
	DropList *wind, *resd;
	EditBox *namee;
	DropList *vold, *langd;
	Link *donel;
	unsigned char i;
	Node *rit;	/* Vec2i */
	Vec2i *rp;
	SvList *svl;
	NewHost *nhv;
	Text *statt;
	Button *canb;

	gui = (Widget*)&g_gui;


	/* Main ViewLayer */

	mainview = (ViewLayer*)malloc(sizeof(ViewLayer));
	bg = (Image*)malloc(sizeof(Image));
	logo = (Image*)malloc(sizeof(Image));
	tutl = (Link*)malloc(sizeof(Link));
	newl = (Link*)malloc(sizeof(Link));
	loadl = (Link*)malloc(sizeof(Link));
	hostl = (Link*)malloc(sizeof(Link));
	joinl = (Link*)malloc(sizeof(Link));
	edl = (Link*)malloc(sizeof(Link));
	opl = (Link*)malloc(sizeof(Link));
	exl = (Link*)malloc(sizeof(Link));
	title = (Text*)malloc(sizeof(Text));
	copyt = (TextBlock*)malloc(sizeof(TextBlock));

	/*
	todo instead of "0" "1" etc. names use extra-data members
	*/

	ViewLayer_init(mainview, gui, "main");
	Image_init(bg, (Widget*)mainview, "", "gui/mmbg.jpg", ectrue, Resize_Fullscreen);
	Image_init(logo, (Widget*)mainview, "", "gui/i-64x64.png", ectrue, Resize_AppLogo);
	Link_init(tutl, (Widget*)mainview, "0", STRTABLE[STR_TUTORIAL], MAINFONT16, Resize_MenuItem, Click_OpenTut);
	Link_init(newl, (Widget*)mainview, "1", STRTABLE[STR_NEWGAME], MAINFONT16, Resize_MenuItem, Click_NewGame);
	Link_init(loadl, (Widget*)mainview, "2", STRTABLE[STR_LOADGAME], MAINFONT16, Resize_MenuItem, Click_LoadGame);
	Link_init(hostl, (Widget*)mainview, "3", STRTABLE[STR_HOSTGAME], MAINFONT16, Resize_MenuItem, Click_HostGame);
	Link_init(joinl, (Widget*)mainview, "4", STRTABLE[STR_JOINGAME], MAINFONT16, Resize_MenuItem, Click_ListHosts);
	Link_init(edl, (Widget*)mainview, "5", STRTABLE[STR_EDITOR], MAINFONT16, Resize_MenuItem, Click_OpenEd);
	Link_init(opl, (Widget*)mainview, "6", STRTABLE[STR_OPTIONS], MAINFONT16, Resize_MenuItem, Click_Options);
	Link_init(exl, (Widget*)mainview, "7", STRTABLE[STR_QUIT], MAINFONT16, Resize_MenuItem, Click_Quit);
	Text_init(title, (Widget*)mainview, "", STRTABLE[STR_TITLE], MAINFONT32, Resize_AppTitle);

	Widget_add(gui, (Widget*)mainview);
	Widget_add((Widget*)mainview, (Widget*)bg);
	Widget_add((Widget*)mainview, (Widget*)logo);
	Widget_add((Widget*)mainview, (Widget*)tutl);
	Widget_add((Widget*)mainview, (Widget*)newl);
	Widget_add((Widget*)mainview, (Widget*)loadl);
	Widget_add((Widget*)mainview, (Widget*)hostl);
	Widget_add((Widget*)mainview, (Widget*)joinl);
#ifndef PLATFORM_MOBILE
	Widget_add((Widget*)mainview, (Widget*)edl);
#endif
	Widget_add((Widget*)mainview, (Widget*)opl);
#ifndef PLATFORM_MOBILE
	Widget_add((Widget*)mainview, (Widget*)exl);
#endif
	Widget_add((Widget*)mainview, (Widget*)title);

	VerStr(APPVERSION, verstr);
	sprintf(infostr, "%sDMD 'Ware\n%s %s", 
		STRTABLE[STR_COPYRIGHT],
		STRTABLE[STR_VERSION], 
		verstr);
	TextBlock_init(copyt, (Widget*)mainview, "copy", infostr, MAINFONT16, Resize_CopyInfo, 0.2f, 1.0f, 0.2f, 1.0f);
	Widget_add((Widget*)mainview, (Widget*)copyt);


	/* Options ViewLayer */

	opview = (ViewLayer*)malloc(sizeof(ViewLayer));
	bg = (Image*)malloc(sizeof(Image));
	logo = (Image*)malloc(sizeof(Image));
	wind = (DropList*)malloc(sizeof(DropList));
	resd = (DropList*)malloc(sizeof(DropList));
	namee = (EditBox*)malloc(sizeof(EditBox));
	vold = (DropList*)malloc(sizeof(DropList));
	langd = (DropList*)malloc(sizeof(DropList));
	title = (Text*)malloc(sizeof(Text));
	donel = (Link*)malloc(sizeof(Link));

	ViewLayer_init(opview gui, "options");
	Image_init(bg, (Widget*)opview, "", "gui/mmbg.jpg", ectrue, Resize_Fullscreen);
	Image_init(logo, (Widget*)opview, "", "gui/i-64x64.png", ectrue, Resize_AppLogo);
	Text_init(title, (Widget*)opview, "", STRTABLE[STR_TITLE], MAINFONT32, Resize_AppTitle);
	DropList_init(wind, (Widget*)opview, "0", MAINFONT16, Resize_MenuItem, NULL);
	DropList_init(resd, (Widget*)opview, "1", MAINFONT16, Resize_MenuItem, NULL);
	EditBox_init(namee, opview, "2", g_name, MAINFONT16, Resize_MenuItem, ecfalse, PYNAME_LEN, NULL, NULL, -1);
	DropList_init(vold, (Widget*)opview, "3", MAINFONT16, Resize_MenuItem, NULL);
	DropList_init(langd, (Widget*)opview, "4", MAINFONT16, Resize_MenuItem, NULL);
	Link_init(donel, (Widget*)opview, "5", STRTABLE[STR_DONE], MAINFONT16, Resize_MenuItem, Click_SetOptions);

	Widget_add(gui, (Widget*)opview);
	Widget_add((Widget*)opview, (Widget*)bg);
	Widget_add((Widget*)opview, (Widget*)logo);
	Widget_add((Widget*)opview, (Widget*)wind);
	Widget_add((Widget*)opview, (Widget*)resd);
	Widget_add((Widget*)opview, (Widget*)namee);
	Widget_add((Widget*)opview, (Widget*)vold);
	Widget_add((Widget*)opview, (Widget*)langd);
	Widget_add((Widget*)opview, (Widget*)donel);
	Widget_add((Widget*)opview, (Widget*)title);

	/* volumes */
	for(i=0; i<=100; i+=10)
	{
		sprintf(infostr, "%s%hhd", STRTABLE[STR_VOLUMEPCT], i);
		List_pushback2(&vold->options, infostr, strlen(infostr)+1);
	}
	vold->selected = g_volume / 10;

	/* languages */
	/* todo hard-code
	std::list<std::string> langdirs;
	char langpath[SFH_MAX_PATH+1];
	FullPath("lang/", langpath);
	ListDirs(langpath, langdirs);

	int lin = 0;
	for(std::list<std::string>::iterator lit=langdirs.begin(); lit!=langdirs.end(); lit++, lin++)
	{
	std::string selflang = LoadLangStr(lit->c_str(), STRFILE[STR_SELFLANG]);

	if(stricmp(lit->c_str(), g_lang.c_str()) == 0)
	langs->selected = lin;

	langs->options.push_back(RichText(selflang.c_str()));
	}
	*/

	/* window modes */
	List_pushback2(&wind->options, STRTABLE[STR_WINDOWED], strlen(STRTABLE[STR_WINDOWED])+1);
	List_pushback2(&wind->options, STRTABLE[STR_FULLSCREEN], strlen(STRTABLE[STR_FULLSCREEN])+1);
	wind->selected = g_fs ? 1 : 0;

	/* resolutions window sizes */
	EnumDisp();
	i = 0;
	for(rit=g_ress.head; rit; rit=rit->next, ++i)
	{
		rp = (Vec2i*)rit->data;

		if(rp->width == g_selres.width &&
			rp->height == g_selres.height)
			resd->selected = i;

		/* todo Vec2i short,short */
		sprintf(infostr, "%hdx%hd", rp->width, rp->height);
		List_pushback2(&resd->options, infostr, strlen(infostr)+1);
	}

	/* window widgets layouts */
	svl = (SvList*)malloc(sizeof(SvList));
	nhv = (NewHost*)malloc(sizeof(NewHost));

	SvList_init(svl, gui, "sv list", Resize_SvList);
	NewHost_init(nhv, gui, "new host", Resize_CenterWin2);

	Widget_add(gui, (Widget*)svl);
	Widget_add(gui, (Widget*)nhv);

	FillLobby();

	/* join status screen */
	jview = (ViewLayer*)malloc(sizeof(ViewLayer));
	bg = (Image*)malloc(sizeof(Image));
	logo = (Image*)malloc(sizeof(Image));
	statt = (Text*)malloc(sizeof(Text));
	canb = (Button*)malloc(sizeof(Button));

	ViewLayer_init(jview, gui, "join");
	Image_init(bg, (Widget*)jview, "", "gui/mmbg.jpg", ectrue, Resize_Fullscreen);
	Image_init(logo, (Widget*)jview, "", "gui/i-64x64.png", ectrue, Resize_AppLogo);
	Text_init(statt, (Widget*)jview, "status", STRTABLE[STR_JOINING], MAINFONT16, Resize_Status);
	Button_init(canb, (Widget*)jview, "cancel", "gui/transp.png", STRTABLE[STR_CANCEL], "", MAINFONT16, BUST_LINEBASED, Resize_JoinCancel, Click_JoinCancel, NULL, NULL, NULL, NULL, -1, NULL);

	Widget_add(gui, (Widget*)jview);
	Widget_add((Widget*)jview, (Widget*)bg);
	Widget_add((Widget*)jview, (Widget*)logo);
	Widget_add((Widget*)jview, (Widget*)statt);
	Widget_add((Widget*)jview, (Widget*)canb);
}

void MouseLDown()
{
	if(g_appmode == APPMODE_PLAY ||
		g_appmode == APPMODE_EDITOR)
	{
		g_mousestart = g_mouse;

		if(g_appmode == APPMODE_EDITOR)
			g_vdrag[0] = g_mouse3d;	/* todo redo */

		if(g_appmode == APPMODE_EDITOR &&
			GetEdTool() == TOOLCAT_BORDERS)
			EdPlaceBords();	/* todo redo */
	}
}

void EdPlaceFol()
{
	Vec2i pixpos;
	Vec3i ray;
	Vec3i point;
	Vec3i cmpos;
	Vec3i line[2];
	char ftype;
	FlType *ft;
	Vec2i cmmin;
	Vec2i cmmax;

	/* todo float->int */
	Vec3i_add(&pixpos, g_mouse, g_scroll);
	IsoToCart(pixpos, &ray, &point);

	Vec3i_muli(&line[0], ray, MAX_MAP * 5 * TILE_SIZE);
	Vec3i_muli(&line[1], ray, - MAX_MAP * 5 * TILE_SIZE);
	Vec3i_add(&line[0], line[0], point);
	Vec3i_add(&line[1], line[1], point);

	/* todo float->int */
	if(!FastMapIntersect(&g_hmap, g_mapsz, line, &cmpos))
		return;

	/* not deterministic: */
	ftype = rand()%FL_TYPES;
	ft = g_fltype+ftype;

	cmmin.x = cmpos.x - ft->size.x / 2;
	cmmin.y = cmpos.y - ft->size.x / 2;
	cmmax.x = cmmin.x + ft->size.x;
	cmmax.y = cmmin.y + ft->size.y;

	if(Collides(cmmin, cmmax, -1))
		return;

	PlaceFol(ftype, cmpos);
}

/* todo rename */
void EdPlaceUnit()
{	
	signed char type, owner;
	Vec2i cmpos;

	type = GetEdUType();

	if(type < 0)
		return;

	owner = GetPlaceOwner();

	if(owner < 0)
		return;

	cmpos.x = g_mouse3d.x;
	cmpos.y = g_mouse3d.y;

	if(!UnitCollides(NULL, cmpos, type))
		PlaceUnit(type, cmpos, owner, NULL);
}

/* todo remove depth maps and do old-school isometric sort, with speed hacks
eg depth sorting only every second */
void EdPlaceBl()
{
	signed char type, owner;
	Vec2uc tpos;

	if(!g_canplace)
		return;

	type = GetEdBlType();

	tpos.x = g_mouse3d.x/TILE_SIZE;
	tpos.y = g_mouse3d.y/TILE_SIZE;

	owner = GetPlaceOwner();

	if(CheckCanPlace(type, tpos, -1))
		PlaceBl(type, tpos, ectrue, owner, NULL);
}

void EdDel()
{
	Selection sel;
	Node *it;
	char cdtype;
	CdType* ct;
	List *cdl;
	CdTile* ctile;
	Vec2uc* tpos;

	Sel_init(&sel);
	DoSel(&sel);

	for(it=sel.mv.head; it; it=it->next)
	{
		Mv_free(&g_mv[ *(short*)it->data ]);
		goto clean;
	}

	for(it=sel.bl.head; it; it=it->next)
	{
		Bl_free(&g_b[ *(short*)it->data ]);
		goto clean;
	}

	for(cdtype=0; cdtype<CD_TYPES; ++cdtype)
	{
		ct = g_cdtype+cdtype;
		cdl = (List*)(((char*)&sel)+ct->seloff);

		for(it=cdl->head; it; it=it->next)
		{
			tpos = (Vec2uc*)it->data; 
			ctile = GetCd(cdtype, tpos->x, tpos->y, ecfalse);
			CdTile_free(ctile);
			ConnectCdAround(cdtype, tpos->x, tpos->y, ecfalse);
			goto clean;
		}
	}

	for(it=sel.fl.head; it; it=it->next)
	{
		Fl_free(&g_fl[ *(unsigned short*)it->data ]);
		goto clean;
	}

clean:
	Sel_free(&sel);
}

void MouseLUp()
{
	unsigned char edtool, edact, elevact, owner, ctype;
	Vec2uc from, to;
	Widget *gui;
	PlaceBlPacket pbp;
	Widget *csv;
	Widget *blv;
	Widget *tmv;
	Widget *savev;
	Widget *loadv;

	if(g_appmode == APPMODE_EDITOR)
	{
		edtool = GetEdTool();
		edact = GetEdAct();
		elevact = GetElevAct();

		if(edtool == TOOLCAT_ELEVATION)
		{
			from.x = g_vdrag[0].x/TILE_SIZE;
			from.y = g_vdrag[0].y/TILE_SIZE;
			to.x = g_mouse3d.x/TILE_SIZE;
			to.y = g_mouse3d.y/TILE_SIZE;

			if(elevact == ELEVACT_LOWER)
				LowerTerr(from, to);
			else if(elevact == ELEVACT_RAISE)
				RaiseTerr(from, to);
			else if(elevact == ELEVACT_SPREAD)
				SpreadTerr(from, to);
		}
		else if(edact == TOOLACT_PLACE)
		{
			ctype = GetEdCdType();
			owner = GetPlaceOwner();

			if(edtool == TOOLCAT_UNITS)
				EdPlaceUnit();
			else if(edtool == TOOLCAT_BUILDINGS)
				EdPlaceBl();
			else if(edtool == TOOLCAT_CONDUITS)
				PlaceCd(ctype, owner);
			else if(edtool == TOOLCAT_FOLIAGE)
				EdPlaceFol();
		}
		else if(edact == TOOLACT_DELETE)
		{
			EdDel();
		}
	}
	else if(g_appmode == APPMODE_PLAY)
	{
		if(g_build == BL_NONE)
		{
			gui = (Widget*)&g_gui;

			csv = Widget_get(gui, "cs view");
			blv = Widget_get(gui, "bl view");
			tmv = Widget_get(gui, "truck mgr");
			savev = Widget_get(gui, "save");
			loadv = Widget_get(gui, "load");

			Widget_hide(csv);
			Widget_hide(blv);
			Widget_hide(tmv);
			Widget_hide(savev);
			Widget_hide(loadv);

			DoSel(&g_sel);
			AfterSel(&g_sel);
		}
		else if(g_build < BL_TYPES)
		{
			if(g_canplace)
			{
				pbp.header.type = PACKET_PLACEBL;
				pbp.player = g_localP;
				pbp.btype = g_build;
				pbp.tpos.x = g_vdrag[0].x/TILE_SIZE;
				pbp.tpos.y = g_vdrag[0].y/TILE_SIZE;

				LockCmd((PacketHeader*)&pbp);
			}

			g_build = BL_NONE;
		}
		else if(g_build >= BL_TYPES && g_build < BL_TYPES+CD_TYPES)
		{
			PlaceCd(g_build - BL_TYPES, g_localP);
			g_build = BL_NONE;
		}
	}
}

void MouseMove(InEv* ie)
{
	Vec3i ray;
	Vec3i point;
	Vec3i line[2];
	unsigned char edtool, edact, elevact, ctype, owner;

	if(g_appmode == APPMODE_PLAY || 
		g_appmode == APPMODE_EDITOR)
	{
#ifdef PLATFORM_MOBILE
		if(g_mousekeys[MOUSE_LEFT] &&
			g_build == BL_NONE)
		{
			Scroll(-ie->dx, -ie->dy);
		}
#endif

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			CenterMouse();
		}

		IsoToCart(g_mouse+g_scroll, &ray, &point);

		Vec3i_muli(&line[0], ray, - MAX_MAP * 5 * TILE_SIZE);
		Vec3i_muli(&line[1], ray, MAX_MAP * 5 * TILE_SIZE);
		Vec3i_add(&line[0], line[0], point);
		Vec3i_add(&line[1], line[1], point);

		if(FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		{
			g_mouse3d = point;

			if(g_mousekeys[MOUSE_LEFT])
			{
				g_vdrag[1] = g_mouse3d;

				if(g_appmode == APPMODE_EDITOR &&
					GetEdTool() == TOOLCAT_BORDERS)
					EdPlaceBords();
			}
			else
			{
				g_vdrag[0] = g_mouse3d;
				g_vdrag[1] = g_mouse3d;
			}
		}

		UpdSBl();

		if(g_appmode == APPMODE_EDITOR)
		{
			edtool = GetEdTool();
			edact = GetEdAct();
			elevact = GetElevAct();
			ctype = GetEdCdType();
			owner = GetPlaceOwner();

			if(edtool == TOOLCAT_CONDUITS)
			{
				if(g_mousekeys[MOUSE_LEFT])
				{
					UpdCdPlans(ctype, owner, g_vdrag[0], g_mouse3d);
				}
			}
		}
	}
}

void MouseRDown()
{
	if(g_appmode == APPMODE_PLAY)
	{
		if(g_build != BL_NONE)
		{
			if(g_build > BL_TYPES && g_build < BL_TYPES+CD_TYPES)
				ClearCdPlans(g_build - BL_TYPES);

			g_build = BL_NONE;
		}
	}
}

void MouseRUp()
{
	if(g_appmode == APPMODE_PLAY)
	{
		Order(g_mouse.x, g_mouse.y, g_width, g_height);
	}
}

void Resize_Logo(Widget *w)
{
	float minsz = (float)120;

	w->pos[0] = g_width / 2 - minsz / 2;
	w->pos[1] = g_height / 2 - minsz / 2;
	w->pos[2] = g_width / 2 + minsz / 2;
	w->pos[3] = g_height / 2 + minsz / 2;
}

void Draw_LoadBar()
{
	float pct;
	float bar[4];
	Widget *gui;
	Shader* s;

	pct = (float)(g_lastLSp + g_lastLTex + g_lastRSp) / 
		(float)(g_spriteload.size() + g_texload.size() + g_spltorend.size());

	EndS();
	UseS(SHADER_COLOR2D);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

	bar[0] = (float)g_width/2 - 50;
	bar[1] = (float)g_height/2 + 30;
	bar[2] = (float)g_width/2 + 100;
	bar[3] = (float)g_height/2 + 30 + 3;

	gui = (Widget*)&g_gui;

	DrawSquare(0, 0.2f, 0, 0.9f, bar[0]-1, bar[1]-1, bar[2]+1, bar[3]+1, gui->crop);
	DrawSquare(0, 1, 0, 0.9f, bar[0], bar[1], bar[0] + pct * (bar[2]-bar[0]), bar[3], gui->crop);

	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
}

void AnyKey(int k)
{
	if(g_appmode == APPMODE_LOGO)
		SkipLogo();
}

void FillGUI()
{
	Widget *gui;
	ViewLayer *loading, *logoview;
	Image *bg, *logo;
	Text *statt;
	InsDraw *lbar;
	ViewLayer *renv;
	Viewport *renvp;

	gui = (Widget*)&g_gui;

	gui->keydownfunc[SDL_SCANCODE_F1] = SaveScreenshot;
	gui->keydownunc[SDL_SCANCODE_F2] = LogPathDebug;
	gui->lbuttondownfunc = MouseLDown;
	gui->lbuttonupfunc = MouseLUp;
	gui->rbuttondownfunc = MouseRDown;
	gui->rbuttonupfunc = MouseRUp);
	gui->mousemovefunc = MouseMov;
	gui->anykeydownfunc = AnyKey;

	/* Logo ViewLayer */

	logoview = (ViewLayer*)malloc(sizeof(ViewLayer));
	bg = (Image*)malloc(sizeof(Image));
	logo = (Image*)malloc(sizeof(Image));

	ViewLayer_init(logoview, gui, "logo");
	Image_init(bg, (Widget*)logoview, "", "gui/mmbg.jpg", ectrue, Resize_Fullscreen, 1,1,1,1);
	Image_init(logo, (Widget*)logoview, "logo", "gui/centerp/dmd.png", ectrue, Resize_Logo, 1, 1, 1, 0));

	Widget_add(gui, (Widget*)logoview);
	Widget_add((Widget*)logoview, (Widget*)bg);
	Widget_add((Widget*)logoview, (Widget*)logo);


	/* Loading ViewLayer */

	loading = (ViewLayer*)malloc(sizeof(ViewLayer));
	bg = (Image*)malloc(sizeof(Image));
	statt = (Text*)malloc(sizeof(Text));
	lbar = (InsDraw*)malloc(sizeof(InsDraw));

	ViewLayer_init(loading, gui, "loading");
	Image_init(bg, (Widget*)loading, "", "gui/mmbg.jpg", ectrue, Resize_Fullscreen);
	Text_init(statt, (Widget*)loading, "status", "...", MAINFONT8, Resize_Status);
	InsDraw_init(lbar, (Widget*)loading, Draw_LoadBar);

	Widget_add(gui, (Widget*)loading);
	Widget_add((Widget*)loading, (Widget*)bg);
	Widget_add((Widget*)loading, (Widget*)statt);
	Widget_add((Widget*)loading, (Widget*)lbar);

	FillMenu();
	FillEd();	/* must be done after QueueSimRes simdef's */

	VpType_init(&g_vptype[VP_FRONT], 0,0,MAX_DISTANCE/3, 0,1,0, "Front");
	VpType_init(&g_vptype[VP_FRONT], 0,0,MAX_DISTANCE/3, 0,1,0, "Front");
	VpType_init(&g_vptype[VP_TOP], 0,MAX_DISTANCE/3,0, 0,0,-1, "Top");
	VpType_init(&g_vptype[VP_LEFT], MAX_DISTANCE/3,0,0, 0,1,0, "Left");
	VpType_init(&g_vptype[VP_ANGLE45O], 1000.0f/3,1000.0f/3,1000.0f/3, 0,1,0, "Angle");
	ResetView(ecfalse);

	VpWrap_init(&g_vpwrap[0], VP_FRONT);
	VpWrap_init(&g_vpwrap[1], VP_TOP);
	VpWrap_init(&g_vpwrap[2], VP_LEFT);
	VpWrap_init(&g_vpwrap[3], VP_ANGLE45O);

	renv = (ViewLayer*)malloc(sizeof(ViewLayer));
	renvp = (Viewport*)malloc(sizeof(Viewport));

	ViewLayer_init((Widget*)renv, gui, "render");
	Viewport_init((Widget*)renvp, (Widget*)renv, "render viewport", Resize_FullViewport, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3);

	Widget_add(gui, (Widget*)renv);
	Widget_add((Widget*)renv, (Widget*)renvp);


	FillPlay();

	FillConsole();
	FillMess();

	Widget_hideall(gui);
	Widget_show(Widget_get(gui, "logo");
}
