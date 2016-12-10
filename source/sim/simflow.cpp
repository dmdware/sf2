













#include "simflow.h"

#ifndef MATCHMAKER
#include "../net/client.h"
#endif

//TODO does g_cmdframe need to be unsigned __int64?

unsigned __int64 g_simframe = 0;
unsigned __int64 g_cmdframe = 0;	//net frames follow sim frames except when there's a pause, simframes will stop but cmdframes will continue
unsigned char g_speed = SPEED_PLAY;
ecbool g_gameover = ecfalse;

#ifndef MATCHMAKER
void UpdSpeed()
{
	short fast = 0;
	short play = 0;
	short pause = 0;

	/*
	We need a separate counter for net frames and sim frames,
	because net frames continue while sim frames are paused.
	*/

	//If anybody's paused, we can't continue.
	for(int ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_cl[ci];

		if(!c->on)
			continue;

		switch(c->speed)
		{
		case SPEED_FAST:
			fast++;
			break;
		case SPEED_PLAY:
			play++;
			break;
		case SPEED_PAUSE:
			pause++;
			break;
		default:
			break;
		}
	}

	if(pause)
		g_speed = SPEED_PAUSE;
	else if(play)
		g_speed = SPEED_PLAY;
	else if(fast)
		g_speed = SPEED_FAST;

#if 0
	if(g_localC >= 0)
	{
		Client* c = &g_cl[g_localC];
		c->speed = g_speed;
	}
#endif
}
#endif