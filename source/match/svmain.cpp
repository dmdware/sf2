












#define MATCHMAKER

#include "svmain.h"
#include "../platform.h"
#include "../utils.h"
#include "../window.h"
#include "../net/net.h"
#include "../net/netconn.h"
#include "../net/readpackets.h"
#include "../net/sendpackets.h"
#include "../net/packets.h"

bool g_quit = false;

void CheckStop()
{
	FILE* fp = fopen("stop", "r");

	if(!fp)
		return;

	fclose(fp);
	unlink("stop");

	g_applog<<"Stopping on command."<<std::endl;
	g_applog.flush();
	g_quit = true;
}

int main(int argc, char* argv[])
{
	if(SDL_Init(0) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		std::cout<<msg<<std::endl;
	}

	if(SDLNet_Init() == -1) 
	{
		char msg[1280];
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		std::cout<<msg<<std::endl;
	}

	OpenLog("log.txt", VERSION);
	
	g_applog<<"NETCONN_TIMEOUT = "<<NETCONN_TIMEOUT<<std::endl;
	g_applog.flush();
	
	g_netmode = NETM_HOST;

	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	if(!(g_sock = SDLNet_UDP_Open(PORT)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		//ErrorMessage("Error", msg);
		g_applog<<msg<<std::endl;
		return 1;
	}
	
	while(!g_quit)
	{
		UpdNet();
		
		//if(UpdNextFrame())
		{
			CheckStop();
			//CheckClients();
			//Update();
		}
		//else
			SDL_Delay(1);
	}
	
	SDLNet_Quit();
	SDL_Quit();
	
	return 0;
}