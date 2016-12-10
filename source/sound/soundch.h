













#ifndef SOUNDCH_H
#define SOUNDCH_H

#include "../platform.h"
#include "../utils.h"

#define SOUND_CHANNELS	16

struct SoundCh
{
public:
	ecbool done;
	unsigned __int64 last;

	SoundCh(){ last = GetTicks(); done = ectrue; }
};

extern SoundCh g_soundch[SOUND_CHANNELS];

int NewChan();

#endif
