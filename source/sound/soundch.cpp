













#include "soundch.h"

SoundCh g_soundch[SOUND_CHANNELS];

int NewChan()
{
	int best = 0;
	unsigned __int64 bestd = 0;
	unsigned __int64 now = GetTicks();

	for(unsigned char i=0; i<SOUND_CHANNELS; i++)
	{
		SoundCh* c = &g_soundch[i];
		unsigned __int64 d = now - c->last;

		if(d < bestd)
			continue;

		bestd = d;
		best = i;
	}

	return best;
}