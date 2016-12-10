


#include "appmain.h"
#include "../math/3dmath.h"
#include "../gui/gui.h"
#include "../texture.h"
#include "segui.h"
#include "../window.h"
#include "seviewport.h"
#include "sesim.h"
#include "../save/save.h"
#include "../save/saveedm.h"
#include "../sim/sim.h"
#include "../render/sortb.h"
#include "undo.h"
#include "../save/compilemap.h"
#include "../render/shadow.h"
#include "../sim/simtile.h"
#include "../save/modelholder.h"
#include "../tool/compilebl.h"
#include "../save/savesprite.h"
#include "../tool/rendersprite.h"
#include "../tool/rendertopo.h"
#include "../render/screenshot.h"
//#include "../gui/widgets/choosefile.h"
#include "../render/model2.h"
#include "../gui/keymap.h"
#include "../gui/layouts/appgui.h"

#ifdef PLATFORM_MAC
#include "filechooser_mac.h"
#endif

int g_projtype = PROJ_ORTHO;
ecbool g_showsky = ecfalse;
float g_snapgrid = 25;
//char g_lastsave[SFH_MAX_PATH+1];

void AnyKeyDown(int k)
{
	SkipLogo();
}

void AnyKeyUp(int k)
{
}

void Delete()
{
	if(g_appmode != APPMODE_EDITOR)
		return;

	if(g_selB.size() > 0 || g_selM.size() > 0)
		LinkPrevUndo();

	for(int i=0; i<g_selB.size(); i++)
	{
		for(std::list<Brush>::iterator j=g_edmap.brush.begin(); j!=g_edmap.brush.end(); j++)
		{
			if(g_selB[i] == &*j)
			{
				g_edmap.brush.erase(j);
				break;
			}
		}
	}

	g_selB.clear();
	g_sel1b = NULL;

	CloseSideView();

	for(int i=0; i<g_selM.size(); i++)
	{
		for(std::list<ModelHolder>::iterator j=g_modelholder.begin(); j!=g_modelholder.end(); j++)
		{
			if(g_selM[i] == &*j)
			{
				g_modelholder.erase(j);
				break;
			}
		}
	}

	g_selM.clear();
	g_sel1m = NULL;

	//LinkLatestUndo();
	//Log("delete b");
	//
}

void MouseLeftButtonDown()
{
	if(g_appmode == APPMODE_LOGO)
		SkipLogo();
	else if(g_appmode == APPMODE_EDITOR)
	{

	}
}

void MouseLeftButtonUp()
{
}

void Change_RotDeg(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
}

void Change_Zoom(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
	Widget* zoombox = g_gui.get("editor")->get("top panel")->get("zoom");

	//if(zoombox->value.c_str()[0] == '\0')
	//	return;

	g_zoom = StrToFloat(zoombox->value.rawstr().c_str());

	if(g_zoom <= 0.0f)
		g_zoom = 1.0f;
}

//void Change_SnapGrid(int dummy)
void Change_SnapGrid()
{
	Widget* snapgbox = g_gui.get("editor")->get("top panel")->get("snapgrid");

	//if(snapgbox->value.c_str()[0] == '\0')
	//	return;

	//g_snapgrid = StrToFloat(snapgbox->value.c_str());
	//int power = snapgbox->selected;
	//g_snapgrid = 400 / pow(2, power);

	float cscales[] = CM_SCALES;

	g_snapgrid = cscales[ snapgbox->selected ];

	if(g_snapgrid <= 0.0f)
		g_snapgrid = 1;
}

void Change_MaxElev(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
	Widget* maxelevbox = g_gui.get("editor")->get("top panel")->get("maxelev");

	g_maxelev = StrToFloat(maxelevbox->value.rawstr().c_str());
}

void UpdateLogo()
{
	static int stage = 0;
	//Log("update logo "<<stage<<std::endl;
	//

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		g_gui.get("logo")->get("logo")->rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		g_gui.get("logo")->get("logo")->rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

void Click_NewBrush()
{
	LinkPrevUndo();
	//Vec3f pos = g_focus;
	Vec3f pos = g_cam.view;
	pos.x = Snap(g_snapgrid, pos.x);
	pos.y = Snap(g_snapgrid, pos.y);
	pos.z = Snap(g_snapgrid, pos.z);
	Brush b;
	BrushSide top(Vec3f(0,1,0), pos + Vec3f(0,STOREY_HEIGHT,0));
	BrushSide bottom(Vec3f(0,-1,0), pos + Vec3f(0,0,0));
	BrushSide left(Vec3f(-1,0,0), pos + Vec3f(-g_tilesize/2.0f,0,0));
	BrushSide right(Vec3f(1,0,0), pos + Vec3f(g_tilesize/2.0f,0,0));
	BrushSide front(Vec3f(0,0,1), pos + Vec3f(0,0,g_tilesize/2.0f));
	BrushSide back(Vec3f(0,0,-1), pos + Vec3f(0,0,-g_tilesize/2.0f));
	b.add(top);		//0
	b.add(bottom);	//1
	b.add(left);	//2
	b.add(right);	//3
	b.add(front);	//4
	b.add(back);	//5
	b.collapse();
	b.remaptex();
	//Log("---------push back brush-----------");
	g_edmap.brush.push_back(b);
	//g_selB.clear();
	//std::list<Brush>::iterator i = g_edmap.brush.rbegin();
	//g_selB.push_back(&*i);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
	//LinkLatestUndo();
}

void Click_LoadEdMap()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	FreeEdMap(&g_edmap);

	if(LoadEdMap(filepath, &g_edmap))
	{
		//InfoMess("L","L");
		strcpy(g_lastsave, filepath);
	}

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
	ClearUndo();
}

void Click_SaveEdMap()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	SaveEdMap(filepath, &g_edmap);
	strcpy(g_lastsave, filepath);
}

void Click_QSaveEdMap()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveEdMap();
		return;
	}

	SaveEdMap(g_lastsave, &g_edmap);
}

void Click_CompileMap()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
}

void Click_ExportBuildingSprite()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\building base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
/////	PrepareRender(filepath, RENDER_BUILDING);
#endif
}

void Click_Export()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\sprite base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
/////	PrepareRender(filepath, RENDER_UNSPEC);
}

void Click_ExportTopo()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("topologies\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("topologies\\sprite base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	PrepareTopo(filepath, RENDER_UNSPEC);
}

void Click_ExportUnitSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\unit base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
/////	PrepareRender(filepath, RENDER_UNIT);
#endif
}

void Click_ExportTileSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\tile base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
//////	PrepareRender(filepath, RENDER_TERRTILE);
#endif
}

void Click_ExportRoadSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\road base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
//////	PrepareRender(filepath, RENDER_ROAD);
#endif
}

void RunMap(const char* full)
{
	// TO DO: launch game

	char relativeexe[] = "BattleComputer.exe";
	char fullexe[1024];
	FullPath(relativeexe, fullexe);
	char relativemap[] = "temp/tempmap";
	char fullmap[1024];
	FullPath(relativemap, fullmap);

	char cmd[2048];
	sprintf(cmd, "%s +devmap %s", fullexe, fullmap);
	system(cmd);
}

void Click_CompileRunMap()
{
	/*
	OPENFILENAME ofn;

	char filepath[SFH_MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[SFH_MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = SFH_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
	*/

	char filepath[SFH_MAX_PATH+1];
	FullPath("temp/tempmap", filepath);
	CompileMap(filepath, &g_edmap);
	RunMap(filepath);
}

void Click_BChooseTex()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("textures\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("textures\\texture", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	LinkPrevUndo();

	unsigned int diffuseindex;
	std::string relativepath = MakeRelative(filepath);
	CreateTex(diffuseindex, relativepath.c_str(), ecfalse, ecfalse);
	unsigned int texname = g_texture[diffuseindex].texname;

	if(diffuseindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath.c_str());

		ErrMess("Error", msg);
	}

	char specpath[SFH_MAX_PATH+1];
	strcpy(specpath, relativepath.c_str());
	StripExt(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	CreateTex(specindex, specpath, ecfalse, ecfalse);

	if(specindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrMess("Error", msg);
	}

	char normpath[SFH_MAX_PATH+1];
	strcpy(normpath, relativepath.c_str());
	StripExt(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	CreateTex(normindex, normpath, ecfalse, ecfalse);

	if(normindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrMess("Error", msg);
	}

	char ownpath[SFH_MAX_PATH+1];
	strcpy(ownpath, relativepath.c_str());
	StripExt(ownpath);
	strcat(ownpath, ".team.png");

	unsigned int ownindex;
	CreateTex(ownindex, ownpath, ecfalse, ecfalse);

	if(ownindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", ownpath);

		ErrMess("Error", msg);
	}

	if(g_sel1b == NULL)
	{
		for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			b->texture = diffuseindex;

			for(int j=0; j<b->nsides; j++)
			{
				BrushSide* s = &b->sides[j];
				s->diffusem = diffuseindex;
				s->specularm = specindex;
				s->normalm = normindex;
				s->ownerm = ownindex;
			}
		}
	}
	else
	{
		Brush* b = g_sel1b;

		b->texture = diffuseindex;

		if(g_dragS >= 0)
		{
			BrushSide* s = &b->sides[g_dragS];
			s->diffusem = diffuseindex;
			s->specularm = specindex;
			s->normalm = normindex;
			s->ownerm = ownindex;
		}
		else
		{
			for(int i=0; i<b->nsides; i++)
			{
				BrushSide* s = &b->sides[i];
				s->diffusem = diffuseindex;
				s->specularm = specindex;
				s->normalm = normindex;
				s->ownerm = ownindex;
			}
		}
	}
}

void Click_FitToFace()
{
	LinkPrevUndo();

	if(g_sel1b == NULL)
	{
		for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			for(int j=0; j<b->nsides; j++)
				b->sides[j].fittex();
		}
	}
	else
	{
		Brush* b = g_sel1b;

		if(g_dragS >= 0)
			b->sides[g_dragS].fittex();
		else
		{
			for(int i=0; i<b->nsides; i++)
				b->sides[i].fittex();
		}
	}

	RedoBSideGUI();
}

void CloseSideView()
{
	g_gui.hide("brush side edit");
	g_gui.hide("brush edit");
	g_gui.hide("door edit");
}

void Click_DoorView()
{
	CloseSideView();
#if 0
	OpenAnotherView("door edit");
#endif
}

void CopyBrush()
{
	//InfoMess("cp","cb");

	if(g_selB.size() <= 0 && g_selM.size() <= 0)
		return;

	if(g_selB.size() > 0)
	{
		g_copyB = *g_selB[0];
		g_copyM.modeli = -1;
	}
	else if(g_selM.size() > 0)
	{
		g_copyM = *g_selM[0];
		g_copyB.nsides = 0;
	}

	//Log("copy brush");
	//
}

void PasteBrush()
{
	//Log("paste brush?");
	//

	if(g_copyB.nsides > 0)
	{
		LinkPrevUndo();

		//Vec3f pos = g_focus;
		Vec3f pos = g_cam.view;
		pos.x = Snap(g_snapgrid, pos.x);
		pos.y = Snap(g_snapgrid, pos.y);
		pos.z = Snap(g_snapgrid, pos.z);
		Brush b = g_copyB;
		//b.moveto(pos);
	/*
		for(int i=0; i<b.nsides; i++)
		{
			BrushSide* s = &b.sides[i];

			Log("side"<<i<<" plane="<<s->plane.normal.x<<","<<s->plane.normal.y<<","<<s->plane.normal.z<<","<<s->plane.d<<std::endl;
		}
		*/
		g_edmap.brush.push_back(b);

		//Log("paste brush");
		//
	}
	else if(g_copyM.modeli >= 0)
	{
		LinkPrevUndo();

		g_modelholder.push_back(g_copyM);
	}
}

void Down_C()
{
	if(g_appmode != APPMODE_EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		CopyBrush();
}

void Down_V()
{
	if(g_appmode != APPMODE_EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		PasteBrush();
}

void Click_CutBrush()
{
	g_edtool = EDTOOL_CUT;
}

ecbool OpenFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN
	memset(filepath, 0, sizeof(char)*SFH_MAX_PATH);

	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(g_window, &info);

	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	//ofn.hwndOwner = info.info.win.window;
	ofn.hwndOwner = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = SFH_MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = SFH_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
	{
#if 0
		int e = CommDlgExtendedError();
		char msg[128];
		sprintf(msg, "cmdlge %d", e);
		InfoMess(msg,msg);
#endif

		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

		return ecfalse;
	}
	
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

#if 0
	int e = CommDlgExtendedError();
	char msg[128];
	sprintf(msg, "cmdlgne %d", e);
	InfoMess(msg,msg);
#endif

	return ectrue;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    NULL);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[SFH_MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

		
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

        return ectrue;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

	return ecfalse;

#elif defined( PLATFORM_MAC )
	
	return OpenFileDialog_Mac(initdir, filepath);
	
#endif
}

ecbool SaveFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN
	memset(filepath, 0, sizeof(char)*SFH_MAX_PATH);

	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(g_window, &info);

	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	//ofn.hwndOwner = info.info.win.window;
	ofn.hwndOwner = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = SFH_MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = SFH_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
	{
		
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

		return ecfalse;
	}

	
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

	return ectrue;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Save File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
    NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[SFH_MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

		
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

        return ectrue;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	
		//temp
		InEv ie;
		ie.type = INEV_MOUSEUP;
		ie.key = 0;
		//g_gui.inev(&ie);

	return ecfalse;
	
#elif defined( PLATFORM_MAC )
	
	return SaveFileDialog_Mac(initdir, filepath);
	
#endif
}

void Click_AddMS3D()
{
	char filepath[SFH_MAX_PATH+1];

	char initdir[SFH_MAX_PATH+1];
	FullPath("models\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("models\\model", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	std::string relative = MakeRelative(filepath);

	Log("relative %s", relative.c_str());

	int modelid = LoadModel2(relative.c_str(), Vec3f(1,1,1), Vec3f(0,0,0), ectrue);

	if(modelid < 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load model %s", relative.c_str());

		ErrMess("Error", msg);

		return;
	}

	LinkPrevUndo();

	ModelHolder mh(modelid, Vec3f(0,0,0));
	g_modelholder.push_back(mh);
}

void RotateModels(float degrees, float xaxis, float yaxis, float zaxis)
{
	LinkPrevUndo();

	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		pmh->rotdegrees = pmh->rotdegrees + Vec3f( degrees*xaxis, degrees*yaxis, degrees*zaxis );
		pmh->retransform();
	}
}

void RotateBrushes(float radians, Vec3f axis)
{
	LinkPrevUndo();
	//EdMap* m = &g_edmap;

	//for(std::list<Brush>::iterator i=map->brush.begin(); i!=map->brush.end(); i++)
	for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;
		Vec3f centroid(0,0,0);

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* s = &b->sides[j];

			centroid = centroid + s->centroid;
		}

		centroid = centroid / (float)b->nsides;

		std::list<float> oldus;
		std::list<float> oldvs;

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* s = &b->sides[j];

			RotatePlane(s->plane, centroid, radians, axis);
			//RotatePlane(s->tceq[0], centroid, radians, axis);
			//RotatePlane(s->tceq[1], centroid, radians, axis);
			//s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
			//s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);
			//RotatePlane(s->tceq[0], s->centroid, radians, axis);
			//RotatePlane(s->tceq[1], s->centroid, radians, axis);
			//Vec3f sharedv = b->sharedv[ s->vindices[0] ];
			//s->tceq[0].normal = RotateAround(s->tceq[0].normal, sharedv, radians, axis.x, axis.y, axis.z);
			//s->tceq[1].normal = RotateAround(s->tceq[1].normal, sharedv, radians, axis.x, axis.y, axis.z);
			//Vec3f sharedv = b->sharedv[ s->vindices[0] ];
			//Vec3f offcenter = sharedv - centroid;
			//Vec3f newoffcenter = Rotate(offcenter, radians, axis.x, axis.y, axis.z);
			//float uminusd = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z;
			//float vminusd = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z;
			//s->tceq[0].normal = Rotate(s->tceq[0].normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->tceq[1].normal = Rotate(s->tceq[1].normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
			//s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);
			//float newuminusd = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z;
			//float newvminusd = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z;
			//float uscale = uminusd / newuminusd;
			//float vscale = vminusd / newvminusd;
			//s->tceq[0].normal = s->tceq[0].normal * uscale;
			//s->tceq[1].normal = s->tceq[1].normal * vscale;
			/*
			Vec3f sharedv = b->sharedv[ s->vindices[0] ];
			float oldu = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float oldv = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
			s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);
			float newu = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float newv = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			float changeu = newu - oldu;
			float changev = newv - oldv;
			s->tceq[0].d += changeu;
			s->tceq[1].d += changev;
			*/
			Vec3f sharedv = b->sharedv[ s->vindices[0] ];
			float u = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float v = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
			s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);
		}

		b->collapse();

		std::list<float>::iterator oldu = oldus.begin();
		std::list<float>::iterator oldv = oldvs.begin();

		for(int j=0; j<b->nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &b->sides[j];

			Vec3f newsharedv = b->sharedv[ s->vindices[0] ];

			float newu = newsharedv.x*s->tceq[0].normal.x + newsharedv.y*s->tceq[0].normal.y + newsharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float newv = newsharedv.x*s->tceq[1].normal.x + newsharedv.y*s->tceq[1].normal.y + newsharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->tceq[0].d -= changeu;
			s->tceq[1].d -= changev;
		}

		b->remaptex();
	}
}

void Click_RotXCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 1, 0, 0);
}

void Click_RotXCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 1, 0, 0);
}

void Click_RotYCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 1, 0);
}

void Click_RotYCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 1, 0);
}

void Click_RotZCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 0, 1);
}

void Click_RotZCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 0, 1);
}

void Click_RotateTex()
{
	LinkPrevUndo();

	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->sides[g_dragS];

	//Vec3f sharedv = b->sharedv[ s->vindices[0] ];
	float oldu = s->centroid.x*s->tceq[0].normal.x + s->centroid.y*s->tceq[0].normal.y + s->centroid.z*s->tceq[0].normal.z + s->tceq[0].d;
	float oldv = s->centroid.x*s->tceq[1].normal.x + s->centroid.y*s->tceq[1].normal.y + s->centroid.z*s->tceq[1].normal.z + s->tceq[1].d;
	Vec3f axis = s->plane.normal;
	float radians = DEGTORAD(degrees);
	s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
	s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);

	//Vec3f newsharedv = b->sharedv[ s->vindices[0] ];
	float newu = s->centroid.x*s->tceq[0].normal.x + s->centroid.y*s->tceq[0].normal.y + s->centroid.z*s->tceq[0].normal.z + s->tceq[0].d;
	float newv = s->centroid.x*s->tceq[1].normal.x + s->centroid.y*s->tceq[1].normal.y + s->centroid.z*s->tceq[1].normal.z + s->tceq[1].d;
	float changeu = newu - oldu;
	float changev = newv - oldv;
	s->tceq[0].d -= changeu;
	s->tceq[1].d -= changev;

	s->remaptex();
}

void RedoBSideGUI()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	if(!g_gui.get("brush side edit"))	InfoMess("e", "no bse");
	if(!g_gui.get("brush side edit")->get("left panel"))	InfoMess("e", "no lp");
	if(!g_gui.get("brush side edit")->get("left panel")->get("u equation"))	InfoMess("e", "no ue");
	if(!g_gui.get("brush side edit")->get("left panel")->get("v equation"))	InfoMess("e", "no ve");

	EditBox* uwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("v equation");

	Plane3f* tceq = g_sel1b->sides[g_dragS].tceq;

	char tceqstr[256];
	sprintf(tceqstr, "%f %f %f %f", tceq[0].normal.x, tceq[0].normal.y, tceq[0].normal.z, tceq[0].d);
	RichText tceqrstr = RichText(tceqstr);
	uwidg->changevalue(&tceqrstr);
	sprintf(tceqstr, "%f %f %f %f", tceq[1].normal.x, tceq[1].normal.y, tceq[1].normal.z, tceq[1].d);
	tceqrstr = RichText(tceqstr);
	vwidg->changevalue(&tceqrstr);

	uwidg->scroll[0] = 0;
	vwidg->scroll[0] = 0;
}

void Change_TexEq(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	LinkPrevUndo();

	EditBox* uwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("v equation");

	float A = 0;
	float B = 0;
	float C = 0;
	float D = 0;

	sscanf(uwidg->value.rawstr().c_str(), "%f %f %f %f", &A, &B, &C, &D);

	Plane3f* tceq = g_sel1b->sides[g_dragS].tceq;
	tceq[0].normal.x = A;
	tceq[0].normal.y = B;
	tceq[0].normal.z = C;
	tceq[0].d = D;

	A = 0;
	B = 0;
	C = 0;
	D = 0;

	sscanf(vwidg->value.rawstr().c_str(), "%f %f %f %f", &A, &B, &C, &D);

	tceq[1].normal.x = A;
	tceq[1].normal.y = B;
	tceq[1].normal.z = C;
	tceq[1].d = D;

	g_sel1b->sides[g_dragS].remaptex();
}

void Change_SelComp()
{
	//RedoBSideGUI();
}

int GetComponent()
{
	int c = g_gui.get("brush side edit")->get("left panel")->get("select component")->selected;

	if(c != 0 && c != 1)
		c = 0;

	return c;
}

void Click_ScaleTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	LinkPrevUndo();

	int c = GetComponent();

	float scale = 1.0f/StrToFloat(g_gui.get("brush side edit")->get("left panel")->get("texture scale")->value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->sides[g_dragS];

	float oldcomp = s->centroid.x*s->tceq[c].normal.x + s->centroid.y*s->tceq[c].normal.y + s->centroid.z*s->tceq[c].normal.z + s->tceq[c].d;
	s->tceq[c].normal = s->tceq[c].normal * scale;

	//Vec3f newsharedv = b->sharedv[ s->vindices[0] ];
	float newcomp = s->centroid.x*s->tceq[c].normal.x + s->centroid.y*s->tceq[c].normal.y + s->centroid.z*s->tceq[c].normal.z + s->tceq[c].d;
	float changecomp = newcomp - oldcomp;
	s->tceq[c].d -= changecomp;

	s->remaptex();

	RedoBSideGUI();
}

void Click_ShiftTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	LinkPrevUndo();

	int c = GetComponent();

	float shift = StrToFloat(g_gui.get("brush side edit")->get("left panel")->get("texture shift")->value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->sides[g_dragS];

	s->tceq[c].d += shift;

	s->remaptex();

	RedoBSideGUI();
}

void Change_ShowSky()
{
	Widget* showskchbox = g_gui.get("editor")->get("top panel")->get("showsky");

	g_showsky = (ecbool)showskchbox->selected;
}

void Click_ProjPersp()
{
	g_projtype = PROJ_PERSP;
}

void Click_ProjOrtho()
{
	g_projtype = PROJ_ORTHO;
}

void Click_ResetView()
{
	ResetView(ecfalse);
}

void Click_Explode()
{
	g_edtool = EDTOOL_EXPLOSION;
}

void Click_SetDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	LinkPrevUndo();

	Brush* b = *g_selB.begin();
	EdDoor* door = b->door;

	if(!b->door)
	{
		b->door = new EdDoor();
		door = b->door;

		door->axis = Vec3f(0,200,0);
		door->point = b->sides[2].centroid;
		door->startopen = ecfalse;
	}

	float opendeg = 90;

	ViewLayer* dooredview = (ViewLayer*)g_gui.get("door edit");
	Frame* leftpanel = (Frame*)dooredview->get("left panel");
	EditBox* opendegedit = (EditBox*)leftpanel->get("opendeg");
	opendeg = StrToFloat(opendegedit->value.rawstr().c_str());

	door->opendeg = opendeg;

	b->getsides(&door->nsides, &door->sides);

#if 0
	Log("set ed door");
	for(int i=0; i<door->nsides; i++)
	{
		Log("side "<<i<<std::endl;
		Plane3f* p = &door->sides[i].plane;

		Log("plane = "<<p->normal.x<<","<<p->normal.y<<","<<p->normal.z<<",d="<<p->d<<std::endl;
	}
#endif
}

void Click_UnmakeDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	Brush* b = *g_selB.begin();

	if(b->door)
	{
		LinkPrevUndo();
		delete [] b->door;
		b->door = NULL;
	}
}

void Click_OpenCloseDoor()
{
	if(g_selB.size() <= 0)
		return;

	Brush* b = *g_selB.begin();
	EdDoor* door = b->door;

	if(!b->door)
		return;

	LinkPrevUndo();

	Brush transformed;
	transformed.setsides(door->nsides, door->sides);
	transformed.collapse();
	transformed.remaptex();

	door->startopen = !door->startopen;

	if(door->startopen)
	{
		//transform

		Vec3f centroid = door->point;
		Vec3f axis = Normalize(door->axis);
		float radians = DEGTORAD(door->opendeg);

		//char msg[128];
		//sprintf(msg, "point(%f,%f,%f) axis(%f,%f,%f)

		std::list<float> oldus;
		std::list<float> oldvs;

		for(int j=0; j<transformed.nsides; j++)
		{
			BrushSide* s = &transformed.sides[j];
			RotatePlane(s->plane, centroid, radians, axis);

			Vec3f sharedv = transformed.sharedv[ s->vindices[0] ];
			float u = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float v = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
			s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);
		}

		//transformed.collapse();

		std::list<float>::iterator oldu = oldus.begin();
		std::list<float>::iterator oldv = oldvs.begin();

		for(int j=0; j<transformed.nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &transformed.sides[j];

			Vec3f newsharedv = transformed.sharedv[ s->vindices[0] ];

			float newu = newsharedv.x*s->tceq[0].normal.x + newsharedv.y*s->tceq[0].normal.y + newsharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float newv = newsharedv.x*s->tceq[1].normal.x + newsharedv.y*s->tceq[1].normal.y + newsharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->tceq[0].d -= changeu;
			s->tceq[1].d -= changev;
		}

		//transformed.remaptex();
	}

	b->setsides(transformed.nsides, transformed.sides);
	b->collapse();
	b->remaptex();
}

void Resize_LoadingText(Widget* w)
{
	w->pos[0] = g_width * 0.4f;
	w->pos[1] = g_height * 0.5f;
}

void Resize_TopPanel(Widget* w)
{
	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = g_width;
	w->pos[3] = TOP_PANEL_HEIGHT;
}

void Resize_LeftPanel(Widget* w)
{
	w->pos[0] = 0;
	w->pos[1] = TOP_PANEL_HEIGHT;
	w->pos[2] = LEFT_PANEL_WIDTH;
	w->pos[3] = g_height;
}

void Resize_LoadButton(Widget* w)
{
	int i = 0;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;

#if 0
	char msg[128];
	sprintf(msg, "pos:%f,%f,%f,%f", w->pos[0], w->pos[1], w->pos[2], w->pos[3]);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif
}

void Resize_SaveButton(Widget* w)
{
	int i = 1;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_QSaveButton(Widget* w)
{
	int i = 2;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_CompileMapButton(Widget* w)
{
	int i = 3;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ExportBldgButton(Widget* w)
{
	int i = 4;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

//
void Resize_ExportTopoButton(Widget* w)
{
	int i = 3;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}
void Resize_ExportButton(Widget* w)
{
	int i = 4;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}
void Resize_InclinesCheck(Widget* w)
{
	int i = 5;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 64+32*i;
	w->pos[3] = 16;
}
void Resize_SidesCheck(Widget* w)
{
	int i = 5;
	w->pos[0] = 0+32*i;
	w->pos[1] = 16;
	w->pos[2] = 64+32*i;
	w->pos[3] = 32;
}
void Resize_FramesCheck(Widget* w)
{
	int i = 7;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 64+32*i;
	w->pos[3] = 16;
}
void Resize_RotationsCheck(Widget* w)
{
	int i = 7;
	w->pos[0] = 0+32*i;
	w->pos[1] = 16;
	w->pos[2] = 64+32*i;
	w->pos[3] = 32;
}
//

void Resize_ExportUnitButton(Widget* w)
{
	int i = 5;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ExportTileButton(Widget* w)
{
	int i = 6;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ExportRoadButton(Widget* w)
{
	int i = 7;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ExportRidgeButton(Widget* w)
{
	int i = 8;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ExportWaterButton(Widget* w)
{
	int i = 9;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_UndoButton(Widget* w)
{
	int i = 10;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RedoButton(Widget* w)
{
	int i = 11;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_NewBrushButton(Widget* w)
{
	int i = 12;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_CutBrushButton(Widget* w)
{
	int i = 13;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_NewEntityButton(Widget* w)
{
	int i = 14;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotXCCWButton(Widget* w)
{
	int i = 15;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotXCWButton(Widget* w)
{
	int i = 16;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotYCCWButton(Widget* w)
{
	int i = 17;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotYCWButton(Widget* w)
{
	int i = 18;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotZCCWButton(Widget* w)
{
	int i = 19;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_RotZCWButton(Widget* w)
{
	int i = 20;
	w->pos[0] = 0+32*i;
	w->pos[1] = 0;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32;
}

void Resize_ViewportsFrame(Widget* w)
{
	w->pos[0] = LEFT_PANEL_WIDTH;
	w->pos[1] = TOP_PANEL_HEIGHT;
	w->pos[2] = g_width;
	w->pos[3] = g_height;
}

void Resize_TopLeftViewport(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = parentw->pos[0];
	w->pos[1] = parentw->pos[1];
	w->pos[2] = (parentw->pos[2]+parentw->pos[0])/2;
	w->pos[3] = (parentw->pos[3]+parentw->pos[1])/2;
}

void Resize_BottomLeftViewport(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = parentw->pos[0];
	w->pos[1] = (parentw->pos[3]+parentw->pos[1])/2;
	w->pos[2] = (parentw->pos[2]+parentw->pos[0])/2;
	w->pos[3] = parentw->pos[3];
}

void Resize_TopRightViewport(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = (parentw->pos[2]+parentw->pos[0])/2;
	w->pos[1] = parentw->pos[1];
	w->pos[2] = parentw->pos[2];
	w->pos[3] = (parentw->pos[3]+parentw->pos[1])/2;
}

void Resize_BottomRightViewport(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = (parentw->pos[2]+parentw->pos[0])/2;
	w->pos[1] = (parentw->pos[3]+parentw->pos[1])/2;
	w->pos[2] = parentw->pos[2];
	w->pos[3] = parentw->pos[3];
}

void Resize_FullViewport(Widget* w)
{
	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = g_width;
	w->pos[3] = g_height;
}

void Resize_HDivider(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = parentw->pos[0];
	w->pos[1] = (parentw->pos[3]+parentw->pos[1])/2;
	w->pos[2] = parentw->pos[2];
	w->pos[3] = (parentw->pos[3]+parentw->pos[1])/2 + 1;
}

void Resize_VDivider(Widget* w)
{
	Widget* parentw = w->parent;
	w->pos[0] = (parentw->pos[2]+parentw->pos[0])/2;
	w->pos[1] = parentw->pos[1];
	w->pos[2] = (parentw->pos[2]+parentw->pos[0])/2 + 1;
	w->pos[3] = parentw->pos[3];
}

void Resize_RotDegEditBox(Widget* w)
{
	int i = 0;
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32+16;
}

void Resize_RotDegText(Widget* w)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 2;
	w->pos[0] = 0+32*(i-2);
	w->pos[1] = 32+16;
	w->pos[2] = w->pos[0] + 164;
	w->pos[3] = w->pos[1] + 164;
}

void Resize_ZoomEditBox(Widget* w)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 2;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32+16;
}

void Resize_ZoomText(Widget* w)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 4;
	w->pos[0] = 0+32*(i-2);
	w->pos[1] = 32+16;
	w->pos[2] = w->pos[0] + 164;
	w->pos[3] = w->pos[1] + 164;
}

void Resize_SnapGridEditBox(Widget* w)
{
	//Margin(0+32*i++), Margin(32), Margin(50+32*i++), Margin(32+16)
	int i = 4;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 50+32*i;
	w->pos[3] = 32+16;
}

void Resize_SnapGridText(Widget* w)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 6;
	w->pos[0] = 0+32*(i-2);
	w->pos[1] = 32+16;
	w->pos[2] = w->pos[0] + 164;
	w->pos[3] = w->pos[1] + 164;
}

void Resize_MaxElevEditBox(Widget* w)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 6;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32+16;
}

void Resize_MaxElevText(Widget* w)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 8;
	w->pos[0] = 0+32*(i-2);
	w->pos[1] = 32+16;
	w->pos[2] = w->pos[0] + 164;
	w->pos[3] = w->pos[1] + 164;
}

void Resize_ShowSkyCheckBox(Widget* w)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 8;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32+16;
}

void Resize_PerspProjButton(Widget* w)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 10;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32*2;
	CenterLabel(w);
}

void Resize_OrthoProjButton(Widget* w)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 11;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32*2;
	CenterLabel(w);
}

void Resize_ResetViewButton(Widget* w)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 12;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32*2;
	CenterLabel(w);
}

void Resize_FramesText(Widget* w)
{
	int i = 13;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32 + 16;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32*2;
}

void Resize_FramesEditBox(Widget* w)
{
	int i = 13;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 16;
}

void Resize_ExplodeButton(Widget* w)
{
	int i = 14;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 32;
	CenterLabel(w);
}

void Resize_AddTileButton(Widget* w)
{
	int i = 15;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 32;
	CenterLabel(w);
}

void Resize_LeadsNECheckBox(Widget* w)
{
	int i = 16;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 16;
	CenterLabel(w);
}

void Resize_LeadsSECheckBox(Widget* w)
{
	int i = 16;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32 + 16;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 32;
	CenterLabel(w);
}

void Resize_LeadsSWCheckBox(Widget* w)
{
	int i = 18;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 16;
	CenterLabel(w);
}

void Resize_LeadsNWCheckBox(Widget* w)
{
	int i = 18;
	w->pos[0] = 0+32*i;
	w->pos[1] = 32 + 16;
	w->pos[2] = 32+32*i;
	w->pos[3] = 32 + 32;
	CenterLabel(w);
}

void Change_Leads()
{
}

void Click_AddTile()
{
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("textures\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("textures\\texture", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	LinkPrevUndo();

	unsigned int diffuseindex;
	std::string relativepath = MakeRelative(filepath);
	CreateTex(diffuseindex, relativepath.c_str(), ecfalse, ecfalse);	//with mipmaps, linear filter
	//CreateTex(diffuseindex, relativepath.c_str(), ecfalse, ecfalse);	//no mipmaps, nearest filter
	unsigned int texname = g_texture[diffuseindex].texname;

	if(diffuseindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath.c_str());

		ErrMess("Error", msg);
	}

	char specpath[SFH_MAX_PATH+1];
	strcpy(specpath, relativepath.c_str());
	StripExt(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	CreateTex(specindex, specpath, ecfalse, ecfalse);	//with mipmaps, linear filter
	//CreateTex(specindex, specpath, ecfalse, ecfalse);	//no mipmaps, nearest filter

	if(specindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrMess("Error", msg);
	}

	char normpath[SFH_MAX_PATH+1];
	strcpy(normpath, relativepath.c_str());
	StripExt(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	CreateTex(normindex, normpath, ecfalse, ecfalse);
	//CreateTex(normindex, normpath, ecfalse, ecfalse);

	if(normindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrMess("Error", msg);
	}

	char ownpath[SFH_MAX_PATH+1];
	strcpy(ownpath, relativepath.c_str());
	StripExt(ownpath);
	strcat(ownpath, ".team.png");

	unsigned int ownindex;
	CreateTex(ownindex, ownpath, ecfalse, ecfalse);
	//CreateTex(ownindex, ownpath, ecfalse, ecfalse);

	if(ownindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", ownpath);

		ErrMess("Error", msg);
	}

	g_tiletexs[TEX_DIFF] = diffuseindex;
	g_tiletexs[TEX_SPEC] = specindex;
	g_tiletexs[TEX_NORM] = normindex;
	g_tiletexs[TEX_TEAM] = ownindex;
}

void Change_Frames(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
}

void Resize_BrushEditFrame(Widget* w)
{
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	w->pos[0] = 0;
	w->pos[1] = TOP_PANEL_HEIGHT;
	w->pos[2] = LEFT_PANEL_WIDTH;
	w->pos[3] = g_height;
}

void Resize_ChooseTexButton(Widget* w)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=1;
	Font* f = &g_font[w->font];
	Widget* parentw = w->parent;
	w->pos[0] = 0;
	w->pos[1] = parentw->pos[1] + f->gheight*i;
	w->pos[2] = LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + f->gheight*(i+1);
	CenterLabel(w);
}

void Resize_FitToFaceButton(Widget* w)
{
	//Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32)
	int i=2;
	int j=0;
	Font* f = &g_font[w->font];
	Widget* parentw = w->parent;
	w->pos[0] = 0;
	w->pos[1] = parentw->pos[1] + f->gheight*i;
	w->pos[2] = 32+32*j;
	w->pos[3] = parentw->pos[1] + f->gheight*i+32;
	CenterLabel(w);
}

void Resize_BrushSideEditFrame(Widget* w)
{
	int j=1;
	//leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_DoorView, NULL, NULL));
	int i=4;
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	w->pos[0] = 0;
	w->pos[1] = TOP_PANEL_HEIGHT;
	w->pos[2] = LEFT_PANEL_WIDTH;
	w->pos[3] = g_height;
}

void Resize_RotateTexButton(Widget* w)
{
	int i = 4;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
	CenterLabel(w);
}

void Resize_TexEqEditBox1(Widget* w)
{
	int i = 6;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexEqEditBox2(Widget* w)
{
	int i = 7;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
}

void Resize_SelComponentDropDownS(Widget* w)
{
	int i = 8;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexScaleEditBox(Widget* w)
{
	int i = 9;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH/2;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
}

void Resize_ScaleTexButton(Widget* w)
{
	int i = 9;
	//MAINFONT8, Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + LEFT_PANEL_WIDTH/2;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
	CenterLabel(w);
}

void Resize_TexShiftEditBox(Widget* w)
{
	int i = 10;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + 0;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH/2;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexShiftButton(Widget* w)
{
	int i = 10;
	//Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = w->parent;
	Font* f = &g_font[w->font];
	w->pos[0] = parentw->pos[0] + LEFT_PANEL_WIDTH/2;
	w->pos[1] = parentw->pos[1] + 32+f->gheight*i;
	w->pos[2] = parentw->pos[0] + LEFT_PANEL_WIDTH;
	w->pos[3] = parentw->pos[1] + 32+f->gheight*(i+1);
	CenterLabel(w);
}

void Click_CompileModel()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[SFH_MAX_PATH+1];
	char initdir[SFH_MAX_PATH+1];
	FullPath("export models\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("export models\\export", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//SaveEdBuilding(filepath, &g_edbldg);
	////CompileModel(filepath, &g_edmap, g_modelholder);	//TODO: update
#endif
}

void Resize_ChooseFile(Widget *w)
{
	w->pos[0] = g_width/2 - 200;
	w->pos[1] = g_height/2 - 200;
	w->pos[2] = g_width/2 + 200;
	w->pos[3] = g_height/2 + 200;
}

void Callback_ChooseFile(const char* fullpath)
{
}

void FillGUI2()
{
	Log("assign keys");
	

	g_gui.assignanykey(&AnyKeyDown, &AnyKeyUp);

	Log("1,");
	

	g_gui.assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
	Log("2,");
	
	g_gui.assignkey(SDL_SCANCODE_ESCAPE, &Esc, NULL);
	Log("3,");
	
	g_gui.assignkey(SDL_SCANCODE_DELETE, &Delete, NULL);
	Log("4,");
	
	g_gui.assignkey(SDL_SCANCODE_C, Down_C, NULL);
	Log("5,");
	
	g_gui.assignkey(SDL_SCANCODE_V, Down_V, NULL);
	Log("5,");
	
	//AssignMouseWheel(&MouseWheel);
	//AssignMouseMove(&MouseMove);
	g_gui.assignlbutton(&MouseLeftButtonDown, &MouseLeftButtonUp);
	//AssignRButton(NULL, &MouseRightButtonUp);
	Log("6,");
	

	Font* f = &g_font[MAINFONT8];

	Log("logo...");
	

	g_gui.add(new ViewLayer(&g_gui, "logo"));
	ViewLayer* logoview = (ViewLayer*)g_gui.get("logo");
	//Image::Image(Widget* parent, const char* nm, const char* filepath, ecbool clamp, void (*reframef)(Widget* w), float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom) : Widget()

	logoview->add(new Image(NULL, "logo", "gui/dmd.jpg", ectrue, Resize_AppLogo, 1,1,1,0));

	Log("loading...");
	

	g_gui.add(new ViewLayer(&g_gui, "loading"));
	ViewLayer* loadingview = (ViewLayer*)g_gui.get("loading");
	loadingview->add(new Text(NULL, "status", "Loading...", MAINFONT8, Resize_LoadingText));

	g_gui.add(new ViewLayer(&g_gui, "editor"));
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	edview->add(new Frame(edview, "top panel", Resize_TopPanel));
	edview->add(new Frame(edview, "left panel", Resize_LeftPanel));

	Widget* toppanel = edview->get("top panel");
	Widget* leftpanel = edview->get("left panel");

	toppanel->add(new Image(toppanel, "top panel bg", "gui/filled.jpg", ectrue, Resize_TopPanel));
	leftpanel->add(new Image(leftpanel, "left panel bg", "gui/filled.jpg", ectrue, Resize_LeftPanel));

//Button(Widget* parent, const char* name, const char* filepath, const std::string label, const std::string tooltip, int f, int style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm) : Widget()

	toppanel->add(new Button(toppanel, "load", "gui/load.png", "", "Load",												MAINFONT8, BUST_LINEBASED, Resize_LoadButton, Click_LoadEdMap, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "save", "gui/save.png", "", "Save",												MAINFONT8, BUST_LINEBASED, Resize_SaveButton, Click_SaveEdMap, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "qsave", "gui/qsave.png", "", "Quick save",										MAINFONT8, BUST_LINEBASED, Resize_QSaveButton, Click_QSaveEdMap, NULL, NULL, NULL, NULL, -1, NULL));
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/build.png", "", "Export model",									MAINFONT8, BUST_LINEBASED, Resize_CompileMapButton, Click_CompileModel, NULL, NULL, NULL, NULL, -1));
#endif
	toppanel->add(new Button(toppanel, "topo", "gui/buildtopo.png", "", "Export topology maps",							MAINFONT8, BUST_LINEBASED, Resize_ExportTopoButton, Click_ExportTopo, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildunit.png", "", "Export sprite(s)",							MAINFONT8, BUST_LINEBASED, Resize_ExportButton, Click_Export, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new CheckBox(toppanel, "do inclines", RichText("Slopes"), MAINFONT8, Resize_InclinesCheck, 0));
	toppanel->add(new CheckBox(toppanel, "do sides", RichText("Sides"), MAINFONT8, Resize_SidesCheck, 0));
	toppanel->add(new CheckBox(toppanel, "do frames", RichText("Frames"), MAINFONT8, Resize_FramesCheck, 0));
	toppanel->add(new CheckBox(toppanel, "do rotations", RichText("6DOF"), MAINFONT8, Resize_RotationsCheck, 0));
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/buildbuilding.png", "", "Export building/tree/animation sprites",	MAINFONT8, BUST_LINEBASED, Resize_ExportBldgButton, Click_ExportBuildingSprite, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "build", "gui/buildunit.png", "", "Export unit/animation sprites from 8 sides",	MAINFONT8, BUST_LINEBASED, Resize_ExportUnitButton, Click_ExportUnitSprites, NULL, NULL, NULL, NULL, -1));
#endif
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/buildtile.png", "", "Export tile with inclines",					MAINFONT8, BUST_LINEBASED, Resize_ExportTileButton, Click_ExportTileSprites, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "build", "gui/buildroad.png", "", "Export road tiles with applicable inclines and rotations",	MAINFONT8, BUST_LINEBASED, Resize_ExportRoadButton, Click_ExportRoadSprites, NULL, NULL, NULL, NULL, -1));
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/buildridge.png", "", "Export ridge with inclines from 4 sides",	MAINFONT8, BUST_LINEBASED, Resize_ExportRidgeButton, Click_CompileMap, NULL, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildwater.png", "", "Export water tile with inclines",			MAINFONT8, BUST_LINEBASED, Resize_ExportWaterButton, Click_CompileMap, NULL, NULL));
	//toppanel->add(new Button(toppanel, "run", "gui/run.png", "", "Compile and run",									MAINFONT8, BUST_LINEBASED, Resize_CompileRunButton, Click_CompileRunMap, NULL, NULL));
#endif
#endif
	toppanel->add(new Button(toppanel, "undo", "gui/undo.png", "", "Undo",												MAINFONT8, BUST_LINEBASED, Resize_UndoButton, Undo, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "redo", "gui/redo.png", "", "Redo",												MAINFONT8, BUST_LINEBASED, Resize_RedoButton, Redo, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "newbrush", "gui/newbrush.png", "", "New brush",									MAINFONT8, BUST_LINEBASED, Resize_NewBrushButton, &Click_NewBrush, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "cutbrush", "gui/cutbrush.png", "", "Cut brush",									MAINFONT8, BUST_LINEBASED, Resize_CutBrushButton, Click_CutBrush, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "newent", "gui/newent.png", "", "Add 3D model from /models/",					MAINFONT8, BUST_LINEBASED, Resize_NewEntityButton, Click_AddMS3D, NULL, NULL, NULL, NULL, -1, NULL));
	//toppanel->add(new Button(toppanel, "selent", "gui/selent.png", "", "Select entities only",						MAINFONT8, BUST_LINEBASED, Resize_SelEntButton, NULL, NULL, NULL));
	//toppanel->add(new Button(toppanel, "selbrush", "gui/selbrush.png", "", "Select brushes only",						MAINFONT8, BUST_LINEBASED, Resize_SelBrushButton, NULL, NULL, NULL));
	//toppanel->add(new Button(toppanel, "selentbrush", "gui/selentbrush.png", "", "Select entities and brushes",		MAINFONT8, BUST_LINEBASED, Resize_SelEntBrushButton, NULL, NULL, NULL));
	toppanel->add(new Button(toppanel, "rotxccw", "gui/rotxccw.png", "", "Rotate counter-clockwise on x axis",			MAINFONT8, BUST_LINEBASED, Resize_RotXCCWButton, Click_RotXCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotxcw", "gui/rotxcw.png", "", "Rotate clockwise on x axis",					MAINFONT8, BUST_LINEBASED, Resize_RotXCWButton, Click_RotXCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotyccw", "gui/rotyccw.png", "", "Rotate counter-clockwise on y axis",			MAINFONT8, BUST_LINEBASED, Resize_RotYCCWButton, Click_RotYCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotycw", "gui/rotycw.png", "", "Rotate clockwise on y axis",					MAINFONT8, BUST_LINEBASED, Resize_RotYCWButton, Click_RotYCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotzccw", "gui/rotzccw.png", "", "Rotate counter-clockwise on z axis",			MAINFONT8, BUST_LINEBASED, Resize_RotZCCWButton, Click_RotZCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotzcw", "gui/rotzcw.png", "", "Rotate clockwise on z axis",					MAINFONT8, BUST_LINEBASED, Resize_RotZCWButton, Click_RotZCW, NULL, NULL, NULL, NULL, -1, NULL));

//EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), ecbool pw, int maxl, void (*change3)(unsigned int key, unsigned int scancode, ecbool down, int parm), void (*submitf)(), int parm);


	toppanel->add(new EditBox(toppanel, "rotdeg", RichText("15"),															MAINFONT8, Resize_RotDegEditBox, ecfalse, 6, &Change_RotDeg, NULL, 0));
	toppanel->add(new Text(toppanel, "rotdegtext", RichText("degrees"),													MAINFONT8, Resize_RotDegText));
	toppanel->add(new EditBox(toppanel, "zoom", RichText("1"),															MAINFONT8, Resize_ZoomEditBox, ecfalse, 6, &Change_Zoom, NULL, 0));
	toppanel->add(new Text(toppanel, "zoomtext", RichText("zoom"),														MAINFONT8, Resize_ZoomText));
	toppanel->add(new DropList(toppanel, "snapgrid",															MAINFONT8, Resize_SnapGridEditBox, Change_SnapGrid));
	DropList* snapgs = (DropList*)toppanel->get("snapgrid");
	//snapgs->options.push_back("4 m");	//0
	//snapgs->options.push_back("2 m");	//1
	//snapgs->options.push_back("1 m");	//2
	//snapgs->options.push_back("50 cm");	//3
	//snapgs->options.push_back("25 cm");	//4
	//snapgs->options.push_back("12.5 cm");	//5
	//snapgs->options.push_back("6.25 cm");	//6
	//snapgs->options.push_back("3.125 cm");	//7
	//snapgs->options.push_back("1.5625 cm");	//8
	//snapgs->select(4);

	float cscales[] = CM_SCALES;
	std::string cscales_txt[] = CM_SCALES_TXT;

	for(int j=0; j<sizeof(cscales)/sizeof(float); j++)
	{
		snapgs->options.push_back(cscales_txt[j].c_str());
	}

	//snapgs->select(6);
	snapgs->selected = 6;

	//toppanel->add(new EditBox(toppanel, "snapgrid", "25",														MAINFONT8, Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16), ecfalse, 6, &Change_SnapGrid, 0));
	toppanel->add(new Text(toppanel, "snapgrid text", RichText("snap grid"),													MAINFONT8, Resize_SnapGridText));
	toppanel->add(new EditBox(toppanel, "maxelev", RichText("10000"),														MAINFONT8, Resize_MaxElevEditBox, ecfalse, 6, &Change_MaxElev, NULL, 0));
	toppanel->add(new Text(toppanel, "maxelev text", RichText("max elev."),													MAINFONT8, Resize_MaxElevText));
	toppanel->add(new CheckBox(toppanel, "showsky", RichText("show clip"),													MAINFONT8, Resize_ShowSkyCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_ShowSky));

	toppanel->add(new Button(toppanel, "persp", "gui/projpersp.png", RichText(""), RichText("Perspective projection"),				MAINFONT8, BUST_LINEBASED, Resize_PerspProjButton, Click_ProjPersp, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "ortho", "gui/projortho.png", RichText(""), RichText("Orthographic projection"),				MAINFONT8, BUST_LINEBASED, Resize_OrthoProjButton, Click_ProjOrtho, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "resetview", "gui/resetview.png", RichText(""), RichText("Reset view"),						MAINFONT8, BUST_LINEBASED, Resize_ResetViewButton, Click_ResetView, NULL, NULL, NULL, NULL, -1, NULL));

	toppanel->add(new Text(toppanel, "frames text", "frames",														MAINFONT8, Resize_FramesText));
	toppanel->add(new EditBox(toppanel, "frames", "1",															MAINFONT8, Resize_FramesEditBox, ecfalse, 6, &Change_Frames, NULL, 0));

#if 0
	toppanel->add(new Button(toppanel, "explosion", "gui/explosion.png", "", "Explode crater",					MAINFONT8, BUST_LINEBASED, Resize_ExplodeButton, Click_Explode, NULL, NULL, NULL, NULL, -1));
#endif


	toppanel->add(new Button(toppanel, "addtile", "gui/addtile.png", "", "Add tile texture",					MAINFONT8, BUST_LINEBASED, Resize_AddTileButton, Click_AddTile, NULL, NULL, NULL, NULL, -1, NULL));

	//toppanel->add(new CheckBox(toppanel, "NE", "Leads NE",													MAINFONT8, Resize_LeadsNECheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	//toppanel->add(new CheckBox(toppanel, "SE", "Leads SE",													MAINFONT8, Resize_LeadsSECheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	//toppanel->add(new CheckBox(toppanel, "SW", "Leads SW",													MAINFONT8, Resize_LeadsSWCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	//toppanel->add(new CheckBox(toppanel, "NW", "Leads NW",													MAINFONT8, Resize_LeadsNWCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));



	//toppanel->add(new Text(toppanel, "fps", "fps: 0", MAINFONT8, Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 10), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 70), ectrue));

	edview->add(new Frame(NULL, "viewports frame", Resize_ViewportsFrame));
	Widget* viewportsframe = edview->get("viewports frame");

#if 0
Viewport(Widget* parent, const char* n, void (*reframef)(Widget* w),
	          void (*drawf)(int p, int x, int y, int w, int h),
	          ecbool (*ldownf)(int p, int relx, int rely, int w, int h),
	          ecbool (*lupf)(int p, int relx, int rely, int w, int h),
	          ecbool (*mousemovef)(int p, int relx, int rely, int w, int h),
	          ecbool (*rdownf)(int p, int relx, int rely, int w, int h),
	          ecbool (*rupf)(int p, int relx, int rely, int w, int h),
	          ecbool (*mousewf)(int p, int d),
	          int parm)
#endif

	viewportsframe->add(new Viewport(viewportsframe, "bottom left viewport",	Resize_BottomLeftViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 0));
	viewportsframe->add(new Viewport(viewportsframe, "top left viewport",		Resize_TopLeftViewport,		&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 1));
	viewportsframe->add(new Viewport(viewportsframe, "bottom right viewport",	Resize_BottomRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 2));
	viewportsframe->add(new Viewport(viewportsframe, "top right viewport",		Resize_TopRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 3));

	g_vptype[VIEWPORT_FRONT] = VpType(Vec3f(0, 0, MAX_DISTANCE/3), Vec3f(0, 1, 0), "Front");
	g_vptype[VIEWPORT_TOP] = VpType(Vec3f(0, MAX_DISTANCE/3, 0), Vec3f(0, 0, -1), "Top");
	g_vptype[VIEWPORT_LEFT] = VpType(Vec3f(MAX_DISTANCE/3, 0, 0), Vec3f(0, 1, 0), "Left");
	//g_vptype[VIEWPORT_ANGLE45O] = VpType(Vec3f(MAX_DISTANCE/3, MAX_DISTANCE/3, MAX_DISTANCE/3), Vec3f(0, 1, 0), "Angle");
	g_vptype[VIEWPORT_ANGLE45O] = VpType(Vec3f(1000.0f/3, 1000.0f/3, 1000.0f/3), Vec3f(0, 1, 0), "Angle");
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);
	ResetView(ecfalse);

	g_viewport[0] = VpWrap(VIEWPORT_FRONT);
	g_viewport[1] = VpWrap(VIEWPORT_TOP);
	g_viewport[2] = VpWrap(VIEWPORT_LEFT);
	g_viewport[3] = VpWrap(VIEWPORT_ANGLE45O);

	viewportsframe->add(new Image(viewportsframe, "h divider", "gui/filled.jpg", ectrue, Resize_HDivider));
	viewportsframe->add(new Image(viewportsframe, "v divider", "gui/filled.jpg", ectrue, Resize_VDivider));

	g_gui.add(new ViewLayer(&g_gui, "brush edit"));
	ViewLayer* brusheditview = (ViewLayer*)g_gui.get("brush edit");
	brusheditview->add(new Frame(NULL, "left panel", Resize_BrushEditFrame));

	leftpanel = brusheditview->get("left panel");
	//leftpanel->add(new EditBox(leftpanel, "texture path", "no texture", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 0), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), ecfalse, 64, NULL, -1));
	//i++;
	leftpanel->add(new Button(leftpanel, "choose texture", "gui/transp.png", "Choose Texture", "Choose texture", MAINFONT8, BUST_LINEBASED, Resize_ChooseTexButton, Click_BChooseTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new Button(leftpanel, "fit to face", "gui/fittoface.png", "", "Fit to face",	MAINFONT8, BUST_LINEBASED, Resize_FitToFaceButton, Click_FitToFace, NULL, NULL, NULL, NULL, -1, NULL));
	//leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_DoorView, NULL, NULL));

	g_gui.add(new ViewLayer(&g_gui, "brush side edit"));
	ViewLayer* brushsideeditview = (ViewLayer*)g_gui.get("brush side edit");
	brushsideeditview->add(new Frame(brushsideeditview, "left panel", Resize_BrushSideEditFrame));

	leftpanel = brushsideeditview->get("left panel");
	leftpanel->add(new Button(leftpanel, "rotate texture", "gui/transp.png", "Rotate Texture", "Rotate texture", MAINFONT8, BUST_LINEBASED, Resize_RotateTexButton, Click_RotateTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new EditBox(leftpanel, "u equation", "A B C D", MAINFONT8, Resize_TexEqEditBox1, ecfalse, 256, Change_TexEq, NULL, 0));
	leftpanel->add(new EditBox(leftpanel, "v equation", "A B C D", MAINFONT8, Resize_TexEqEditBox2, ecfalse, 256, Change_TexEq, NULL, 1));
	leftpanel->add(new DropList(leftpanel, "select component", MAINFONT8, Resize_SelComponentDropDownS, Change_SelComp));
	Widget* selcompwidg = leftpanel->get("select component");
	selcompwidg->options.push_back("u component");
	selcompwidg->options.push_back("v component");
	leftpanel->add(new EditBox(leftpanel, "texture scale", "1", MAINFONT8, Resize_TexScaleEditBox, ecfalse, 10, NULL, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture scale button", "gui/transp.png", "Scale", "Scale texture component", MAINFONT8, BUST_LINEBASED, Resize_ScaleTexButton, Click_ScaleTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new EditBox(leftpanel, "texture shift", "0.05", MAINFONT8, Resize_TexShiftEditBox, ecfalse, 10, NULL, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture shift button", "gui/transp.png", "Shift", "Shift texture component", MAINFONT8, BUST_LINEBASED, Resize_TexShiftButton, Click_ShiftTex, NULL, NULL, NULL, NULL, -1, NULL));

#if 0
	g_gui.add(new ViewLayer(&g_gui, "choose file"));
	ViewLayer* choosefileview = (ViewLayer*)g_gui.get("choose file");
	choosefileview->add(new ChooseFile(choosefileview, "choose file", Resize_ChooseFile, Callback_ChooseFile));
#endif

#if 1
	g_gui.add(new ViewLayer(&g_gui, "door edit"));
	ViewLayer* dooreditview = (ViewLayer*)g_gui.get("door edit");
#if 0
	dooreditview->add(new Frame(NULL, "left panel", Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)));

	leftpanel = dooreditview->get("left panel", WIDGET_FRAME);
	i=1;
	j=0;
	leftpanel->add(new Button(leftpanel, "set door", "gui/yesdoor.png", "", "Make door and set properties",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_SetDoor, NULL, NULL));
	j++;
	leftpanel->add(new Button(leftpanel, "unmake door", "gui/nodoor.png", "", "Unmake door",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_UnmakeDoor, NULL, NULL));
	j=0;
	i+=3;
	leftpanel->add(new Button(leftpanel, "show/hide", "gui/transp.png", "Open / Close", "Open / Close door", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1)), Click_OpenCloseDoor, NULL, NULL));
	i++;
	leftpanel->add(new Text(leftpanel, "opendeg label", "Open degrees:", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i)));
	leftpanel->add(new EditBox(leftpanel, "opendeg", "90", MAINFONT8, Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 70), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH-10), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1)), ecfalse, 10, NULL, 0));
#endif
#endif

	g_gui.add(new ViewLayer(&g_gui, "render"));
	ViewLayer* renderview = (ViewLayer*)g_gui.get("render");

	renderview->add(new Viewport(NULL, "render viewport",	Resize_FullViewport, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3));

	g_gui.hideall();
	g_gui.show("loading");
	//OpenAnotherView("brush edit view");
}

void GetDoFrames()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* check = toppanel->get("do frames");

	g_doframes = check->selected == 1;
}

void GetDoInclines()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* check = toppanel->get("do inclines");

	g_doinclines = check->selected == 1;
}

void GetDoSides()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* check = toppanel->get("do sides");

	g_dosides = check->selected == 1;
}

void GetDoRotations()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* check = toppanel->get("do rotations");

	g_dorots = check->selected == 1;
}


int GetNumFrames()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* frameseditbox = toppanel->get("frames");

	int nframes = StrToInt(frameseditbox->value.rawstr().c_str());

	return nframes;
}

void SetNumFrames(int nframes)
{
	///g_renderframes = nframes;
	return;
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	if(!edview)  ErrMess("Error", "edview not found");

	Widget* toppanel = edview->get("top panel");

	if(!toppanel)  ErrMess("Error", "toppanel not found");

	EditBox* frameseditbox = (EditBox*)toppanel->get("frames");

	if(!frameseditbox)  ErrMess("Error", "frameseditbox not found");

	char nframesstr[128];
	sprintf(nframesstr, "%d", nframes);
	RichText rnframesstr = RichText(nframesstr);
	frameseditbox->changevalue(&rnframesstr);
}
