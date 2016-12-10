











#include "sound.h"
#include "soundch.h"
#include "../utils.h"

Sound g_sound[SOUNDS];
int g_volume = 100;
short g_beep = -1;

void SetVol(int percent)
{
	g_volume = percent;
	Mix_Volume(-1, MIX_MAX_VOLUME*g_volume/100);
}

void SoundPath(const char* from, char* to)
{
	char intermed[64];
	StripPathExt(from, intermed);
	sprintf(to, "%s.wav", intermed);
}

Sound::Sound(const char* fp)
{
	FullPath(fp, filepath);
	//strcpy(filepath, fp);
	sample = Mix_LoadWAV(filepath);
}

Sound::~Sound()
{
	if(sample)
	{
		Mix_FreeChunk(sample);
		sample = NULL;
	}
}

int NewSound()
{
	for(int i=0; i<SOUNDS; i++)
		if(!g_sound[i].on)
			return i;

	return -1;
}

ecbool QueueSound(const char* relative, short* index)
{
	return ecfalse;
}

ecbool LoadSound(const char* relative, short* index)
{
	*index = -1;

	int i = NewSound();

	if(i < 0)
		return ecfalse;

	Sound* s = &g_sound[i];

	s->on = ectrue;
	strcpy(s->filepath, relative);
	char full[SFH_MAX_PATH+1];
	FullPath(relative, full);
	s->sample = Mix_LoadWAV(full);

	if(!s->sample)
	{
		char msg[1280];
		sprintf(msg, "Error loading sound %s\n %s", relative, Mix_GetError());
		ErrMess("Error", msg);
		return ecfalse;
	}

	*index = i;

	return ectrue;
}

void FreeSounds()
{
	for(int si=0; si<SOUNDS; si++)
	{
		Sound* s = &g_sound[si];

		if(!s->on)
			continue;

		if(s->sample)
		{
			Mix_FreeChunk(s->sample);
			s->sample = NULL;
		}

		s->on = ecfalse;
	}
}

void ReloadSounds()
{
}

int g_freechan = SOUND_CHANNELS-1;

void PlayClip(short si)
{
	//return;	//temp

	if(si < 0)
		return;

	//InfoMess("p","a");
	
	if(!g_freechan)
		return;

	Sound* s = &g_sound[si];
	int ret = Mix_PlayChannel(-1, s->sample, 0);
	g_freechan--;
	
#ifdef PLATFORM_MOBILE
	if(ret < 0)
	{
		NSLog(@"Mix_PlayChannel error: %s\r\n", SDL_GetError());
	}
#endif
		
}

void ChannelDone(int channel)
{
	g_freechan++;
}

void Sound::play()
{
#if 0
#ifdef PLATFORM_WIN
	PlayClip(filepath, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
#endif
#endif
	if(!g_freechan)
		return;

	Mix_PlayChannel(-1, sample, 0);
	g_freechan--;
}

void Sound_Order()
{
	//g_sound[0].play();
}
