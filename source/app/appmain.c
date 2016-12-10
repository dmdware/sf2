


#include "appmain.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"

char g_appmode = APPMODE_LOGO;
char g_viewmode = VIEWMODE_FIRST;

void UpdLoad()
{
	Widget *gui, *menu;
	Node* it;

	gui = (Widget*)&g_gui;

	switch(g_restage)
	{
	case 0:
		if(!Load1Model()) g_restage++;
		break;
	case 1:
		if(!Load1Texture())
		{
			List_free(&g_modload);
			List_free(&g_texload);

			g_lastLMd = -1;
			g_lastLTex = -1;

			g_appmode = APPMODE_MENU;
			Widget_hide(gui);
			menu = Widget_get(gui, "menu");
			Widget_show(menu);
		}
		break;
	}
}

void UpdReload()
{
	Widget *gui, *load;

	gui = (Widget*)&g_gui;

	g_restage = 0;
	g_lastLTex = -1;
	g_lastLMd = -1;
	Widget_free(gui);
	FreeModels();
	FreeSprites();
	FreeTextures();
	BreakWin(TITLE);
	MakeWin(TITLE);

	/* Important - VBO only possible after window GL context made. */
	g_appmode = APPMODE_LOADING;

	LoadSysRes();
	QueueSimRes();
	FillGUI();

	Widget_hide(gui);
	load = Widget_get(gui, "loading");
	Widget_show(load);

	/*
	TODO
	Overhaul ui system
	Make scrollable everywhere
	Croppable pipeline 
	*/
}

void UpdSim()
{
}

void Update()
{
	if(g_sock)
		UpdNet();

	if(g_appmode == APPMODE_LOGO)
		UpdLogo();
	/* else if(g_appmode == APPMODE_INTRO)
		UpdateIntro(); */
	else if(g_appmode == APPMODE_LOADING)
		UpdLoad();
	else if(g_appmode == APPMODE_RELOADING)
		UpdReload();
	else if(g_appmode == APPMODE_PLAY)
		UpdSim();
	else if(g_appmode == APPMODE_EDITOR)
		UpdEd();
}

void DrawScene(Matrix proj, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
}

void DrawSceneDepth()
{
}

void MakeFBO(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex, int w, int h)
{
	/* OpenGL 1.4 way */

	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, renderdepthtex);
	glBindTexture(GL_TEXTURE_2D, *renderdepthtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *renderdepthtex, 0);

	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
	glDrawBuffers(1, DrawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ErrMess("Error", "Couldn't create framebuffer for render.");
		return;
	}
}

void DelFBO(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex)
{
	/* delete resources */
	glDeleteTextures(1, rendertex);
	glDeleteTextures(1, renderdepthtex);
	/* Bind 0, which means render to back buffer, as a result, fb is unbound */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, renderfb);
	CHECKGLERROR();
}

void Draw()
{
	Widget *gui;

	gui = (Widget*)&g_gui;

	/* TODO leave as float for now then use fixmath int's */

	Widget_frameupd(gui);
	Widget_draw(gui);
}

void LoadCfg()
{
	EnumDisp();
	Node *rit; /* Vec2i */
	Vec2i *rp;
	int w, h;
	float scale;
	char cfgfull[SFH_MAX_PATH+1];
	char line[128];
	char key[128];
	char act[128];
	FILE *fp;
	float valuef;
	int valuei;
	ecbool valueb;

	if(g_ress.size)
	{
		rit = g_ress.head;
		rp = (Vec2i*)rit->data;
		g_selres = *rp;
	}
	else
	{
		SDL_GL_GetDrawableSize(g_win, &w, &h);

		g_selres.x = w;
		g_selres.y = h;
	}

	for(rit=g_ress.head; rit; rit=rit->next)
	{
		/* below acceptable height? */
		if(g_selres.y < 480)
		{
			rp = (Vec2i*)rit->data;

			if(rp->y > g_selres.y &&
				rp->x > rp->y)
			{
				g_selres = *rit;
			}
		}
		/* already of acceptable height? */
		else
		{
			rp = (Vec2i*)rit->data;
			//get smallest acceptable resolution
			if(rp->x < g_selres.y &&
				rp->x > rp->y)
			{
				g_selres = *rit;
			}

			break;
		}
	}

	SwitchLang(LANG_ENG);

	FullWritePath(CONFIGFILE, cfgfull);

	fp = fopen(cfgfull, "r");

	if(!fp)
		return;

	while(!feof(fp))
	{
		strcpy(key, "");
		strcpy(act, "");

		getline(fp, line);

		if(strlen(line) > 127)
			continue;

		act[0] = 0;

		sscanf(line, "%s %s", key, act);

		valuef = StrToFloat(act);
		valuei = StrToInt(act);
		valueb = valuei ? dmtrue : dmfalse;

		if(strcmp(key, "fullscreen") == 0)					g_fs = valueb;
		else if(strcmp(key, "client_width") == 0)			g_width = g_selres.width = g_origwidth = valuei;
		else if(strcmp(key, "client_height") == 0)			g_height = g_selres.height = g_origheight = valuei;
		else if(strcmp(key, "screen_bpp") == 0)				g_bpp = valuei;
		else if(strcmp(key, "volume") == 0)					SetVol(valuei);
		else if(strcmp(key, "language") == 0)				SwitchLang(GetLang(act));
	}

	fclose(fp);
}

void LoadName()
{
	char cfgfull[SFH_MAX_PATH+1];
	FILE *fp;
	char line[128];

	FullWritePath("name.txt", cfgfull);

	fp = fopen(cfgfull, "r");

	if(!fp)
	{
		GenName(g_name);
		return;
	}

	getline(fp, line);
	strcpy(g_name, line);
	fclose(fp);
}

void WriteConfig()
{
	char cfgfull[SFH_MAX_PATH+1];
	FILE* fp = fopen(cfgfull, "w");
	FullWritePath(CONFIGFILE, cfgfull);
	if(!fp)
		return;
	fprintf(fp, "fullscreen %d \r\n\r\n", g_fs ? 1 : 0);
	fprintf(fp, "client_width %d \r\n\r\n", g_selres.width);
	fprintf(fp, "client_height %d \r\n\r\n", g_selres.height);
	fprintf(fp, "screen_bpp %d \r\n\r\n", g_bpp);
	fprintf(fp, "volume %d \r\n\r\n", g_volume);
	fprintf(fp, "language %s\r\n\r\n", g_lang);
	fclose(fp);
}

void WriteName()
{
	char cfgfull[SFH_MAX_PATH+1];
	FILE* fp = fopen(cfgfull, "w");
	FullWritePath("name.txt", cfgfull);
	if(!fp)
		return;
	fprintf(fp, "%s", g_name);
	fclose(fp);
}

/* Define the function to be called when ctrl-c (SIGINT) signal is sent to process */
void SignalCallback(int signum)
{
	exit(0);
}

void Init()
{
	char msg[128];
	SDL_version compile_version;
	SDL_version *link_version;
	int flags;
	int initted;

#ifdef PLATFORM_LINUX
	signal(SIGINT, SignalCallback);
#endif

	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
	}

	if(SDLNet_Init() == -1)
	{
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
	}

	link_version=Mix_Linked_Version();
	SDL_MIXER_VERSION(&compile_version);
	printf("compiled with SDL_mixer version: %d.%d.%d\n",
		compile_version.major,
		compile_version.minor,
		compile_version.patch);
	printf("running with SDL_mixer version: %d.%d.%d\n",
		link_version->major,
		link_version->minor,
		link_version->patch);

	// load support for the OGG and MOD sample/music formats
	flags=MIX_INIT_OGG|MIX_INIT_MP3;
	initted=Mix_Init(flags);
	if( (initted & flags) != flags)
	{
		sprintf(msg, "Mix_Init: Failed to init required ogg and mod support!\nMix_Init: %s", Mix_GetError());
		/* ErrMess("Error", msg); */
	}

	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
	}
	/* show 44.1KHz, signed 16bit, system byte order,
	      stereo audio, using 1024 byte chunks */
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1)
	{
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		ErrMess("Error", msg);
	}

	Mix_AllocateChannels(SOUND_CHANNELS);

	if(!g_applog)
		OpenLog("log.txt", VERSION);

	srand((unsigned int)GetTicks());

	/* TODO c-style inits, not constructors */
	LoadCfg();
	LoadName();
	MapKeys();
}

void Deinit()
{
	Node *cit;
	GUI* gui;
	unsigned __int64 start;

	EndSess();
	FreeMap();

	gui = &g_gui;
	Widget_freech((Widget*)gui);

	BreakWin(TITLE);

	for(cit=g_cn.head; cit; cit=cit->next)
	{
		Disconnect((NetConn*)cit->data);
	}

	start = GetTicks();
	/* After quit, wait to send out quit packet to make sure host/clients recieve it. */
	while (GetTicks() - start < QUIT_DELAY)
	{
		if(NetQuit())
			break;
		if(g_sock)
			UpdNet();
	}

	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	List_free(&g_cn);

	FreeSounds();
	Mix_CloseAudio();
	Mix_Quit();
	SDLNet_Quit();
	SDL_Quit();
}

int HandleEvent(void *userdata, SDL_Event *e)
{
	Widget *gui;
	InEv ie;
	Vec2i old;

	gui = (Widget*)&g_gui;

	ie.intercepted = dmfalse;
	ie.curst = CU_DEFAULT;

	switch(e->type)
	{
		case SDL_QUIT:
			g_quit = dmtrue;
			break;
		case SDL_KEYDOWN:
			ie.type = INEV_KEYDOWN;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			CHECKGLERROR();

			/* Handle copy */
			if( e->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_COPY;
			}
			/* Handle paste */
			if( e->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_PASTE;
			}
			/* Select all */
			if( e->key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_SELALL;
			}

			CHECKGLERROR();
			Widget_inev(gui, &ie);
			CHECKGLERROR();

			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = dmtrue;

			g_keyintercepted = ie.intercepted;
			break;
		case SDL_KEYUP:
			ie.type = INEV_KEYUP;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;

			CHECKGLERROR();
			Widget_inev(gui, &ie);
			CHECKGLERROR();

			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = dmfalse;

			g_keyintercepted = ie.intercepted;
			break;
		case SDL_TEXTINPUT:
			/* UTF8 */
			ie.type = INEV_TEXTIN;
			ie.text = e->text.text;

			CHECKGLERROR();
			Widget_inev(gui, &ie);
			CHECKGLERROR();
			break;

		case SDL_MOUSEWHEEL:
			ie.type = INEV_MOUSEWHEEL;
			ie.amount = e->wheel.y;

			CHECKGLERROR();
				Widget_inev(gui, &ie);
			CHECKGLERROR();
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (e->button.button)
			{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = dmtrue;
				g_moved = dmfalse;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();

				g_keyintercepted = ie.intercepted;
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = dmtrue;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = dmtrue;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (e->button.button)
			{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = dmfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = dmfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = dmfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();
				break;
			}
			break;
		case SDL_MOUSEMOTION:

			if(g_mouseout)
			{
				g_mouseout = dmfalse;
			}

			old = g_mouse;

			if(MousePosition())
			{
				g_moved = dmtrue;

				ie.type = INEV_MOUSEMOVE;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				ie.dx = g_mouse.x - old.x;
				ie.dy = g_mouse.y - old.y;

				CHECKGLERROR();
				Widget_inev(gui, &ie);
				CHECKGLERROR();

				g_curst = ie.curst;
			}
			break;
	}

	return 0;
}

void EventLoop()
{
	CHECKGLERROR();
	while (!g_quit)
	{
		CHECKGLERROR();
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			HandleEvent(NULL, &e);
		}
		CHECKGLERROR();

		StopTimer(TIMER_EVENT);

		if(g_quit)
			break;

		if ( !g_background &&
			( (g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || DrawNextFrame() ) )
		{
			CHECKGLERROR();

			CalcDrawRate();

			CHECKGLERROR();

			Draw();
			CHECKGLERROR();
			SDL_GL_SwapWindow(g_win);
			CHECKGLERROR();

			CHECKGLERROR();
		}

		if((g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || UpdNextFrame() )
		{
			CalcUpdRate();
			Update();
		}

		CHECKGLERROR();
	}
}

#ifdef USESTEAM
//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef PLATFORM_WIN
	::OutputDebugString( pchDebugText );
#endif

	if(!g_applog)
		OpenLog("log.txt", VERSION);

	Log(pchDebugText);


	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		x = x;
	}
}
#endif

void Main()
{
	//*((int*)0) = 0;

#ifdef USESTEAM

	if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steaappid.txt from the game depot.

		return;
	}

	// Init Steam CEG
	if ( !Steamworks_InitCEGLibrary() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "Steamworks_InitCEGLibrary() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (InitDrmLibrary() failed).\n" );
		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if ( !SteamAPI_Init() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "SteamAPI_Init() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

#endif

	Init();

	MakeWin(TITLE);

	SDL_ShowCursor(dmfalse);
	LoadSysRes();
	QueueSimRes();
	FillGUI();

	EventLoop();

	Deinit();
	SDL_ShowCursor(dmtrue);
}

ecbool RunOptions(const char* cmdline)
{
	if(strcmp(cmdline, "") == 0)
	{
		strcpy(g_startmap, "");

		return dmfalse;
	}
	else
	{
		/*
		TODO c90
		std::string cmdlinestr(cmdline);
		std::string find("+devmap ");
		int found = cmdlinestr.rfind(find);

		if(found != std::string::npos)
		{
			strcpy(g_startmap, "");

			startmap = cmdlinestr.psubstr(found+find.length(), cmdlinestr.length()-found-find.length());

			fprintf(g_applog, "%s\r\n", cmdline);
			fprintf(g_applog, "%s\r\n", startmap);

			//LoadMap(startmap.c_str());
			g_startmap = startmap;
		}
		*/
	}
}

#ifdef PLATFORM_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
#ifdef PLATFORM_WIN
	g_hInstance = hInstance;
#endif

#ifdef PLATFORM_WIN
	RunOptions(lpCmdLine);
#else
	char cmdline[DMD_MAX_PATH+124];
	cmdline[0] = 0;
	for(int ai=0; ai<argc; ai++)
	{
		strcat(cmdline, argv[ai]);

		if(ai+1<argc)
			strcat(cmdline, " ");
	}
	RunOptions(cmdline);
#endif

#ifdef PLATFORM_WIN
	if ( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.	

		Main();
		return 0;
	}
#endif

#ifdef PLATFORM_WIN
#ifdef USESTEAM
	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
#endif
#endif
		Main();
#ifdef PLATFORM_WIN
#ifdef USESTEAM
	}
	catch( ... )
	{
		return -1;
	}
#endif
#endif

	return 0;
}
