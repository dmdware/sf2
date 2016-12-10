
#include "mod.h"
#include "../platform.h"
#include "../utils.h"
#include "../app/appmain.h"
#include "../script/script.h"
#include "../sim/conduit.h"

char g_modname[MODNAME_LEN+1] = "";
RichText g_abortmess;
int g_moderrs = 0;

//abort loading mod, quit to menu from lobby if necessary
void AbortMod(const char* m)
{
	g_abortmess = g_abortmess + RichText("\n") + RichText(m);
	g_moderrs ++;
}

void UnloadMod()
{
	for(int i=0; i<CD_TYPES; ++i)
	{
		CdType* ct = &g_cdtype[i];
		*ct = CdType();
		//ct->on = ecfalse;
	}
}

void GetGlobals(ObjectScript::OS* os)
{
	os->getGlobal("TILE_SIZE");
	int ts = os->popInt();

	if(ts <= 0 || ts > USHRT_MAX)
	{
		AbortMod("Invalid value for TILE_SIZE.");
		return;
	}
}

//TODO check files with server, queue up resources to download and load
void LoadMod(const char* folder)
{
	strcpy(g_modname, folder);
	g_abortmess = RichText("");
	g_moderrs = 0;
	char screl[SFH_MAX_PATH+1];
	sprintf(screl, "mods/%s/test.os", folder);
	//char full[SFH_MAX_PATH+1];
	//FullPath(screl, full);
	
	//InfoMess(screl,screl);
	char scfull[SFH_MAX_PATH+1];
	FullPath(screl, scfull);
	//InfoMess(scfull,scfull);

	//for(int i=0; i<900000; i++)
	//	1+1;

	ObjectScript::OS* os = ObjectScript::OS::create();

	//DefGlobals(os);
	
	//os->setGlobal(ObjectScript::def("Vec2i", Script_Vec2i));   // use binder

	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	os->require(scfull);
	//os->setGlobal(def("test", test));   // use binder
	//os->eval("print(test(2, 3))");      // outputs: 5


	
	GetGlobals(os);


	os->release();


	//ExecScript(screl);

	if(g_moderrs)
	{
		UnloadMod();
		//TODO
	}
}

void LoadMod()
{
	char modfull[SFH_MAX_PATH+1];
	FullPath("mod.ini", modfull);
	FILE* fp = fopen(modfull, "r");
	char modname[MODNAME_LEN+1];
	if(!fp)
	{
		g_modname[0] = 0;
		return;
	}
	fgets(modname, MODNAME_LEN, fp);
	fclose(fp);
	LoadMod(modname);
}