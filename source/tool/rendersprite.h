

#ifndef RENDERSPRITE_H
#define RENDERSPRITE_H

#include "../platform.h"
#include "../math/vec2i.h"

#define RENDERPITCH	30
#define RENDERYAW	45

extern char g_renderpath[SFH_MAX_PATH+1];
extern char g_renderbasename[SFH_MAX_PATH+1];
extern int g_rendertype;
extern int g_renderframes;
extern float g_transpkey[3];
extern int g_nrendsides;

extern ecbool g_doframes;
extern ecbool g_dosides;
extern ecbool g_doinclines;
extern ecbool g_dorots;
extern ecbool g_usepalette;
extern int g_savebitdepth;

extern int g_origwidth;
extern int g_origheight;

extern int g_deswidth;
extern int g_desheight;
extern unsigned int g_rendertex[4];
extern unsigned int g_renderrb[4];
extern unsigned int g_renderfb[4];

#define RENDER_BUILDING		0
#define RENDER_UNIT			1
#define RENDER_TERRTILE		2
#define RENDER_ROAD			3
#define RENDER_UNSPEC		4
#define RENDER_MODEL		6

#define RENDSTAGE_DUMMY		0
#define RENDSTAGE_SHADOW	1
#define RENDSTAGE_COLOR		2
#define RENDSTAGE_TEAM		3
#define RENDSTAGE_DEPTH		4
#define RENDSTAGE_ELEV		5

extern ecbool g_antialias;
extern ecbool g_fit2pow;
extern ecbool g_exportdepth;
extern ecbool g_exportteam;
extern ecbool g_exportdiff;
extern ecbool g_hidetexerr;
extern int g_nslices;
extern int g_slicex;
extern int g_slicey;

//should be a power of two,
//otherwise there's some space-saving optimizations i could've made
//in SaveRender();... checking if downsample size fits in a smaller image.
//edit: MUST be a power of two. until i fix it.
#define ANTIALIAS_UPSCALE	4

//Arbitrary max texture size, needed for determining upscale vertex screen coordinates.
//If it's too small for the upscale sprite, the coordinates won't fit and the sprite won't be rendered at those points.
#define MAX_TEXTURE		4096

extern int g_rendspl;	//output rendered sprite list index

struct SpListToRender
{
public:
	std::string relative;
	int rendtype;
	std::list<int*> outspl;
	ecbool doframes;
	ecbool doinclines;
	ecbool dosides;
	ecbool dorots;
	int nframes;
	int nsides;
	ecbool expdiff;
	ecbool expdepth;
	ecbool expteam;
	ecbool expelev;
	int nslices;
	ecbool (*prepfunc)(SpListToRender* torend);
};

struct LoadedTex;

extern std::vector<SpListToRender> g_spltorend;
extern int g_lastRSp;

ecbool Rend1();
void QueueRend(const char* relative, int rendtype,
			   unsigned int* outspl,
				   ecbool doframes,
				   ecbool doinclines,
				   ecbool dosides,
				   ecbool dorots,
				   int nframes,
				   int nsides,
				   ecbool expdepth,
				   ecbool expteam,
				   ecbool expelev,
				   ecbool expdiff,
				   int nslices);

void AdjustFrame(ecbool resetview=ectrue);
void PrepareRender(//const char* fullpath, 
				   int rendtype, std::list<int*>& outspl,
				   ecbool doframes,
				   ecbool doinclines,
				   ecbool dosides,
				   ecbool dorots,
				   int nframes,
				   int nsides,
				   ecbool expdepth,
				   ecbool expteam,
				   ecbool expelev,
				   ecbool expdiff,
				   int nslices);
void UpdRend();
void RotateView();
void EndRender();
void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height);
void SaveRender(int rendstage, LoadedTex* finalsprite);
void LoadRender(int rendstage, LoadedTex* finalsprite);
void NameSp(char* fullpath, int rendstage);
void CreateSp(int rendstage, LoadedTex* finalsprite);
void AddTile(const char* relativepath);
void FreeTile();
void RenderQueue();

#endif
