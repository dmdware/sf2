











#ifndef SOUND_H
#define SOUND_H

#include "../platform.h"

struct Sound
{
public:
	Sound() 
	{ 
		on = ecfalse;
		sample = NULL;
	}
	Sound(const char* fp);
	~Sound();

	ecbool on;
	Mix_Chunk *sample;
	char filepath[SFH_MAX_PATH+1];
	void play();
};

#define SOUNDS	1024

extern Sound g_sound[SOUNDS];

struct SoundLoad
{
	char fullpath[SFH_MAX_PATH+1];
	int* retindex;
};

//TO DO queue sound

extern int g_volume;

extern short g_beep;

void SoundPath(const char* from, char* to);
ecbool QueueSound(const char* relative, short* index);
ecbool LoadSound(const char* relative, short* index);
void FreeSounds();
void ReloadSounds();
void PlayClip(short si);
void SetVol(int percent);
void ChannelDone(int channel);

void Sound_Order();

#endif
