











#include "platform.h"
#include "debug.h"
#include "utils.h"
//#include "main.h"
//#include "gui.h"
//#include "unit.h"
//#include "pathfinding.h"
//#include "collision.h"
//#include "building.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "gui/widgets/spez/cstrview.h"
#include "sim/player.h"
#include "window.h"
#include "app/appmain.h"

Timer g_profile[TIMERS];
 ecbool g_debuglines = ecfalse;
std::ofstream g_profF;

#if 0

void StartTimer(int id)
{
	return;

	//if(g_appmode != APPMODE_PLAY)
	//	return;

	g_profile[id].starttick = GetTicks();
}

void StopTimer(int id)
{
	return;

	if(g_appmode != APPMODE_PLAY)
		return;

#ifdef DEBUGLOG
	if(id == TIMER_UPDATE)
	{
		Log(std::endl<<"upd el = "<<GetTicks()<<" - "<<g_profile[id].starttick););
	}
#endif

	unsigned __int64 elapsed = GetTicks() - g_profile[id].starttick;
	g_profile[id].starttick = GetTicks();
	g_profile[id].lastframeelapsed += elapsed;

#ifdef DEBUGLOG
	if(id == TIMER_UPDATE)
	{
		Log(std::endl<<"upd el"<<elapsed<<" tot"<<g_profile[id].lastframeelapsed<<" avg"<<g_profile[id].averagems););
	}
#endif

	if(id == TIMER_FRAME || g_profile[id].lastframe < g_profile[TIMER_FRAME].lastframe)
	{

#ifdef DEBUGLOG
		if(id == TIMER_UPDATE)
		{
			Log(std::endl<<"upd ( (double)"<<g_profile[id].lastframeelapsed<<" + "<<g_profile[id].averagems<<"*(double)"<<g_profile[id].frames<<" ) / (double)("<<g_profile[id].frames<<"+1); = ";
		}
#endif

		//g_profile[id].averagems = ( g_profile[id].lastframeaverage + g_profile[id].averagems*g_profile[id].frames ) / (g_profile[id].frames+1);
		g_profile[id].averagems = ( (double)g_profile[id].lastframeelapsed + g_profile[id].averagems*(double)g_profile[id].frames ) / (double)(g_profile[id].frames+1);
		g_profile[id].frames++;
		//g_profile[id].timescountedperframe = 0;
		g_profile[id].lastframeelapsed = 0;
		g_profile[id].lastframe = g_profile[TIMER_FRAME].lastframe;


#ifdef DEBUGLOG
		if(id == TIMER_UPDATE)
		{
			Log(g_profile[id].averagems););
		}
#endif

		//Log(g_profile[id].name<<" "<<g_profile[id].averagems<<"ms");
	}
	if(id == TIMER_FRAME)
		g_profile[id].lastframe++;

	//g_profile[id].lastframeaverage = ( elapsed + g_profile[id].lastframeaverage*g_profile[id].timescountedperframe ) / (g_profile[id].timescountedperframe+1);
	//g_profile[id].timescountedperframe+=1.0f;
}

#endif

void WriteProfiles(int in, int layer)
{
	double parentavgms;

	if(in == -1)
	{
		char fullpath[SFH_MAX_PATH+1];
		FullWritePath("profiles.txt", fullpath);
		g_profF.open(fullpath, std::ios_base::out);
		parentavgms = g_profile[TIMER_FRAME].averagems;
	}
	else
	{
		parentavgms = g_profile[in].averagems;
	}

	double ofparentpct;
	double totalms = 0;
	double totalofparentpct = 0;
	double percentage;
	int subprofiles = 0;

	for(int j=0; j<TIMERS; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		totalms += g_profile[j].averagems;
	}

	for(int j=0; j<TIMERS; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		percentage = 100.0 * g_profile[j].averagems / totalms;
		ofparentpct = 100.0 * g_profile[j].averagems / parentavgms;
		totalofparentpct += ofparentpct;
		subprofiles++;

		for(int k=0; k<layer; k++)
			g_profF<<"\t";

		g_profF<<g_profile[j].name<<"\t...\t"<<g_profile[j].averagems<<"ms per frame, "<<percentage<<"% of this level's total"<<std::endl;

		WriteProfiles(j, layer+1);
	}

	if(subprofiles > 0)
	{
		for(int k=0; k<layer; k++)
			g_profF<<"\t";

		g_profF<<"level total sum: "<<totalms<<" ms per frame, that means "<<totalofparentpct<<"% of this parent's duration underwent profiling"<<std::endl;
	}

	if(in == -1)
	{
		g_profF.flush();
		g_profF.close();
	}
}

void DefTimer(int id, int inside, const char* name)
{
	g_profile[id].inside = inside;
	strcpy(g_profile[id].name, name);
}

void Profiles()
{
	DefTimer(TIMER_FRAME, -1, "Frame");
	DefTimer(TIMER_EVENT, TIMER_FRAME, "EventProc");
	DefTimer(TIMER_DRAW, TIMER_FRAME, "Draw();");
	DefTimer(TIMER_DRAWSCENEDEPTH, TIMER_DRAW, "DrawSceneDepth();");
	DefTimer(TIMER_DRAWSETUP, TIMER_DRAW, "Draw(); setup");
	DefTimer(TIMER_DRAWGUI, TIMER_DRAW, "DrawGUI();");
	DefTimer(TIMER_DRAWMINIMAP, TIMER_DRAW, "DrawMinimap();");
	DefTimer(TIMER_UPDATE, TIMER_FRAME, "Update();");
	DefTimer(TIMER_RESETPATHNODES, TIMER_UPDATE, "ResetPathNodes();");
	DefTimer(TIMER_MANAGETRIPS, TIMER_UPDATE, "MgTrips();");
	DefTimer(TIMER_UPDATEUNITS, TIMER_UPDATE, "UpdMvs();");
	DefTimer(TIMER_UPDUONCHECK, TIMER_UPDATEUNITS, "Upd U On Ch");
	DefTimer(TIMER_UPDUNITAI, TIMER_UPDATEUNITS, "Upd Mv AI");
	DefTimer(TIMER_UPDLAB, TIMER_UPDUNITAI, "UpdLab();");
	DefTimer(TIMER_UPDTRUCK, TIMER_UPDUNITAI, "UpdTruck();");
	DefTimer(TIMER_FINDJOB, TIMER_UPDLAB, "FindJob();");
	DefTimer(TIMER_JOBLIST, TIMER_FINDJOB, "Job list collection");
	DefTimer(TIMER_JOBSORT, TIMER_FINDJOB, "Job list sort");
	DefTimer(TIMER_JOBPATH, TIMER_FINDJOB, "Job prepathing");
	DefTimer(TIMER_MOVEUNIT, TIMER_UPDATEUNITS, "Move Mv");
	DefTimer(TIMER_ANIMUNIT, TIMER_UPDATEUNITS, "Anim Mv");
	DefTimer(TIMER_UPDATEBUILDINGS, TIMER_UPDATE, "UpdBls();");
	DefTimer(TIMER_DRAWBL, TIMER_DRAW, "DrawBuildings();");
	DefTimer(TIMER_DRAWUNITS, TIMER_DRAW, "DrawUnits();");
	DefTimer(TIMER_DRAWRIM, TIMER_DRAW, "DrawRim();");
	DefTimer(TIMER_DRAWWATER, TIMER_DRAW, "DrawWater();");
	DefTimer(TIMER_DRAWCD, TIMER_DRAW, "DrawCrPipes();");
	DefTimer(TIMER_DRAWPOWLS, TIMER_DRAW, "DrawPowls();");
	DefTimer(TIMER_DRAWFOLIAGE, TIMER_DRAW, "DrawFol();");
	DefTimer(TIMER_DRAWLIST, TIMER_DRAW, "gather drawing list");
	DefTimer(TIMER_DRAWSORT, TIMER_DRAW, "sort drawing list");
	DefTimer(TIMER_SORTPARTICLES, TIMER_DRAW, "SortParticles();");
	DefTimer(TIMER_DRAWPARTICLES, TIMER_DRAW, "DrawParticles();");
	DefTimer(TIMER_DRAWMAP, TIMER_DRAW, "DrawMap();");
	//DefTimer(SHADOWS, TIMER_DRAW, "Shadows");
	DefTimer(TIMER_DRAWSKY, TIMER_DRAW, "DrawSky();");
	DefTimer(TIMER_DRAWROADS, TIMER_DRAW, "DrawRoads();");
	//DefTimer(DRAWMODEL1, TIMER_DRAWBL, "Draw model 1");
	//DefTimer(DRAWMODEL2, TIMER_DRAWBL, "Draw model 2");
	//DefTimer(DRAWMODEL3, TIMER_DRAWBL, "Draw model 3");
	DefTimer(TIMER_DRAWMAPDEPTH, TIMER_DRAWSCENEDEPTH, "DrawMap(); depth");
	DefTimer(TIMER_DRAWUMAT, TIMER_DRAWUNITS, "CPU-side matrix math etc.");
	DefTimer(TIMER_DRAWUGL, TIMER_DRAWUNITS, "GPU-side");
	DefTimer(TIMER_DRAWUTEXBIND, TIMER_DRAWUNITS, "texture bind");
}

void LastNum(const char* l)
{
	return;

#if 1
	char fullpath[SFH_MAX_PATH+1];
	FullWritePath("last.txt", fullpath);
	std::ofstream last;
	last.open(fullpath, std::ios_base::out);
	last<<l;
	last.flush();
#else
	Log(l);
	
#endif
}

#ifdef GLDEBUG
void CheckGLError(const char* file, int line)
{
	//char msg[2048];
	//sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	//ErrMess("Out of memory", msg);
	int error = glGetError();

	if(error == GL_NO_ERROR)
		return;

	Log("GL Error #%d in %s on line %d using shader #%d", error, file, line, g_curS);
}
#endif

#if 0
void UDebug(int i)
{
	return;

	Mv* mv = &g_mv[i];
	MvType* t = &g_unitType[mv->type];

	Log("UNIT DEBUGLOG: "<<t->name<<" ("<<i<<")");
	Log("path size: "<<mv->path.size());

	if(mv->collidesfast())
	{
		Log("COLLIDES: type:"<<g_collidertype<<" ID:"<<g_lastcollider);

		if(g_collidertype == COLLIDER_BUILDING)
		{
			Bl* b = &g_bl[g_lastcollider];
			BlType* bt = &g_bltype[b->type];

			Log("COLLIDER B: "<<bt->name);

			if(mv->confirmcollision(g_collidertype, g_lastcollider))
			{
				Log("CONFIRMED COLLISION");

				Vec3f p = mv->camera.Position();
				Vec3f p2 = b->pos;

				float r = t->radius;
				float hwx = bt->widthX*TILE_SIZE/2.0f;
				float hwz = bt->widthZ*TILE_SIZE/2.0f;

				Log("COLLISION DX,DZ: "<<(fabs(p2.x-p.x)-r-hwx)<<","<<(fabs(p2.z-p.z)-r-hwz));
			}
		}
	}
}
#endif

#ifdef PLATFORM_WIN
#include <windows.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")
#endif

#ifdef MEMDEBUG
void CheckMem(const char* file, int line, const char* sep)
{
	return;
#ifdef PLATFORM_WIN
	PROCESS_MEMORY_COUNTERS info = {0};
	DWORD cb;
	ecbool b = GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
	Log(sep<<" "<<file<<" line"<<line<<" (?"<<b<<")info.WorkingSetSize: "<<info.WorkingSetSize<<"B / "<<(info.WorkingSetSize/1024)<<"KB "<<(info.WorkingSetSize/1024/1024)<<"MB "<<(info.WorkingSetSize/1024/1024/1024)<<"GB ");
	
#endif
}
#endif

#if !defined( PLATFORM_MAC ) && !defined( PLATFORM_IOS )
GLvoid APIENTRY GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
//DEBUGPROC GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	//ErrMess("GL Error", message);
	Log("GL Message: %s\r\n", message);
}
#endif

void CheckError(const char* file, int line)
{
	const char* m = SDL_GetError();

	if(!m[0])
		return;

	Log("SDL error: %s %d : %s", file, line, m);
}

int GetFreeVideoMem()
{
	//TODO mobile
	int availableKB[4];
//#ifdef GL234
#ifndef PLATFORM_MOBILE
#ifdef PLATFORM_WIN
	if(GLEW_NVX_gpu_memory_info)
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableKB[0]);
	int temp = GLEW_ATI_meminfo;
	if(GLEW_ATI_meminfo)
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, availableKB);
	return availableKB[0];
#endif
#endif
	return 0;
}
