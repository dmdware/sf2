











#ifndef SPRITE_H
#define SPRITE_H

#include "../texture.h"

struct Sprite
{
public:

	Sprite();
	~Sprite();
	void free();

	ecbool on;
	unsigned int difftexi;
	unsigned int teamtexi;
	unsigned int depthtexi;
	unsigned int elevtexi;
	float offset[4];	//the pixel texture coordinates centered around a certain point, for use for vertex positions when not cropping (old)
	float crop[4];	//the texture coordinates [0..1] of the cropped sprite, used for the texture coords
	float cropoff[4];	//the pixel texture coordinates of the cropped sprite, used for vertex positions
	LoadedTex* pixels;
	std::string fullpath;
};

//#define SPRITES	4096
#define SPRITES	16096
extern Sprite g_sprite[SPRITES];

struct SpList
{
public:
	ecbool on;
	ecbool inclines;
	ecbool sides;
	int nsides;
	ecbool rotations;
	ecbool frames;
	int nframes;
	int nslices;
	std::string fullpath;

	int nsp;
	unsigned int* sprites;

	void free()
	{
		nsp = 0;
		delete [] sprites;
		sprites = NULL;
		on = ecfalse;
		nslices = 1;
	}

	SpList()
	{
		nsp = 0;
		sprites = NULL;
		on = ecfalse;
		nslices = 1;
	}

	~SpList()
	{
		free();
	}
};

#define SPRITELISTS	256
extern SpList g_splist[SPRITELISTS];

struct SpriteToLoad
{
public:
	std::string relative;
	unsigned int* spindex;
	ecbool loadteam;
	ecbool loaddepth;
};

extern std::vector<SpriteToLoad> g_spriteload;

extern int g_lastLSp;

ecbool Load1Sprite();
void FreeSprites();
ecbool LoadSprite(const char* relative, unsigned int* spindex, ecbool loadteam, ecbool loaddepth);
void QueueSprite(const char* relative, unsigned int* spindex, ecbool loadteam, ecbool loaddepth);
void ParseSprite(const char* relative, Sprite* s);
ecbool PlayAnim(float& frame, int first, int last, ecbool loop, float rate);
ecbool PlayAnimB(float& frame, int first, int last, ecbool loop, float rate);	//Play animation backwards
int SpriteRef(SpList* sl, int frame, int incline, int pitch, int yaw, int roll,
				  int slicex, int slicey);
ecbool LoadSpriteList(const char* relative, unsigned int* splin, ecbool loadteam, ecbool loaddepth, ecbool queue);
int NewSpriteList();
int NewSprite();

#endif
