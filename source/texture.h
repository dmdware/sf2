











#ifndef TEXTURE_H
#define TEXTURE_H

#include "platform.h"
#include "algo/vector.h"
#include "algo/bool.h"
#include "math/vec2i.h"

struct Texture
{
	unsigned int texname;
	char* fullpath;
	ecbool loaded;
	int width, height;
	ecbool transp;
	ecbool sky;
	ecbool breakable;
	ecbool passthru;
	ecbool grate;
	ecbool ladder;
	ecbool water;
	ecbool fabric;
	ecbool clamp;
	ecbool mipmaps;
};

typedef struct Texture Texture;

#define TEXTURES	40960
extern Texture g_texture[TEXTURES];

#define TGA_RGB		 2		// This tells us it's a normal RGB (really BGR) file
#define TGA_A		 3		// This tells us it's an ALPHA file
#define TGA_RLE		10		// This tells us that the targa is Run-Length Encoded (RLE)

#ifndef int_p_NULL
#define int_p_NULL (int*)NULL
#endif
#define png_infopp_NULL (png_infopp)NULL
#define png_voidp_NULL	(png_voidp)NULL

#define JPEG_BUFFER_SIZE (8 << 10)

struct LoadedTex
{
	int channels;			// The channels in the image (3 = RGB : 4 = RGBA)
	int sizex;				// The width of the image in pixels
	int sizey;				// The height of the image in pixels
	unsigned char *data;	// The image pixel data
};

typedef struct LoadedTex LoadedTex;

void LoadedTex_init(LoadedTex *lt);
void LoadedTex_free(LoadedTex *lt);

struct TextureToLoad
{
	unsigned int* ptexindex;
	unsigned int texindex;
	char relative[DMD_MAX_PATH+1];
	ecbool clamp;
	ecbool reload;
	ecbool mipmaps;
};

typedef struct TextureToLoad TextureToLoad;

extern Vector g_texload;

extern int g_lastLTex;

void Tex_init(Texture *tex);
void Tex_free(Texture *tex);

LoadedTex *LoadTGA(const char *fullpath);
void DecodeJPG(jpeg_decompress_struct* cinfo, LoadedTex *pImageData);
LoadedTex *LoadJPG(const char *fullpath);
LoadedTex *LoadPNG(const char *fullpath);
ecbool FindTexture(unsigned int &texture, const char* relative);
int NewTexture();
ecbool TextureLoaded(unsigned int texture, const char* relative, ecbool transp, ecbool clamp, ecbool mipmaps, unsigned int& texindex);
void FindTextureExtension(char *relative);
void FreeTextures();
ecbool Load1Texture();
void QueueTex(unsigned int* texindex, const char* relative, ecbool clamp, ecbool mipmaps);
void RequeueTex(unsigned int texindex, const char* relative, ecbool clamp, ecbool mipmaps);
LoadedTex* LoadTexture(const char* full);
ecbool CreateTex(LoadedTex* pImage, unsigned int* texname, ecbool clamp, ecbool mipmaps);
ecbool CreateTex(unsigned int &texindex, const char* relative, ecbool clamp, ecbool mipmaps, ecbool reload=ecfalse);
void ReloadTextures();
void FreeTexture(const char* relative);
void FreeTexture(int i);
void DiffPath(const char* basepath, char* diffpath);
void DiffPathPNG(const char* basepath, char* diffpath);
void SpecPath(const char* basepath, char* specpath);
void NormPath(const char* basepath, char* normpath);
void OwnPath(const char* basepath, char* ownpath);
void AllocTex(LoadedTex* empty, int width, int height, int channels);
void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos);
void SaveJPEG(const char* fullpath, LoadedTex* image, float quality);
int SavePNG(const char* fullpath, LoadedTex* image);
void FlipImage(LoadedTex* image);
ecbool SaveRAW(const char* fullpath, LoadedTex* image);
void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim);
void Extract(LoadedTex* original, LoadedTex* empty, int x1, int y1, int x2, int y2);


#endif
