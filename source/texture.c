











#include "platform.h"
#include "texture.h"
#include "gui/gui.h"
#include "utils.h"
#include "debug.h"
#include "app/appmain.h"

Texture g_texture[TEXTURES];
Vector g_texload; /* TextureToLoad */

int g_texwidth;
int g_texheight;
int g_lastLTex = -1;

ecbool g_hidetexerr = ecfalse;
ecbool g_usepalette = ecfalse;
int g_savebitdepth = 8;


void LoadedTex_init(LoadedTex *lt)
{
	lt->data = NULL;
}

void LoadedTex_free(LoadedTex *lt)
{
	free(lt->data);
	lt->data = NULL;
}

void Tex_init(Texture *tex)
{
	tex->filepath = NULL;
	tex->loaded = ecfalse;
}

void Tex_free(Texture *tex)
{
	free(&tex->fullpath);
	tex->filepath = NULL;
	tex->loaded = ecfalse;
}

LoadedTex* LoadJPG(const char *strFileName)
{
	LoadedTex *pImageData = NULL;
	struct jpeg_decompress_struct cinfo;

	jpeg_error_mgr jerr;

	//pImageData = (Image*)malloc(sizeof(Image));
	pImageData = new LoadedTex;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	FILE* fp = fopen(strFileName, "rb");

	if (!fp)
	{
		delete pImageData;
		Log("Error opening jpeg %s", strFileName);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long myAssetLength = ftell(fp);
	unsigned char* ucharRawData = (unsigned char*)malloc(myAssetLength);
	fseek(fp, 0, SEEK_SET);
	fread(ucharRawData, 1, myAssetLength, fp);
	fclose(fp);

	// the jpeg_stdio_src alternative func, which is also included in IJG's lib.
	jpeg_mem_src(&cinfo, ucharRawData, myAssetLength);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	pImageData->channels = cinfo.num_components;
	pImageData->sizex    = cinfo.image_width;
	pImageData->sizey    = cinfo.image_height;

	int rowSpan = cinfo.image_width * cinfo.num_components;

	pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizey));

	unsigned char** rowPtr = new unsigned char*[pImageData->sizey];

	for (int i = 0; i < pImageData->sizey; i++)
		rowPtr[i] = &(pImageData->data[i * rowSpan]);

	int rowsRead = 0;

	while (cinfo.output_scanline < cinfo.output_height)
		rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);

	delete [] rowPtr;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	free(ucharRawData);

	return pImageData;
}

static FILE* loadpngfp = NULL;

void png_file_read(png_structp png_ptr, png_bytep data, png_size_t length) 
{
	fread(data, 1, length, loadpngfp);
}

LoadedTex *LoadPNG(const char *strFileName)
{
	LoadedTex *pImageData = NULL;

	loadpngfp = fopen(strFileName, "rb");

	if(!loadpngfp)
	{
		return NULL;
	}

	//header for testing if it is a png
	png_byte header[8];

	fread(header, 8, 1, loadpngfp);
	//g_src.read((void*)header, 8);

	//test if png
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png) 
	{
		Log("Not a png file : %s %d,%d,%d,%d,%d,%d,%d,%d", strFileName, 
			(int)header[0], (int)header[1], (int)header[2], (int)header[3], 
			(int)header[4], (int)header[5], (int)header[6], (int)header[7]);
		fclose(loadpngfp);
		return NULL;
	}

	//create png struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) 
	{
		Log("Unable to create png struct : %s", strFileName);
		fclose(loadpngfp);
		return NULL;
	}

	//create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) 
	{
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		Log("Unable to create png info : %s", strFileName);
		fclose(loadpngfp);
		return NULL;
	}

	//create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) 
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		Log("Unable to create png end info : %s", strFileName);
		fclose(loadpngfp);
		return NULL;
	}

	//png error stuff, not sure libpng man suggests this.
	if (setjmp(png_jmpbuf(png_ptr))) 
	{
		Log("Error during setjmp : %s", strFileName);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(loadpngfp);
		return NULL;
	}

	png_set_read_fn(png_ptr, NULL, png_file_read);

	//let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);

	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	//variables to pass to get info
	int bit_depth, color_type;
	png_uint_32 twidth, theight;

	// get info about png
	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	//	ecbool alphaFlag;

	//pImageData = (Image*)malloc(sizeof(Image));
	pImageData = new LoadedTex;

	pImageData->sizex = twidth;
	pImageData->sizey = theight;

	png_bytep trans_alpha = NULL;
	int num_trans = 0;
	png_color_16p trans_color = NULL;

	switch( color_type )
	{
	case PNG_COLOR_TYPE_RGBA:
		pImageData->channels = 4;
		break;
	case PNG_COLOR_TYPE_RGB:
		pImageData->channels = 3;
		break;
	case PNG_COLOR_TYPE_PALETTE:
		{
			png_set_palette_to_rgb(png_ptr);
			pImageData->channels = 3;
#if 0
			png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
			if (trans_alpha != NULL)
				alphaFlag = ectrue;
			else
				alphaFlag = ecfalse;
			if(alphaFlag)
				pImageData->channels = 4;
#endif

			//if(alphaFlag)
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			{
				pImageData->channels = 4;
				png_set_tRNS_to_alpha(png_ptr);
			}
		}
		break;
	default:
		Log("%s color type %d not supported", strFileName, (int)png_get_color_type(png_ptr, info_ptr));

		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		delete pImageData;
		fclose(loadpngfp);
		return NULL;
	}

	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// Row size in bytes.
	int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

	pImageData->data = (unsigned char*) malloc(row_bytes * pImageData->sizey);

	// Allocate the image_data as a big block, to be given to opengl
	if(!pImageData->data)
	{
		//clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		Log("Unable to allocate image_data while loading %s ", strFileName);
		delete pImageData;
		fclose(loadpngfp);
		return NULL;
	}

	//row_pointers is for pointing to image_data for reading the png with libpng
	png_bytep *row_pointers = new png_bytep[pImageData->sizey];
	if (!row_pointers) 
	{
		//clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(pImageData->data);
		delete pImageData;
		Log("Unable to allocate row_pointer while loading %s ", strFileName);
		fclose(loadpngfp);
		return NULL;
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	for (int i = 0; i < pImageData->sizey; ++i)
		row_pointers[i] = pImageData->data + i * row_bytes;

	//read the png into image_data through row_pointers
	png_read_image(png_ptr, row_pointers);

	//clean up memory and close stuff
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	delete[] row_pointers;
	fclose(loadpngfp);

	return pImageData;
}

ecbool FindTexture(unsigned int *textureidx, const char* relative)
{
	char corrected[1024];
	Texture* t;
	int i;

	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(i=0; i<TEXTURES; ++i)
	{
		t = g_texture+i;

		if(t->loaded && strcmp(t->fullpath, corrected) == 0)
		{
			*textureidx = i;
			return ectrue;
		}
	}

	return ecfalse;
}

void FreeTexture(const char* relative)
{
	int i;
	char corrected[DMD_MAX_PATH+1];
	Texture* t;

	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(i=0; i<TEXTURES; i++)
	{
		t = g_texture+i;

		if(t->loaded && strcmp(t->fullpath, corrected) == 0)
		{
			t->loaded = ecfalse;
			glDeleteTextures(1, &t->texname);
			return;
		}
	}
}

void FreeTexture(int i)
{
	Texture* t;

	if(i < 0)
		return;

	t = g_texture+i;

	if(t->loaded)
	{
		t->loaded = ecfalse;
		glDeleteTextures(1, &t->texname);
	}
}

int NewTexture()
{
	for(int i=0; i<TEXTURES; i++)
		if(!g_texture[i].loaded)
			return i;

	return -1;
}
//extern int thetex;
ecbool TextureLoaded(unsigned int texture, const char* relative, ecbool transp, ecbool clamp, ecbool mipmaps, unsigned int* texindex, ecbool reload)
{
	char corrected[1024];
	Texture* t;

	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	if(!reload)
	{
		*texindex = NewTexture();

		if((int)texindex < 0)
		{
			texindex = 0;	// Give a harmless texture index
			return ecfalse;
		}
	}

	t = &g_texture[texindex];
	t->loaded = ectrue;
	t->fullpath = corrected;
	t->texname = texture;
	t->width = g_texwidth;
	t->height = g_texheight;
	t->transp = transp;
	t->clamp = clamp;
	t->mipmaps = mipmaps;

	return ectrue;
}

void FreeTextures()
{
	int i;

	for(i=0; i<TEXTURES; i++)
	{
		if(!g_texture[i].loaded)
			continue;

		glDeleteTextures(1, &g_texture[i].texname);
		g_texture[i].loaded = ecfalse;	// Needed to reload textures. EDIT: not.
	}
}

void FindTextureExtension(char *relative)
{
	char strJPGPath[DMD_MAX_PATH] = {0};
	char strPNGPath[DMD_MAX_PATH] = {0};
	FILE *fp = NULL;

	FullPath("", strJPGPath);

	strcat(strJPGPath, relative);
	strcpy(strPNGPath, strJPGPath);

	strcat(strJPGPath, ".jpg");
	strcat(strPNGPath, ".png");

	if((fp = fopen(strJPGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".jpg");
		return;
	}

	if((fp = fopen(strPNGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".png");
		return;
	}
}

ecbool Load1Texture()
{
	TextureToLoad* t;

	if(g_lastLTex+1 < (int32_t)g_texload.size)
		SetStatus(g_texload[g_lastLTex+1].relative);

	CHECKGLERROR();

	if(g_lastLTex >= 0)
	{
		t = &g_texload[g_lastLTex];
		if(t->reload)
			CreateTex(t->texindex, t->relative, t->clamp, t->mipmaps, t->reload);
		else
			CreateTex(*t->ptexindex, t->relative, t->clamp, t->mipmaps, t->reload);
	}

	g_lastLTex ++;

	if(g_lastLTex >= (int32_t)g_texload.size())
	{
		return ecfalse;	// Done loading all textures
	}

	return ectrue;	// Not finished loading textures
}

void QueueTex(unsigned int* texindex, const char* relative, ecbool clamp, ecbool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.ptexindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = ecfalse;
	toLoad.mipmaps = mipmaps;

	g_texload.push_back(toLoad);
}

void RequeueTex(unsigned int texindex, const char* relative, ecbool clamp, ecbool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.texindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = ectrue;
	toLoad.mipmaps = mipmaps;

	g_texload.push_back(toLoad);
}

void ReloadTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(!t->loaded)
			continue;

		std::string rel;
		rel = MakeRelative(t->fullpath.c_str());
		RequeueTex(i, rel.c_str(), t->clamp, t->mipmaps);
	}
}

LoadedTex* LoadTexture(const char* full)
{
	if(strstr(full, ".jpg") || strstr(full, ".JPG") || strstr(full, ".jpeg") || strstr(full, ".JPEG") || strstr(full, ".jpe") || strstr(full, ".JPE"))
	{
		return LoadJPG(full);
	}
	else if(strstr(full, ".png") || strstr(full, ".PNG"))
	{
		return LoadPNG(full);
	}
	else if(strstr(full, ".tga") || strstr(full, ".TGA"))
	{
		return LoadTGA(full);
	}

	Log("Unrecognized texture file extension: %s. Only .jpg .png .tga are accepted.", full);

	return NULL;
}

ecbool CreateTex(LoadedTex* pImage, unsigned int* texname, ecbool clamp, ecbool mipmaps)
{
	if(!pImage)
		return ecfalse;
	if(!pImage->data)
		return ecfalse;

	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, texname);

	CHECKGLERROR();
	// This sets the alignment requirements for the start of each pixel row in memory.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	CHECKGLERROR();

	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, *texname);

	CHECKGLERROR();
	// Assume that the texture is a 24 bit RGB texture (We convert 16-bit ones to 24-bit)
	int internalFormat = GL_RGB8;
	int textureType = GL_RGB;
	ecbool transp = ecfalse;

	// If the image is 32-bit (4 channels), then we need to specify GL_RGBA for an alpha
	if(pImage->channels == 4)
	{
		internalFormat = GL_RGBA8;
		textureType = GL_RGBA;
		transp = ectrue;
	}
	//grayscale
	else if(pImage->channels == 1)
	{
		internalFormat = GL_ALPHA8;
		textureType = GL_ALPHA;
	}

	CHECKGLERROR();

#if 0	//old

	if(mipmaps)
	{
#if 0
		glEnable(GL_TEXTURE_2D);	// ATI fix
		// Option 1: with mipmaps

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		//glGenerateMipmap(GL_TEXTURE_2D);
		gluBuild2DMipmaps(GL_TEXTURE_2D, textureType, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);

		CheckGLError(__FILE__, __LINE__);
#else
		//if(transp)
		if(ectrue)
		{
			//linear filter, better for thin details with transparency

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			if(clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

			CheckGLError(__FILE__, __LINE__);
		}
		else
		{
			glEnable(GL_TEXTURE_2D);	// ATI fix
			// Option 1: with mipmaps

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if(clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
			//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
			//glGenerateMipmap(GL_TEXTURE_2D);
			gluBuild2DMipmaps(GL_TEXTURE_2D, textureType, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);

			CheckGLError(__FILE__, __LINE__);
		}
#endif
	}
	else
	{
		// Option 2: without mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

		CheckGLError(__FILE__, __LINE__);
	}

	CheckGLError(__FILE__, __LINE__);
#endif


#if 1

	//Log("mipmaps:"<<(int)mipmaps<<" :"<<relative);

	if(mipmaps)
	{
		glEnable(GL_TEXTURE_2D);	// ATI fix
		// Option 1: with mipmaps

		CHECKGLERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		CHECKGLERROR();
#ifdef PLATFORM_GL14
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		CHECKGLERROR();

		if(clamp)
		{
#ifdef PLATFORM_GL14
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif

#ifdef PLATFORM_GLES20
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

		glGenerateMipmap(GL_TEXTURE_2D);  //undeclared identifier in xcode!?

		//gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, pImage->sizex, pImage->sizey, textureType, GL_UNSIGNED_BYTE, pImage->data);  //undeclared identifier in xcode!?

		CHECKGLERROR();
	}
	else
	{
		// Option 2: without mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		CHECKGLERROR();

		if(clamp)
		{
#ifdef PLATFORM_GL14
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif

#ifdef PLATFORM_GLES20
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

		CHECKGLERROR();
	}

	CHECKGLERROR();
#else
	// Option 3: without mipmaps linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizex, pImage->sizey, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
#endif

	glBindTexture(GL_TEXTURE_2D, -1);
	return ectrue;
}

ecbool CreateTex(unsigned int &texindex, const char* relative, ecbool clamp, ecbool mipmaps, ecbool reload)
{
	CHECKGLERROR();

	if(!relative)
		return ecfalse;

	if(!reload)
		if(FindTexture(texindex, relative))
			return ectrue;

	// Define a pointer to a LoadedTex
	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadTexture(full);

	// Make sure valid image data was given to pImage, otherwise return ecfalse
	if(pImage == NULL)
	{
		Log("Failed to load %s\r\n", relative);


		if(!reload)
			texindex = 0;	// Give a harmless texture index

		return ecfalse;
	}

	unsigned int texname;

	if(!CreateTex(pImage, &texname, clamp, mipmaps))
	{
		delete pImage;								// Free the image structure
		return ecfalse;
	}

	ecbool transp = ecfalse;

	if(pImage->channels == 4)
	{
		transp = ectrue;
	}

	g_texwidth = pImage->sizex;
	g_texheight = pImage->sizey;

	TextureLoaded(texname, relative, transp, clamp, mipmaps, texindex, reload);

	if(pImage)
	{
		delete pImage;								// Free the image structure

		Log(relative);
	}

	// Return a success
	return ectrue;
}

void RequeueTextures()
{
	FreeTextures();

	for(int i=0; i<TEXTURES; i++)
	{
		if(g_texture[i].loaded)
			RequeueTex(i, g_texture[i].fullpath.c_str(), g_texture[i].clamp, g_texture[i].mipmaps);
	}

	//LoadParticles();
	//LoadProjectiles();
	//LoadTerrainTextures();
	//LoadUnitSprites();
	//BSprites();
}

void DiffPath(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExt(diffpath);
	strcat(diffpath, ".jpg");
}

void DiffPathPNG(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExt(diffpath);
	strcat(diffpath, ".png");
}

void SpecPath(const char* basepath, char* specpath)
{
	strcpy(specpath, basepath);
	//StripExt(specpath);
	strcat(specpath, ".spec.jpg");
}

void NormPath(const char* basepath, char* normpath)
{
	strcpy(normpath, basepath);
	//StripExt(normpath);
	strcat(normpath, ".norm.jpg");
}

void OwnPath(const char* basepath, char* ownpath)
{
	strcpy(ownpath, basepath);
	//StripExt(ownpath);
	strcat(ownpath, ".team.png");
}

void AllocTex(LoadedTex* empty, int width, int height, int channels)
{
	empty->data = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
	empty->sizex = width;
	empty->sizey = height;
	empty->channels = channels;

	if(!empty->data)
	{
		OutOfMem(__FILE__, __LINE__);
	}
}

void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos)
{
	if(src == NULL || src->data == NULL)
		return;

	for(int x=0; x<src->sizex; x++)
	{
		if(x+pos.x < 0)
			continue;

		if(x+pos.x >= dest->sizex)
			continue;

		for(int y=0; y<src->sizey; y++)
		{
			if(y+pos.y < 0)
				continue;

			if(y+pos.y >= dest->sizey)
				continue;

			int srcpixel = x*src->channels + y*src->channels*src->sizex;
			int destpixel = (x+pos.x)*dest->channels + (y+pos.y)*dest->channels*dest->sizex;

			dest->data[destpixel + 0] = src->data[srcpixel + 0];
			dest->data[destpixel + 1] = src->data[srcpixel + 1];
			dest->data[destpixel + 2] = src->data[srcpixel + 2];

			if(dest->channels > 3)
			{
				if(src->channels <= 3)
					dest->data[destpixel + 3] = 255;
				else
					dest->data[destpixel + 3] = src->data[srcpixel + 3];
			}
		}
	}
}

static Vector my_buffer; /* JOCTET */
#define BLOCK_SIZE 16384

void my_init_destination(j_compress_ptr cinfo)
{
	my_buffer.resize(BLOCK_SIZE);
	Vector_init(&my_buffer, sizeof(JOCTET));
	cinfo->dest->next_output_byte = &my_buffer[0];
	cinfo->dest->free_in_buffer = my_buffer.size();
}

boolean my_empty_output_buffer(j_compress_ptr cinfo)
{
	size_t oldsize = my_buffer.size();
	my_buffer.resize(oldsize + BLOCK_SIZE);
	cinfo->dest->next_output_byte = &my_buffer[oldsize];
	cinfo->dest->free_in_buffer = my_buffer.size() - oldsize;
	return ectrue;
}

void my_term_destination(j_compress_ptr cinfo)
{
	my_buffer.resize(my_buffer.size() - cinfo->dest->free_in_buffer);
}


void SaveJPEG(const char* fullpath, LoadedTex* image, float quality)
{
	FILE *outfile;
	if ((outfile = fopen(fullpath, "wb")) == NULL)
	{
		return;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, NULL);
	cinfo.dest->init_destination = &my_init_destination;
	cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
	cinfo.dest->term_destination = &my_term_destination;

	cinfo.image_width      = image->sizex;
	cinfo.image_height     = image->sizey;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);

	cinfo.dest->init_destination = &my_init_destination;
	cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
	cinfo.dest->term_destination = &my_term_destination;

	/*set the quality [0..100]  */
	//have to use TRUE instead of ectrue or else doesn't work
	//in xcode which requires conversion to custom type boolean
	//jpeg_set_quality (&cinfo, 100*quality, TRUE);
	//jpeg_start_compress(&cinfo, TRUE);
	jpeg_set_quality (&cinfo, (int32_t)(100*quality), (boolean)TRUE);
	jpeg_start_compress(&cinfo, (boolean)TRUE);

	JSAMPROW row_pointer;
	int row_stride = image->sizex * 3;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &image->data[cinfo.next_scanline*row_stride];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fwrite(&my_buffer[0], my_buffer.size(), 1, outfile);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	Vector_free(&my_buffer);
}

static FILE* savepngfp = NULL;

void png_file_write(png_structp png_ptr, png_bytep data, png_size_t length) 
{
	fwrite(data, 1, length, savepngfp);
}

void png_file_flush(png_structp png_ptr)
{
	fflush(savepngfp);
}

//only for palette+alpha
void Palletize(png_color_16 Colors[PNG_MAX_PALETTE_LENGTH],
			   png_byte Trans[PNG_MAX_PALETTE_LENGTH],
			   int* npal,
			   LoadedTex* lt,
			   png_colorp palette)
{
	*npal = 0;

	//1	
	int* closest = new int [lt->sizex * lt->sizey];
	int mentions[PNG_MAX_PALETTE_LENGTH];
	memset(closest, 0, lt->sizex * lt->sizey * sizeof(int));
	memset(mentions, 0, PNG_MAX_PALETTE_LENGTH * sizeof(int));

	for(int x=0; x<lt->sizex; ++x)
	{
		//tryout:
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = -1;
			int neard = -1;


			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));

				if(nearpi < 0)
					nearpi = pi;
				if(neard < 0)
					neard = d;

				//if(mentions[pi] <= mentions[nearpi] &&
				//	d >= neard)
				if(d < neard)
				{
					nearpi = pi;
					neard = d;
				}
			}

			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));

				if(nearpi < 0)
					nearpi = pi;
				if(neard < 0)
					neard = d;

				if(mentions[pi] <= mentions[nearpi] &&
					d >= neard)
				{
					nearpi = pi;
					neard = d;
					if(d > 0)
						mentions[pi] = 0;
					//x=0;
					//y=0;
					//goto tryout;
				}
			}

			closest[(x+y*lt->sizex)] = nearpi;
			mentions[nearpi]++;
			palette[nearpi].red = lt->data[lt->channels*(x+y*lt->sizex)+0];
			palette[nearpi].green = lt->data[lt->channels*(x+y*lt->sizex)+1];
			palette[nearpi].blue = lt->data[lt->channels*(x+y*lt->sizex)+2];
		}
	}

	//2
	//LoadedTex closest;
	//AllocTex(&closest, lt->sizex, lt->sizey, lt->channels);
	//memset(closest.data, 0, lt->sizex * lt->sizey * lt->channels);
	memset(closest, 0, lt->sizex * lt->sizey * sizeof(int));
	memset(mentions, 0, PNG_MAX_PALETTE_LENGTH * sizeof(int));

	for(int x=0; x<lt->sizex; ++x)
	{
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = -1;
			int neard = -1;
			for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
			{
				int d = iabs((int)(palette[pi].red-lt->data[lt->channels*(x+y*lt->sizex)+0])) * 
					iabs((int)(palette[pi].green-lt->data[lt->channels*(x+y*lt->sizex)+1])) * 
					iabs((int)(palette[pi].blue-lt->data[lt->channels*(x+y*lt->sizex)+2]));

				if(nearpi < 0 || neard > d)
				{
					nearpi = pi;
					neard = d;
				}
			}

			closest[(x+y*lt->sizex)] = nearpi;
			mentions[nearpi]++;
			//lt->data[lt->channels*(x+y*lt->sizex)+0] = palette[pi].red;
			//lt->data[lt->channels*(x+y*lt->sizex)+1] = palette[pi].green;
			//lt->data[lt->channels*(x+y*lt->sizex)+2] = palette[pi].blue;
		}
	}

	int avga[PNG_MAX_PALETTE_LENGTH];
	memset(avga, 0, sizeof(int)*PNG_MAX_PALETTE_LENGTH);

	for(int x=0; x<lt->sizex; ++x)
	{
		for(int y=0; y<lt->sizey; ++y)
		{
			int nearpi = closest[(x+y*lt->sizex)];
			avga[nearpi] += lt->data[(x+y*lt->sizex)*lt->channels] *2000/255;
		}
	}

	for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
		avga[pi] /= imax(1, 2000 * mentions[pi] / 255);

	for(int pi=0; pi<PNG_MAX_PALETTE_LENGTH; ++pi)
		Trans[pi] = imax(0, imin(255, avga[pi]));

	delete [] closest;

	*npal = PNG_MAX_PALETTE_LENGTH;
}

int SavePNG(const char* fullpath, LoadedTex* image)
{
	//FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;

	/* Open the file */
	savepngfp = fopen(fullpath, "wb");
	if (savepngfp == NULL)
		return (ERROR);

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(savepngfp);
		return (ERROR);
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return (ERROR);
	}

	int color_type = PNG_COLOR_TYPE_RGB;

	if(image->channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;
	if(image->channels == 1)
		color_type = PNG_COLOR_TYPE_GRAY;

	if(g_usepalette)
	{
		color_type = PNG_COLOR_TYPE_PALETTE;
	}

	int bit_depth = 8;

	if(g_savebitdepth == 2 ||
		g_savebitdepth == 4 ||
		g_savebitdepth == 8 ||
		g_savebitdepth == 16)
	{
		bit_depth = g_savebitdepth;
	}
	else if(!g_hidetexerr)
	{
		ErrMess("PNG write_bit_depth", "write_bit_depth must be either 2, 4, 8, or 16. Defaulting to 8.");
	}

	png_set_IHDR(png_ptr, info_ptr, image->sizex, image->sizey, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (ERROR);
	}

	if(g_usepalette)
	{
		png_colorp palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));//4
		if (!palette) {
			ErrMess("PNG error", "Error allocating palette");
			OUTOFMEM();
			fclose(savepngfp);
			png_destroy_write_struct(&png_ptr, &info_ptr);//
			return ecfalse;
		}

		png_color_16 Colors[PNG_MAX_PALETTE_LENGTH];
		png_byte Trans[PNG_MAX_PALETTE_LENGTH];
		int npal = 0;

		Palletize(Colors, Trans, &npal, image, palette);
		//BGColor[0].index = 0;
		//TrnsColor[0] = 100; //0 = transparent , 255 = opaque
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);//12
		//png_set_tRNS(png_ptr, info_ptr, Trans, npal, NULL);//Colors);
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	//png_init_io(png_ptr, savepngfp);
	png_set_write_fn(png_ptr, NULL, png_file_write, png_file_flush);

	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image->sizey);

	if(!row_pointers)
	{
		OutOfMem(__FILE__, __LINE__);
		return NULL;
	}

	for (int y=0; y<image->sizey; y++)
		row_pointers[y] = (png_byte*)&image->data[y*image->sizex*image->channels];

	png_text text_ptr[1];

	char srcstr[123];
	sprintf(srcstr, "Rendered using %s Version %d", TITLE, APPVERSION);
#ifdef USESTEAM
	strcat(srcstr, " Authorized Steam Build");
#endif
	text_ptr[0].key = "Source";
	text_ptr[0].text = srcstr;
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	// text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
#endif
	text_ptr[0].lang_key = NULL;

	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<image->sizey; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	* image info living in the structure.  You could "|" many
	* PNG_TRANSFORM flags into the png_transforms integer here.
	*/
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	* as recommended in versions 1.0.5m and earlier of this example; if
	* libpng mallocs info_ptr->palette, libpng will free it).  If you
	* allocated it with malloc() instead of png_malloc(), use free() instead
	* of png_free().
	*/
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	* png_set_something(), such as a hist or trans array, free it here,
	* when you can be sure that libpng is through with it.
	*/
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	* NULL in case your application inadvertently tries to png_free() it
	* again.  When png_free() sees a NULL it returns without action, thus
	* avoiding the double-free security problem.
	*/

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(savepngfp);

	/* That's it */
	return (1);
}

void FlipImage(LoadedTex* image)
{
	int x;
	int y2;
	byte temp[4];
	int stride = image->sizex * image->channels;

	for(int y=0; y<image->sizey/2; y++)
	{
		y2 = image->sizey - y - 1;

		unsigned char *pLine = &(image->data[stride * y]);
		unsigned char *pLine2 = &(image->data[stride * y2]);

		for(x = 0; x < image->sizex * image->channels; x += image->channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(image->channels == 4)
				temp[3] = pLine[x + 3];

			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(image->channels == 4)
				pLine[x + 3] = pLine2[x + 3];

			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(image->channels == 4)
				pLine2[x + 3] = temp[3];
		}
	}
}

ecbool SaveRAW(const char* fullpath, LoadedTex* image)
{
	FILE* fp = fopen(fullpath, "wb");

	fwrite(image->data, image->sizex*image->sizey*image->channels, 1, fp);

	fclose(fp);

	return ectrue;
}

void StreamRaw(FILE* fp, unsigned int* texname, Vec2i fullsz, Vec2i srcpos, Vec2i srcsz, Vec2i destsz)
{
	if(srcpos.x < 0)
		srcpos.x = 0;
	if(srcpos.y < 0)
		srcpos.y = 0;
	if(srcpos.x + srcsz.x > fullsz.x)
		srcsz.x = fullsz.x - srcpos.x;
	if(srcpos.y + srcsz.y > fullsz.y)
		srcsz.y = fullsz.y - srcpos.y;

	int stridex = srcsz.x / destsz.x * 3;
	int stridey = srcsz.y / destsz.y * 3 * fullsz.x;

	LoadedTex newtex;
	AllocTex(&newtex, destsz.x, destsz.y, 3);

	int i = 0;

	for(int y=srcpos.y; srcpos.y < srcpos.y+srcsz.y; y+=stridey)
		for(int x=srcpos.x; srcpos.x < srcpos.x+srcsz.x; x+=stridex)
		{
			fseek(fp, y * 3 * fullsz.x + x * 3, SEEK_SET);
			fread(&newtex.data[i], 3, 1, fp);
			i+=3;
		}

		newtex.destroy();
}

void Extract(LoadedTex* original, LoadedTex* empty, int x1, int y1, int x2, int y2)
{
	AllocTex(empty, x2-x1, y2-y1, original->channels);

	for(int x=x1; x<x2; x++)
		for(int y=y1; y<y2; y++)
		{
			int i1 = (x + y * original->sizex) * original->channels;
			int i2 = ((x-x1) + (y-y1) * empty->sizex) * empty->channels;

			for(int c=0; c<original->channels; c++)
				empty->data[i2] = original->data[i1];
		}
}

void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim)
{
	if(original == NULL || original->data == NULL || original->sizex <= 0 || original->sizey <= 0)
	{

		empty->data = NULL;

		empty->sizex = 0;
		empty->sizey = 0;

		if(original != NULL)
			empty->channels = original->channels;

		return;
	}

	AllocTex(empty, newdim.x, newdim.y, original->channels);

	double scaleW =  (double)newdim.x / (double)original->sizex;
	double scaleH = (double)newdim.y / (double)original->sizey;

	for(int cy = 0; cy < newdim.y; cy++)
	{
		for(int cx = 0; cx < newdim.x; cx++)
		{
			int pixel = cy * (newdim.x * original->channels) + cx*original->channels;
			int nearestMatch =  (int)(cy / scaleH) * original->sizex * original->channels + (int)(cx / scaleW) * original->channels;

			empty->data[pixel    ] =  original->data[nearestMatch    ];
			empty->data[pixel + 1] =  original->data[nearestMatch + 1];
			empty->data[pixel + 2] =  original->data[nearestMatch + 2];

			if(original->channels > 3)
				empty->data[pixel + 3] =  original->data[nearestMatch + 3];
		}
	}
}
