

#include "rescache.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "../app/appmain.h"
#include "../debug.h"

FILE* g_rescachefp = NULL;
ecbool g_rescacheread = ecfalse;

ecbool LoadResCache(const char* relative)
{
	char fulldir[SFH_MAX_PATH+1];
	FullPath(RESCACHE, fulldir);
	g_rescachefp = fopen(fulldir, "rb");
	if(!g_rescachefp)
		return ecfalse;
	int ver = -1;
	fread(&ver, sizeof(int), 1, g_rescachefp);
	if(ver != RESCACHEVER)
	{
		fclose(g_rescachefp);
		return ecfalse;
	}
	g_appmode = APPMODE_LOADING;
	Widget *gui = (Widget*)&g_gui;
	gui->hideall();
	gui->show("loading");
	g_rescacheread = ectrue;
	return ectrue;
}

ecbool MakeResCache(const char* relative)
{
	char fulldir[SFH_MAX_PATH+1];
	FullPath("rescache/", fulldir);
	MakeDir(fulldir);
	FullPath(RESCACHE, fulldir);
	g_rescachefp = fopen(fulldir, "wb");
	int ver = RESCACHEVER;
	fwrite(&ver, sizeof(int), 1, g_rescachefp);
	g_rescacheread = ecfalse;
	return ectrue;
}

void AddResCache(const char* identstr)
{
	int len = strlen(identstr)+1;

	fwrite(&len, sizeof(int), 1, g_rescachefp);
	fwrite(identstr, sizeof(char), len, g_rescachefp);
}