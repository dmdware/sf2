

#include "../platform.h"
#include "../save/savesprite.h"
#include "rendersprite.h"
#include "compilebl.h"
#include "../app/appmain.h"
#include "../gui/gui.h"
#include "../save/compilemap.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../sim/simtile.h"
#include "../math/camera.h"
#include "../render/shadow.h"
#include "../math/vec4f.h"
#include "../render/screenshot.h"
#include "../render/sortb.h"
#include "../app/seviewport.h"
#include "../debug.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "../save/saveedm.h"
#include "../app/undo.h"
#include "../render/rescache.h"
#include "../sys/unicode.h"

char g_renderpath[SFH_MAX_PATH+1];
char g_renderbasename[SFH_MAX_PATH+1];
int g_rendertype = -1;
int g_renderframes;
int g_origwidth;
int g_origheight;
int g_deswidth;
int g_desheight;
Vec2i g_spritecenter;
Vec2i g_clipmin;
Vec2i g_clipmax;
float g_transpkey[3] = {255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f};
ecbool g_doframes = ecfalse;
ecbool g_dosides = ecfalse;
ecbool g_doinclines = ecfalse;
ecbool g_dorots = ecfalse;
//ecbool g_usepalette = ecfalse;
//int g_savebitdepth = 8;

Vec3f g_origlightpos;
Camera g_origcam;
int g_rendside;
int g_rendpitch, g_rendyaw, g_rendroll;
int g_nrendsides = 8;
ecbool g_antialias = ecfalse;
ecbool g_fit2pow = ecfalse;
ecbool g_exportdepth = ecfalse;
ecbool g_exportteam = ecfalse;
ecbool g_exportelev = ecfalse;
ecbool g_exportdiff = ectrue;
//ecbool g_hidetexerr = ecfalse;
int g_nslices = 1;
int g_slicex = 0;
int g_slicey = 0;

ecbool g_warned = ecfalse;

unsigned int g_rendertex[4];
unsigned int g_renderdepthtex[4];
unsigned int g_renderrb[4];
unsigned int g_renderfb[4];
ecbool g_renderbs = ecfalse;

int g_rendspl = -1;	//output rendered sprite list index

std::vector<SpListToRender> g_spltorend;
int g_lastRSp = -1;

std::string stage;
std::string incline;
char frame[32] = "";
char side[32] = "";

void MakeFBO(int sample, int rendstage)
{
	if(g_renderbs)
		return;

	if(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME)
	{
		g_renderbs = ectrue;
#if 0   //OpenGL 3.0 way
#if 1
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		CHECKGLERROR();
#if 0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		CHECKGLERROR();
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		CHECKGLERROR();

		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		CHECKGLERROR();
#else
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//-------------------------
		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		//Attach 2D texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		//-------------------------
		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//-------------------------
#endif
#else   //OpenGL 1.4 way


        glGenTextures(1, &g_rendertex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_rendertex[sample]);
        glTexImage2D(GL_TEXTURE_2D, 0, 
			(rendstage == RENDSTAGE_COLOR || rendstage == RENDSTAGE_DUMMY)? GL_RGBA8 : GL_RGBA8, 
			g_deswidth, g_desheight, 0, (rendstage == RENDSTAGE_COLOR) ? GL_RGBA : GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Log("gge1 "<<glGetError()<<std::endl;

        glGenTextures(1, &g_renderdepthtex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_renderdepthtex[sample]);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Log("gge2 "<<glGetError()<<std::endl;
        //glDrawBuffer(GL_NONE); // No color buffer is drawn
        //glReadBuffer(GL_NONE);

        glGenFramebuffers(1, &g_renderfb[sample]);
        glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[sample]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_rendertex[sample], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderdepthtex[sample], 0);

        //Log("gge3 "<<glGetError()<<std::endl;

        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT_EXT};
        glDrawBuffers(1, DrawBuffers);

        //Log("gge4 "<<glGetError()<<std::endl;

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
			char m[123];
			sprintf(m, "Couldn't create framebuffer for render.\r\nSize=%d,%d", 
				g_deswidth, g_desheight);
            ErrMess("Error", m);
            return;
        }
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef DEBUG
//		Log(__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
//		Log(__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
#endif
	}
}

void DelFBO(int sample)
{
#if 0
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	CHECKGLERROR();
	glDeleteFramebuffers(1, &g_renderfb);
	CHECKGLERROR();
	glDeleteRenderbuffers(1, &g_renderrb);
	CHECKGLERROR();
	glDeleteTextures(1, &g_rendertex);
	CHECKGLERROR();
#else
	//Delete resources
	glDeleteTextures(1, &g_rendertex[sample]);
	glDeleteTextures(1, &g_renderdepthtex[sample]);
	//glDeleteRenderbuffers(1, &g_renderrb);
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &g_renderfb[sample]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
#endif

	g_renderbs = ecfalse;
}

ecbool CallResize(int w, int h)
{
	//return ecfalse;	//don't resize, has bad consequences when resizing with ANTIALIAS_UPSCALE=4

	int maxsz[] = {0,0};
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxsz);

	if((w >= maxsz[0]*0.7f || h >= maxsz[0]*0.7f) && !g_warned)
	{
		g_warned = ectrue;
		char msg[1024];
		sprintf(msg, "The required texture size of %dx%d exceeds or approaches your system's supported maximum of %d. You might not be able to finish the render. Reduce 1_tile_pixel_width in config.ini.", w, h, maxsz[0]);
		WarnMess("Warning", msg);
	}

	Log("resz %d,%d", w, h);

#if 1
	//w=2048;
	//h=2048;
#if 0
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)w;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)h;
#else
    //SDL_SetWindowSize(g_window, w, h);
	Resize(w, h);
#endif

	int startx = 0;
	int starty = 0;

#if 0
	if(g_fs)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//startx = CW_USEDEFAULT;
		//starty = CW_USEDEFAULT
		startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selres.width/2;
		starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selres.height/2;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
#endif

#if 0
	//Log("rf", g_renderframe<<" desired: "<<w<<","<<h<<", g_wh: "<<g_width<<","<<g_height<<" wr.rl: "<<(WindowRect.right-WindowRect.left)<<","<<(WindowRect.bottom-WindowRect.top)<<std::endl;
	
#endif

#if 0
	if(WindowRect.right-WindowRect.left == g_width
		&& WindowRect.bottom-WindowRect.top == g_height)
#endif
	{
		if(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME)
		{
			g_width = w;
			g_height = h;
		}

		return ecfalse;
	}

#if 0
	SetWindowPos(g_hWnd,0,0,0,WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif

	//if(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME)
	{
		g_width = w;
		g_height = h;
	}

	return ectrue;
#else

	return ectrue;
#endif
}

ecbool FitFocus(Vec2i vmin, Vec2i vmax)
{
#if 0
	int xextent = max(abs(vmin.x-g_width/2), abs(vmax.x-g_width/2));
	int yextent = max(abs(vmin.y-g_height/2), abs(vmax.y-g_height/2));
#else
	int xextent = imax(iabs(vmin.x-g_width/2), iabs(vmax.x-g_width/2));
	int yextent = imax(iabs(vmin.y-g_height/2), iabs(vmax.y-g_height/2));
#endif

	//int width2 = Max2Pow(xextent*2);
	//int height2 = Max2Pow(yextent*2);

	int width = (xextent*2);
	int height = (yextent*2);

	//char m[123];
	//sprintf(m, "des w,h=%d,%d \r\n vm=%d,%d,%d,%d whg=%d,%d", width, height,
	//	vmin.x, vmin.y, vmax.x, vmax.y, g_width, g_height);
	//InfoMess("Info", m);

	//Log("ftf xye %d,%d", xextent, yextent);

	//g_deswidth = width;
	//g_desheight = height;

	//g_spritecenter.x = xextent;
	//g_spritecenter.y = yextent;

	// size must be multiple of 32 or else glReadPixels will crash !!!!!
	Vec2i compatsz;
	//compatsz.x = Max2Pow32(g_width);
	//compatsz.y = Max2Pow32(g_height);
	if(g_fit2pow)
	{
		compatsz.x = Max2Pow32(width);
		compatsz.y = Max2Pow32(height);
	}
	else
	{
		compatsz.x = (width);
		compatsz.y = (height);
	}

	g_deswidth = compatsz.x;
	g_desheight = compatsz.y;

	//Log("ftf des %d,%d", g_deswidth, g_desheight);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" o des wh "<<g_deswidth<<","<<g_desheight<<"  xyextn:"<<xextent<<","<<yextent<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<") gwh:"<<g_width<<","<<g_height<<" "<<std::endl;
#endif

	//Sleep(10);

	return CallResize(compatsz.x, compatsz.y);

#if 0
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_projtype = PROJ_ORTHO;
	g_cam.position(0, 0, 1000.0f, 0, 0, 0, 0, 1, 0);
	g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
	g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);

	g_zoom = 1;

	Vec3f topleft(-g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomleft(-g_tilesize/2, 0, g_tilesize/2);
	Vec3f topright(g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomright(g_tilesize/2, 0, g_tilesize/2);

	int width;
	int height;

	if(g_appmode == APPMODE_RENDERING)
	{
		width = g_width;
		height = g_height;
	}
	//if(g_appmode == APPMODE_EDITOR)
	else
	{
		View* edview = g_gui.get("editor");
		Widget* viewportsframe = edview->get("viewports frame", WIDGET_FRAME);
		Widget* toprightviewport = viewportsframe->get("top right viewport", WIDGET_VIEWPORT);
		width = toprightviewport->pos[2] - toprightviewport->pos[0];
		height = toprightviewport->pos[3] - toprightviewport->pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	ecbool persp = ecfalse;

	if(g_appmode == APPMODE_EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	//VpWrap* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_cam.view;
	Vec3f viewvec = g_cam.view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->offset;
	Vec3f posvec = g_cam.pos;
	//Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.view + t->offset;
		//viewvec = posvec + Normalize(g_cam.view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->offset;
    //Vec3f upvec = t->up;
    Vec3f upvec = g_cam.up;
	//Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m);
	mvpmat.postmult(viewmat);

	persp = ecfalse;

	Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	float minx = min(topleft4.x, min(topright4.x, min(bottomleft4.x, bottomright4.x)));
	float maxx = max(topleft4.x, max(topright4.x, max(bottomleft4.x, bottomright4.x)));
	//float miny = min(topleft4.y, min(topright4.y, min(bottomleft4.y, bottomright4.y)));
	//float maxy = max(topleft4.y, max(topright4.y, max(bottomleft4.y, bottomright4.y)));

	float xrange = (float)maxx - (float)minx;

	if(xrange <= 0.0f)
		xrange = g_1tilewidth;

	float zoomscale = (float)g_1tilewidth / xrange;

	g_zoom *= zoomscale;

	//Log("zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<std::endl;

	g_appmode = APPMODE_PRERENDADJFRAME;
#endif
}

void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height)
{
	//int width = g_width;
	//int height = g_height;

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	ecbool persp = ecfalse;

	if(g_appmode == APPMODE_EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else
	{
		//float oldz = g_zoom;
		//g_zoom *= 1.01f;
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
		//g_zoom = oldz;
	}

#ifdef DEBUG
	{

		Log("rf"<<g_renderframe<<" params:"<<aspect<<","<<width<<","<<height<<","<<g_zoom<<std::endl;
		Log("rf"<<g_renderframe<<" pmat0:"<<projection.m[0]<<","<<projection.m[1]<<","<<projection.m[2]<<","<<projection.m[3]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat1:"<<projection.m[4]<<","<<projection.m[5]<<","<<projection.m[6]<<","<<projection.m[7]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat2:"<<projection.m[8]<<","<<projection.m[9]<<","<<projection.m[10]<<","<<projection.m[11]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat3:"<<projection.m[12]<<","<<projection.m[13]<<","<<projection.m[14]<<","<<projection.m[15]<<std::endl;
	}
#endif

	//VpWrap* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_cam.view;
	////Vec3f viewvec = g_cam.view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;

	/////Vec3f dir = Normalize( g_cam.view - g_cam.pos );
	/////Vec3f posvec = g_cam.view - dir * 100000.0f / g_zoom;

	//Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.view + t->offset;
		//viewvec = posvec + Normalize(g_cam.view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->offset;
    //Vec3f upvec = t->up;
    //////Vec3f upvec = g_cam.up;
    //Vec3f upvec = g_cam.up2();
	//Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	////Vec3f focusvec = viewvec;

    /////Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	/////Matrix mvpmat;
	/////mvpmat.set(projection.m);
	/////mvpmat.postmult(viewmat);


//	float aspect = fabsf((float)g_width / (float)g_height);
//	Matrix projection;

	VpWrap* v = &g_viewport[3];
	VpType* t = &g_vptype[v->type];

//	ecbool persp = ecfalse;

#if 0
	if(g_projtype == PROJ_PERSP && v->type == VIEWPORT_ANGLE45O)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else if(g_projtype == PROJ_ORTHO || v->type != VIEWPORT_ANGLE45O)
#endif
	{
	//	projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	Vec3f offset = Vec3f(0,0,0);
	//Vec3f viewvec = g_focus;	//g_cam.view;
	//Vec3f viewvec = g_cam.view;
	Vec3f focusvec = v->focus() + offset;
	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos() + offset;

	//if(v->type != VIEWPORT_ANGLE45O)
	//	posvec = g_cam.view + t->offset;

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up;
	Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
#if 0
	Log("vpt %d cp %f,%f,%f", (int)v->type,
		g_cam.up.x, g_cam.up.y, g_cam.up.z);
	Log("pfu\r\n %f,%f,%f\r\n %f,%f,%f\r\n %f,%f,%f",
		posvec.x, posvec.y, posvec.z,
		focusvec.x, focusvec.y, focusvec.z,
		upvec.x, upvec.y, upvec.z);
#endif
	Matrix modelview;
	Matrix modelmat;
	float translation[] = {0, 0, 0};
	modelview.translation(translation);
	modelmat.translation(translation);
	if(persp)
		modelview.postmult(viewmat);
	else
		modelview.postmult2(viewmat);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	Matrix mvpmat;
	mvpmat.set(projection.m);
	if(!persp)
		mvpmat.postmult2(viewmat);
	else
		mvpmat.postmult(viewmat);

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" vmat0:"<<viewmat.m[0]<<","<<viewmat.m[1]<<","<<viewmat.m[2]<<","<<viewmat.m[3]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat1:"<<viewmat.m[4]<<","<<viewmat.m[5]<<","<<viewmat.m[6]<<","<<viewmat.m[7]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat2:"<<viewmat.m[8]<<","<<viewmat.m[9]<<","<<viewmat.m[10]<<","<<viewmat.m[11]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat3:"<<viewmat.m[12]<<","<<viewmat.m[13]<<","<<viewmat.m[14]<<","<<viewmat.m[15]<<std::endl;
	}
			Log("projection\r\n %f,%f,%f,%f,,\r\n %f,%f,%f,%f,,\r\n ,,%f,%f,%f,%f\r\n ,,%f,%f,%f,%f",
				projection.m[0],
				projection.m[1],
				projection.m[2],
				projection.m[3],
				projection.m[4],
				projection.m[5],
				projection.m[6],
				projection.m[7],
				projection.m[8],
				projection.m[9],
				projection.m[10],
				projection.m[11],
				projection.m[12],
				projection.m[13],
				projection.m[14],
				projection.m[15]);
			Log("viewmat\r\n %f,%f,%f,%f,,\r\n %f,%f,%f,%f,,\r\n ,,%f,%f,%f,%f\r\n ,,%f,%f,%f,%f",
				viewmat.m[0],
				viewmat.m[1],
				viewmat.m[2],
				viewmat.m[3],
				viewmat.m[4],
				viewmat.m[5],
				viewmat.m[6],
				viewmat.m[7],
				viewmat.m[8],
				viewmat.m[9],
				viewmat.m[10],
				viewmat.m[11],
				viewmat.m[12],
				viewmat.m[13],
				viewmat.m[14],
				viewmat.m[15]);

#endif
	persp = ecfalse;

	//Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	//Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	//Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	//Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	ecbool showsky = ecfalse;

	ecbool setmm[] = {ecfalse, ecfalse, ecfalse, ecfalse};

	Heightmap hm;

	if((g_appmode == APPMODE_PRERENDADJFRAME ||
		g_appmode == APPMODE_RENDERING) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0];
		heights[1] = g_cornerinc[g_currincline][1];
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2];
		heights[3] = g_cornerinc[g_currincline][3];

		//Heightmap hm;
		hm.alloc(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();
	}

#if 1
	for(std::list<Brush>::iterator b=g_edmap.brush.begin(); b!=g_edmap.brush.end(); b++)
	{
		Texture* t = &g_texture[b->texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->nsides; i++)
		//for(int i=0; i<1; i++)
		{
			BrushSide* side = &b->sides[i];
			VertexArray* va = &side->drawva;

			for(int j=0; j<va->numverts; j++)
			{
				Vec3f v = va->vertices[j];

				if((g_appmode == APPMODE_PRERENDADJFRAME ||
					g_appmode == APPMODE_RENDERING) &&
					g_doinclines)
				{
					//TransformedPos[i].y += Bilerp(&hm,
					//	g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
					//	g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
					v.y += hm.accheight2(
						g_tilesize/2.0f + v.x,
						g_tilesize/2.0f + v.z);
				}

				Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
				Log("rf"<<g_renderframe<<" v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
#endif

#if 0
				if(floorf(v4.x) < vmin->x)
					vmin->x = floorf(v4.x);
				if(floorf(v4.y) < vmin->y)
					vmin->y = floorf(v4.y);
				if(ceilf(v4.x) > vmax->x)
					vmax->x = ceilf(v4.x);
				if(ceilf(v4.y) > vmax->y)
					vmax->y = ceilf(v4.y);
#endif

				if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
				{
					vmin->x = floor(v4.x+0.5f);
					setmm[0] = ectrue;
				}
				if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
				{
					vmin->y = floor(v4.y+0.5f);
					setmm[1] = ectrue;
				}
				if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
				{
					vmax->x = floor(v4.x+0.5f);
					setmm[2] = ectrue;
				}
				if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
				{
					vmax->y = floor(v4.y+0.5f);
					setmm[3] = ectrue;
				}
			}
		}

		//break;
	}
#endif

#ifdef DEBUG
	//for(int my=0; my<4; my++)
	{
		Log("rf"<<g_renderframe<<" mat0:"<<mvpmat.m[0]<<","<<mvpmat.m[1]<<","<<mvpmat.m[2]<<","<<mvpmat.m[3]<<std::endl;
		Log("rf"<<g_renderframe<<" mat1:"<<mvpmat.m[4]<<","<<mvpmat.m[5]<<","<<mvpmat.m[6]<<","<<mvpmat.m[7]<<std::endl;
		Log("rf"<<g_renderframe<<" mat2:"<<mvpmat.m[8]<<","<<mvpmat.m[9]<<","<<mvpmat.m[10]<<","<<mvpmat.m[11]<<std::endl;
		Log("rf"<<g_renderframe<<" mat3:"<<mvpmat.m[12]<<","<<mvpmat.m[13]<<","<<mvpmat.m[14]<<","<<mvpmat.m[15]<<std::endl;
	}
#endif

#if 1
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		//Model2* m = &g_model2[h->modeli];
		Model2* m = &h->model;
		Model2* origm = &g_model2[h->modeli];

		//m->usetex();
		//DrawVA(&h->frames[ g_renderframe % m->ms3d.totalFrames ], h->translation);

#if 0	//TODO
		VertexArray* va = &h->frames[ g_renderframe % m->ms3d.totalFrames ];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i] + h->translation;
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
//#ifdef 1
			Log("rf"<<g_renderframe<<" mdl v:"<<v.x<<","<<v.y<<","<<v.z<<std::endl;
			Log("rf"<<g_renderframe<<" mdl v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
#endif

#if 0
			if(floorf(v4.x) < vmin->x)
				vmin->x = floorf(v4.x);
			if(floorf(v4.y) < vmin->y)
				vmin->y = floorf(v4.y);
			if(ceilf(v4.x) > vmax->x)
				vmax->x = ceilf(v4.x);
			if(ceilf(v4.y) > vmax->y)
				vmax->y = ceilf(v4.y);
#endif

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = ectrue;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = ectrue;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = ectrue;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = ectrue;
			}
		}
#else
		//perform frame transformation on-the-fly
		std::vector<Matrix> BoneTransforms;

		if(m->pScene->mNumAnimations > 0)
		{
			float TicksPerSecond = (float)(m->pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			float frames = TicksPerSecond * (float)m->pScene->mAnimations[0]->mDuration;
			//int frame = g_renderframe % (int)frames;
			int frame = g_renderframe;
			float percentage = (float)frame / frames;
			float RunningTime = percentage * (float)m->pScene->mAnimations[0]->mDuration;

			m->BoneTransform(RunningTime, BoneTransforms);
		}
		else
		{
			BoneTransforms.resize( m->BoneInfo.size() );

			for(int i=0; i<m->BoneInfo.size(); i++)
			{
				BoneTransforms[i].InitIdentity();
			}
		}

		std::vector<Vec3f> TransformedPos;
		std::vector<Vec3f> TransformedNorm;
		TransformedPos.resize(m->Positions.size());
		TransformedNorm.resize(m->Normals.size());

		for(uint i=0; i<m->Positions.size(); i++)
		{
			Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
			Transform.InitIdentity();

			//ecbool influenced = ecfalse;

			for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			{
				if(m->Bones[i].IDs[bi] < 0)
					continue;

				//if(m->Bones[i].IDs[bi] >= m->BoneInfo.size())
				//	continue;

				//if(m->Bones[i].IDs[bi] >= BoneTransforms.size())
				//	continue;

				if(m->Bones[i].Weights[bi] == 0.0f)
					continue;

				//influenced = ectrue;

				//Transform += BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

				if(bi == 0)
					Transform = BoneTransforms[ m->Bones[i].IDs[bi] ] * m->Bones[i].Weights[bi];
				else
					Transform += BoneTransforms[ m->Bones[i].IDs[bi] ] * m->Bones[i].Weights[bi];
			}

			//if(!influenced)
			//	Transform.InitIdentity();

			Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
			Transformed.transform(Transform);
			Transformed.transform(h->rotationmat);
			//Transformed = Transformed * h->scale;
			TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * h->scale;
			TransformedNorm[i] = origm->Normals[i];
			TransformedNorm[i].transform(Transform);
			TransformedNorm[i].transform(h->rotationmat);
		}

		//If we're including inclines, adjust vertex heights
		if(//(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME) &&
			g_doinclines)
		{
			float heights[4];
			//As said about "g_cornerinc":
			//corners in order of digits displayed on name, not in clock-wise corner order
			//So we have to reverse using (3-x).
			//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
			//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
			//Edit: or no...
			heights[0] = g_cornerinc[g_currincline][0];
			heights[1] = g_cornerinc[g_currincline][1];
			//important, notice "g_cornerinc" uses clock-wise ordering of corners
			heights[2] = g_cornerinc[g_currincline][2];
			heights[3] = g_cornerinc[g_currincline][3];

			Heightmap hm;
			hm.alloc(1, 1);
			//x,z, y
			//going round the corners clockwise
			hm.setheight(0, 0, heights[0]);
			hm.setheight(1, 0, heights[1]);
			hm.setheight(1, 1, heights[2]);
			hm.setheight(0, 1, heights[3]);
			hm.remesh();

			for(uint i=0; i<TransformedPos.size(); i++)
			{
				//TransformedPos[i].y += Bilerp(&hm,
				//	g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
				//	g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
				TransformedPos[i].y += hm.accheight2(
					g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
					g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
			}

			//Regenerate normals:
			//Not possible, based on vertices alone, because we would also need to blend shared faces,
			//so leave this inaccuracy for now. TODO
		}

		
			//for(uint i=0; i<TransformedPos.size(); i++)
			//	TransformedPos[i] = TransformedPos[i]/TransformedPos[i].z;
#if 0
			for(uint i=0; i<TransformedPos.size(); i++)
		Log("tp %f,%f,%f,%f", 
			TransformedPos[i].x,
			 TransformedPos[i].y,
			  TransformedPos[i].z,
			  1.0f);

			Log("mvpmat\r\n %f,%f,%f,%f,,\r\n %f,%f,%f,%f,,\r\n ,,%f,%f,%f,%f\r\n ,,%f,%f,%f,%f",
				mvpmat.m[0],
				mvpmat.m[1],
				mvpmat.m[2],
				mvpmat.m[3],
				mvpmat.m[4],
				mvpmat.m[5],
				mvpmat.m[6],
				mvpmat.m[7],
				mvpmat.m[8],
				mvpmat.m[9],
				mvpmat.m[10],
				mvpmat.m[11],
				mvpmat.m[12],
				mvpmat.m[13],
				mvpmat.m[14],
				mvpmat.m[15]);
#endif

		for (uint i = 0 ; i < m->Entries.size() ; i++)
		{
			const unsigned int numindices = m->Entries[i].NumIndices;
			const unsigned int basevertex = m->Entries[i].BaseVertex;
			const unsigned int baseindex = m->Entries[i].BaseIndex;
			const unsigned int numunique = m->Entries[i].NumUniqueVerts;

			for(unsigned int indi=baseindex; indi<baseindex+numindices; indi++)
			{
			#if 0
				unsigned int ind0 = indi + 0;
				unsigned int ind1 = indi + 1;
				unsigned int ind2 = indi + 2;
				Vec3f v0 = TransformedPos[basevertex + ind0];
				Vec3f v1 = TransformedPos[basevertex + ind1];
				Vec3f v2 = TransformedPos[basevertex + ind2];
			#endif
				Vec3f v = TransformedPos[basevertex + m->Indices[indi]] + h->translation;
				Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#if 0
		Log("tp %f,%f,%f,%f v4 %f,%f,%f,%f", 
			TransformedPos[basevertex + m->Indices[indi]].x,
			 TransformedPos[basevertex + m->Indices[indi]].y,
			  TransformedPos[basevertex + m->Indices[indi]].z,
			  1.0f,
			  v4.x,v4.y,v4.z,v4.w);
#endif

				if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
				{
					vmin->x = floor(v4.x+0.5f);
					setmm[0] = ectrue;
				}
				if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
				{
					vmin->y = floor(v4.y+0.5f);
					setmm[1] = ectrue;
				}
				if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
				{
					vmax->x = floor(v4.x+0.5f);
					setmm[2] = ectrue;
				}
				if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
				{
					vmax->y = floor(v4.y+0.5f);
					setmm[3] = ectrue;
				}
			}
		}
#if 0
		for(int i=0; i<h->model.Positions.size(); i++)
		{
			Vec3f v = h->model.Positions[i] + h->translation;
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = ectrue;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = ectrue;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = ectrue;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = ectrue;
			}
		}
#endif
#endif
	}


	if(g_tiletexs[TEX_DIFF] != 0)
	{
		VertexArray* va = &g_tileva[g_currincline];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i];
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = ectrue;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = ectrue;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = ectrue;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = ectrue;
			}
		}
	}
#ifdef DEBUG
	Log("rf"<<g_renderframe<<" setmm:"<<setmm[0]<<","<<setmm[1]<<","<<setmm[2]<<","<<setmm[3]<<std::endl;
	Log("rfvminmax "<<vmin->x<<","<<vmin->y<<"->"<<vmax->x<<","<<vmax->y<<std::endl;
#endif
#endif

#if 0
	//not needed anymore, we upscale the render resolution,
	//then downscale it to intended size.
	if(g_antialias)
	{
		//make room for "blur pixels" around the edges
		vmax->x++;
		vmax->y++;
	}
#endif
}

ecbool PrepRendTl(SpListToRender* torend)
{
	if(!g_rescachefp || !g_rescacheread)
	{
		//AddTile("textures/marsdirt/MarsViking1Lander-BigJoeRock-19780211.jpg");
		AddTile(torend->relative.c_str());
	}

	PrepareRender(RENDER_TERRTILE, torend->outspl,
		torend->doframes, 
		torend->doinclines, 
		torend->dosides, 
		torend->dorots, 
		torend->nframes, 
		torend->nsides, 
		torend->expdepth,
		torend->expteam, 
		torend->expelev,
		torend->expdiff,
		torend->nslices);

	return ectrue;
}

ecbool PrepRendPj(SpListToRender* torend)
{
	//CorrectSlashes(filepath);
	//FreeEdMap(&g_edmap);

	if(!g_rescachefp || !g_rescacheread)
	{
		if(!LoadEdMap(torend->relative.c_str(), &g_edmap))
		{
			//InfoMess("L","L");
			//strcpy(g_lastsave, filepath);
			return ecfalse;
		}

		VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
		//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
		SortEdB(&g_edmap, g_cam.view, g_cam.pos);
		ClearUndo();
	}

	PrepareRender(RENDER_UNSPEC, torend->outspl,
		torend->doframes, 
		torend->doinclines, 
		torend->dosides, 
		torend->dorots, 
		torend->nframes, 
		torend->nsides, 
		torend->expdepth,
		torend->expteam, 
		torend->expelev,
		torend->expdiff,
		torend->nslices);

	return ectrue;
}

ecbool PrepRendMl(SpListToRender* torend)
{
	//CorrectSlashes(filepath);
	//FreeEdMap(&g_edmap);

	if(!g_rescachefp || !g_rescacheread)
	{
		FreeEdMap(&g_edmap);

		int modelid = LoadModel2(torend->relative.c_str(), Vec3f(1,1,1), Vec3f(0,0,0), ectrue);

		if(modelid < 0)
		{
			char msg[SFH_MAX_PATH+1];
			sprintf(msg, "Couldn't load model %s", torend->relative.c_str());

			ErrMess("Error", msg);

			return ecfalse;
		}

		ModelHolder mh(modelid, Vec3f(0,0,0));
		g_modelholder.push_back(mh);

		//g_model2[modelid].Render(0, Vec3f(0,0,0), modelid, Vec3f(1,1,1), Matrix());

		ClearUndo();
	}

	PrepareRender(RENDER_MODEL, torend->outspl,
		torend->doframes, 
		torend->doinclines, 
		torend->dosides, 
		torend->dorots, 
		torend->nframes, 
		torend->nsides, 
		torend->expdepth,
		torend->expteam, 
		torend->expelev,
		torend->expdiff,
		torend->nslices);

	return ectrue;
}

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
				   int nslices)
{
	for(std::vector<SpListToRender>::iterator sit=g_spltorend.begin();
		sit!=g_spltorend.end();
		++sit)
	{
		if(strcmp(sit->relative.c_str(), relative) == 0)
		{
			sit->outspl.push_back((int*)outspl);
			return;
		}
	}

	SpListToRender torend;

	torend.relative = relative;
	torend.outspl.push_back((int*)outspl);
	torend.doframes = doframes;
	torend.doinclines = doinclines;
	torend.dosides = dosides;
	torend.dorots = dorots;
	torend.nframes = nframes;
	torend.nsides = nsides;
	torend.expdepth = expdepth;
	torend.expteam = expteam;
	torend.expelev = expelev;
	torend.expdiff = expdiff;
	torend.nslices = nslices;

	torend.expdiff = ectrue;

	if(rendtype == RENDER_TERRTILE)
		torend.prepfunc = PrepRendTl;
	else if(rendtype == RENDER_MODEL)
		torend.prepfunc = PrepRendMl;
	else
		torend.prepfunc = PrepRendPj;

	g_spltorend.push_back(torend);	// g_spltorend
}

ecbool Rend1()
{
//	unsigned int* utf321 = ToUTF32("Кешивание ");
//	unsigned int* utf322 = ToUTF32("Грузит ");

	if(g_lastRSp+1 < g_spltorend.size())
	{
		std::string stat = ((g_rescachefp&&!g_rescacheread)?std::string("Caching "):std::string("Reading "))+
		g_spltorend[g_lastRSp+1].relative;

		SetStatus( stat .c_str() );
	}

//	delete [] utf321;
//	delete [] utf322;

	//char m[123];
	//sprintf(m, "l%d", (int)g_spriteload.size()-g_lastLSp);
	//InfoMess("asd",m);

	CHECKGLERROR();

	//g_gui.hideall();
	//g_gui.show("loading");

	if(g_lastRSp >= 0)
	{
		SpListToRender* s = &g_spltorend[g_lastRSp];
		//if(!LoadSprite(s->relative.c_str(), s->spindex, s->loadteam, s->loaddepth))
		if(!s->prepfunc(s))
		{
			char m[128];
			sprintf(m, "Failed to render/load sprite %s", s->relative.c_str());
			////ErrMess("Error", m);
		}
	}

	g_lastRSp ++;

	if(g_lastRSp >= g_spltorend.size())
	{
		return ecfalse;	// Done loading all
	}

	return ectrue;	// Not finished loading
}

void RenderQueue()
{
	//AddTile("textures/marsdirt/MarsViking1Lander-BigJoeRock-19780211.jpg");
	//PrepareRender(RENDER_TERRTILE, (int*)&g_ground,
	//	ecfalse, ectrue, ecfalse, ecfalse, 1, 1, ectrue, ecfalse, ecfalse);

	//g_appmode = APPMODE_LOADING;
	//Widget *gui = (Widget*)&g_gui;
	//gui->hideall();
	//gui->show("loading");
}

void FreeTile()
{
	if(!g_tiletexs[TEX_DIFF])
		return;
	FreeTexture(g_tiletexs[TEX_DIFF]);
	FreeTexture(g_tiletexs[TEX_SPEC]);
	FreeTexture(g_tiletexs[TEX_NORM]);
	FreeTexture(g_tiletexs[TEX_TEAM]);
	g_tiletexs[TEX_DIFF] = 0;
	g_tiletexs[TEX_SPEC] = 0;
	g_tiletexs[TEX_NORM] = 0;
	g_tiletexs[TEX_TEAM] = 0;
}

void AddTile(const char* relativepath)
{
	
	unsigned int diffuseindex;
	if(!CreateTex(diffuseindex, relativepath, ecfalse, ecfalse) || diffuseindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath);

		ErrMess("Error", msg);
	}

	unsigned int texname = g_texture[diffuseindex].texname;
	char specpath[SFH_MAX_PATH+1];
	strcpy(specpath, relativepath);
	StripExt(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	if(!CreateTex(specindex, specpath, ecfalse, ecfalse) || specindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrMess("Error", msg);
	}

	char normpath[SFH_MAX_PATH+1];
	strcpy(normpath, relativepath);
	StripExt(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	if(!CreateTex(normindex, normpath, ecfalse, ecfalse) || normindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrMess("Error", msg);
	}

	char ownpath[SFH_MAX_PATH+1];
	strcpy(ownpath, relativepath);
	StripExt(ownpath);
	strcat(ownpath, ".team.png");

	unsigned int ownindex;
	//CreateTex(ownindex, ownpath, ecfalse, ecfalse);
	if(!CreateTex(ownindex, ownpath, ecfalse, ecfalse) || ownindex == 0)
	{
		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", ownpath);

		ErrMess("Error", msg);
	}

	g_tiletexs[TEX_DIFF] = diffuseindex;
	g_tiletexs[TEX_SPEC] = specindex;
	g_tiletexs[TEX_NORM] = normindex;
	g_tiletexs[TEX_TEAM] = ownindex;
}

void FailLoadRend()
{
	static ecbool once = ecfalse;

	if(!once)
	{
		char fullpath[SFH_MAX_PATH+1];
		NameSp(fullpath, RENDSTAGE_COLOR);
		char m[123];
		sprintf(m, "Failed to load resource %s", fullpath);
		ErrMess("Error", m);
		once=ectrue;
	}

	EndRender();
}

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
				   int nslices)
{
	g_rendspl = NewSpriteList();

	if(g_rendspl < 0)
		return;

	for(std::list<int*>::iterator oit=outspl.begin();
		oit!=outspl.end();
		++oit)
		(*(*oit)) = g_rendspl;

	Widget *gui = (Widget*)&g_gui;
	g_appmode = APPMODE_PRERENDADJFRAME;
	g_rendertype = rendtype;
	//strcpy(g_renderbasename, fullpath);
	strcpy(g_renderbasename, "");
	///gui->hideall();
	///gui->show("render");
	//glClearColor(255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f, 1.0f);
	g_renderframe = 0;
	g_origlightpos = g_lightPos;
	g_origcam = g_cam;
	g_rendside = 0;
	g_rendpitch = 0;
	g_rendyaw = 0;
	g_rendroll = 0;
	g_currincline = 0;
	//g_renderframes = GetNumFrames();
	g_renderframes = nframes;
	g_nrendsides = nsides;
	//GetDoFrames();
	g_doframes = (doframes);
	//GetDoInclines();
	g_doinclines = (doinclines);
	//GetDoSides();
	g_dosides = (dosides);
	//GetDoRotations();
	g_dorots = (dorots);
	g_origwidth = g_width;
	g_origheight = g_height;
	g_warned = ecfalse;
	g_exportdepth = expdepth;
	g_exportteam = expteam;
	g_exportelev = expelev;
	g_exportdiff = expdiff;

	SpList* spl = &g_splist[g_rendspl];
	spl->nframes = nframes;
	spl->nsides = nsides;
	spl->on = ectrue;
	spl->rotations = dorots;
	spl->sides = dosides;
	spl->inclines = doinclines;
	spl->frames = doframes;
	spl->nslices = nslices;
	int ci = SpriteRef(spl, nframes-1, INCLINES-1, nsides-1,nsides-1,nsides-1, spl->nslices-1, spl->nslices-1)+1;
	//int ci = ((doinclines)?INCLINES:1) * 
	//	((doframes)?nframes:1) * 
	//	((dorots||dosides)?nsides:1) * 
	//	((dorots)?nsides:1) * 
	//	((dorots)?nsides:1);
	spl->sprites = new unsigned int [ ci ];
	spl->nsp = ci;
	spl->fullpath = g_spltorend[g_lastRSp].relative;
	memset(spl->sprites,
		0, sizeof(unsigned int)*ci);

	if(!g_rescachefp || !g_rescacheread)
	{
		ResetView(ectrue);
		AdjustFrame();
		RotateView();
		AdjustFrame();
	}
	else if(feof(g_rescachefp))
	{
		FailLoadRend();
		return;
	}

/*
	ResetView(ectrue);
	RotateView();
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
	/////AdjustFrame(ecfalse);
	//AllScreenMinMax needs to be called again because the pixels center of rendering depends on the window width and height, influencing the clip min/max
	AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);
	//FitFocus(g_clipmin, g_clipmax);
	//AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);
	//Because we're always centered on origin, we can do this:
	g_spritecenter.x = g_width/2 - g_clipmin.x;
	g_spritecenter.y = g_height/2 - g_clipmin.y;
	glEnable(GL_BLEND);
	glViewport(0, 0, g_width, g_height);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	*/

	//UpdRend();
}

void AdjustFrame(ecbool resetview)
{
	g_appmode = APPMODE_PRERENDADJFRAME;
	//CallResize(MAX_TEXTURE, MAX_TEXTURE);
	if(resetview)
		ResetView(ectrue);
	Vec2i vmin(g_width/2, g_height/2);
	Vec2i vmax(g_width/2, g_height/2);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" adjf1 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")");
#endif

	RotateView();
	AllScreenMinMax(&vmin, &vmax, g_width, g_height);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" asmm adjf2 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")");
#endif

	//if(vmin.x < 0)
	//	ErrMess("asda","asdsad");
	//if(vmin.x < 0)
	//	exit(0);

	//Log("fit %d,%d->%d,%d", vmin.x, vmin.y, vmax.x, vmax.y);

	if(!FitFocus(vmin, vmax))
		g_appmode = APPMODE_RENDERING;
	g_appmode = APPMODE_RENDERING;
}

static int finalimagew;
static int finalimageh;
static Vec2i finalclipsz;
static Vec2i finalclipmin;
static Vec2i finalclipmax;
static Vec2i finalcenter;

void LoadRender(int rendstage, LoadedTex* finalsprite)
{
	//file ident
	int ressz = 0;
	fread(&ressz, sizeof(int), 1, g_rescachefp);
	fseek(g_rescachefp, ressz, SEEK_CUR);

	fread(&finalsprite->sizex, sizeof(int), 1, g_rescachefp);
	fread(&finalsprite->sizey, sizeof(int), 1, g_rescachefp);
	fread(&finalsprite->channels, sizeof(int), 1, g_rescachefp);

	AllocTex(finalsprite, 
		finalsprite->sizex, finalsprite->sizey,
		finalsprite->channels);

	fread(finalsprite->data, sizeof(unsigned char), 
		finalsprite->sizex*finalsprite->sizey*finalsprite->channels, 
		g_rescachefp);

	if(rendstage == RENDSTAGE_COLOR)
	{
		fread(&finalcenter.x, sizeof(int), 1, g_rescachefp);
		fread(&finalcenter.y, sizeof(int), 1, g_rescachefp);
		fread(&finalimagew, sizeof(int), 1, g_rescachefp);
		fread(&finalimageh, sizeof(int), 1, g_rescachefp);
		fread(&finalclipsz.x, sizeof(int), 1, g_rescachefp);
		fread(&finalclipsz.y, sizeof(int), 1, g_rescachefp);
		fread(&finalclipmin.x, sizeof(int), 1, g_rescachefp);
		fread(&finalclipmin.y, sizeof(int), 1, g_rescachefp);
		fread(&finalclipmax.x, sizeof(int), 1, g_rescachefp);
		fread(&finalclipmax.y, sizeof(int), 1, g_rescachefp);
	}

	static int spi = -1;

	if(rendstage == RENDSTAGE_DUMMY)
	{
		spi = NewSprite();

		if(spi < 0)
		{
			ErrMess("Error", "No more sprite slots");
			EndRender();
		}
	}
	else if(spi >= 0)
	{
		Sprite* sp = &g_sprite[spi];

		sp->on = ectrue;
		if(rendstage == RENDSTAGE_COLOR)
		{
			sp->cropoff[0] = -finalcenter.x;
			sp->cropoff[1] = -finalcenter.y;
			sp->cropoff[2] = (float)finalclipsz.x - (float)finalcenter.x;
			sp->cropoff[3] = (float)finalclipsz.y - (float)finalcenter.y;
			sp->crop[0] = 0;
			sp->crop[1] = 0;
			sp->crop[2] = (float)finalclipsz.x / (float)finalimagew;
			sp->crop[3] = (float)finalclipsz.y / (float)finalimageh;
			sp->offset[0] = -finalcenter.x;
			sp->offset[1] = -finalcenter.y;
			sp->offset[2] = finalimagew - finalcenter.x;
			sp->offset[3] = finalimageh - finalcenter.y;

			sp->difftexi = 0;
			sp->depthtexi = 0;
			sp->teamtexi = 0;
			sp->elevtexi = 0;
		}

		SpList* spl = &g_splist[g_rendspl];
		int ci = SpriteRef(spl, g_renderframe, g_currincline, g_rendpitch, imax(g_rendside, g_rendyaw), g_rendroll,
			g_slicex, g_slicey);

		spl->sprites[ci] = spi;

		if(ci >= spl->nsp)
		{
			char m[123];
			sprintf(m, "sp %d>%d", ci, spl->nsp);
			ErrMess("Error", m);
		}

		//if(g_exportdepth)
		if(rendstage == RENDSTAGE_COLOR //&& g_exportdiff
			)
		{
			sp->pixels = new LoadedTex;
			AllocTex(sp->pixels, finalsprite->sizex, finalsprite->sizey, finalsprite->channels);
			memcpy(sp->pixels->data, finalsprite->data, finalsprite->sizex*finalsprite->sizey*finalsprite->channels);
		}

		int texi = NewTexture();

		if(texi >= 0)
		{
			Texture* tex = &g_texture[texi];
			tex->loaded = ectrue;
			CreateTex(finalsprite, &tex->texname, ecfalse, ecfalse);

			char fullpath[SFH_MAX_PATH+1];
			NameSp(fullpath, rendstage);

			//sprintf(fullpath, "sp%s%s%s%s%s%s.png", g_spltorend[g_lastRSp].relative.c_str(),
			//	"", side, frame, incline.c_str(), stage.c_str());

			tex->fullpath = fullpath;
			tex->width = finalsprite->sizex;
			tex->height = finalsprite->sizey;
			//tex->
			unsigned int* sti = NULL;
			if(rendstage == RENDSTAGE_COLOR)
				sti = &sp->difftexi;
			else if(rendstage == RENDSTAGE_TEAM)
				sti = &sp->teamtexi;
			else if(rendstage == RENDSTAGE_DEPTH)
				sti = &sp->depthtexi;
			else if(rendstage == RENDSTAGE_ELEV)
				sti = &sp->elevtexi;
			//
			if(sti)
				*sti = texi;
		}
		else
		{
			ErrMess("Error", "No more texture slots");
		}
	}
	else
	{
		ErrMess("Error", "No more sprite slots");
	}

	finalsprite->destroy();
}

void SaveRender(int rendstage, LoadedTex* finalsprite)
{
#if 0
	SaveScreenshot();

	Log("sv r");
	
#endif

	LoadedTex prescreen;
	LoadedTex screen;

#if 0
	AllocTex(&screen, g_width, g_height, 3);
	memset(screen.data, 0, g_width * g_height * 3);

	// size must be multiple of 32 or else this will crash !!!!!
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screen.data);
	FlipImage(&screen);
#else
	int channels = 4;

	if(rendstage == RENDSTAGE_TEAM)
		channels = 1;

	//Must read RGBA from FBO, can't read GL_RED directly for team colour mask for some reason
	AllocTex(&prescreen, g_width, g_height, 4);
	memset(prescreen.data, 0, g_width * g_height * 4);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// size must be multiple of 32 or else this will crash !!!!!
#ifdef DEBUG
	CHECKGLERROR();
#endif
	if(rendstage != RENDSTAGE_DUMMY)
		glReadPixels(0, 0, g_width, g_height, channels == 4 ? GL_RGBA : GL_RGBA, GL_UNSIGNED_BYTE, prescreen.data);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	FlipImage(&prescreen);
#endif

	AllocTex(&screen, g_width, g_height, channels);
	memset(screen.data, 0, g_width * g_height * channels);

	//Convert to 1-channel if team colour mask render
	for(int x=0; x<g_width; x++)
		for(int y=0; y<g_height; y++)
		{
			int i = ( x + y * g_width ) * 4;
			int i2 = ( x + y * g_width ) * channels;

			for(int c=0; c<channels; c++)
				screen.data[i2+c] = prescreen.data[i+c];
		}

	//if(g_clipmin.x < 0)
	//	ErrMess("asd","asda");
	//if(g_clipmin.x<0)
	//	exit(0);

	Vec2i clipsz;
	clipsz.x = g_clipmax.x - g_clipmin.x;
	clipsz.y = g_clipmax.y - g_clipmin.y;

#ifdef DEBUG
	char msg[128];
	sprintf(msg, "clipsz %d,%d->%d,%d sz(%d,%d)", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y, clipsz.x, clipsz.y);
	Log(msg<<std::endl;
	//InfoMess(msg, msg);
#endif

	int imagew;
	int imageh;

	if(!g_fit2pow)
	{
		imagew = clipsz.x;
		imageh = clipsz.y;
	}
	else
	{
		imagew = Max2Pow(clipsz.x);
		imageh = Max2Pow(clipsz.y);
	}

#if 0
	int downimagew = imagew;
	int downimageh = imageh;
	Vec2i downclipsz = clipsz;

	if(g_antialias)
	{
		//deal with non-power-of-2 upscales.
		downclipsz = clipsz / ANTIALIAS_UPSCALE;

	}
#endif

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" gwh"<<g_width<<","<<g_height<<" clipxymm "<<g_clipmin.x<<","<<g_clipmin.y<<"->"<<g_clipmax.x<<","<<g_clipmax.y<<" clipsz "<<clipsz.x<<","<<clipsz.y<<" imgwh "<<imagew<<","<<imageh<<std::endl;
	

	Log("sv r 4");
	
#endif

	LoadedTex sprite;
	AllocTex(&sprite, imagew, imageh, channels);

	int transpkey[3] = {(int)(g_transpkey[0]*255), (int)(g_transpkey[1]*255), (int)(g_transpkey[2]*255)};

	int xoff = g_clipmin.x;
	int yoff = g_clipmin.y;

	//if(g_deswidth != g_width)	xoff += (g_width-g_deswidth)/2;
	//if(g_desheight != g_height)	yoff += (g_height-g_desheight)/2;

#ifdef DEBUG
	Log("des wh "<<g_deswidth<<","<<g_desheight<<" actl "<<g_width<<","<<g_height<<std::endl;
	Log("xy off "<<xoff<<","<<yoff<<std::endl;
	
#endif

#if 1
	for(int x=0; x<imagew; x++)
		for(int y=0; y<imageh; y++)
		{
			int index = channels * ( y * imagew + x );
#if 0
			sprite.data[index + 0] = transpkey[0];
			sprite.data[index + 1] = transpkey[1];
			sprite.data[index + 2] = transpkey[2];
			sprite.data[index + 3] = 0;
#else
			for(int c=0; c<channels; c++)
				sprite.data[index + c] = 0;
#endif
		}
#endif

#if 1
	for(int x=0; x<clipsz.x && x<g_width; x++)
		for(int y=0; y<clipsz.y && y<g_height; y++)
		{
			int index = channels * ( y * imagew + x );
			int index2 = channels * ( (y+yoff) * g_width + (x+xoff) );

			//index2 = imin( g_width * g_height, index2 );
			
				if(index < 0 || index >= clipsz.x * clipsz.y * 4)
					ErrMess("!12","!12");
				if(index < 0)
					ErrMess("!1111","!1111");
				if(index >= clipsz.x * clipsz.y * 4)
					ErrMess("!222","!222");
				if(index >= clipsz.x * clipsz.y * 4)
				{
					char mm[234];
					sprintf(mm, "cs %d,%d", clipsz.x, clipsz.y);
					ErrMess(mm,mm);
				}
				if(imagew < 0)
					ErrMess("!123","!123");
				
				if(index2 < 0 || index2 >= g_width * g_height * 4)
					ErrMess("!13","!13");

#if 0
			//should have RGBA in FBO
			if(channels >= 3
			&& screen.data[index2+0] == transpkey[0]
			&& screen.data[index2+1] == transpkey[1]
			&& screen.data[index2+2] == transpkey[2])
				continue;
#endif
			//Log(" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" ");
			//

			for(int c=0; c<channels; c++)
				sprite.data[index+c] = screen.data[index2+c];
		}
#endif

	finalimagew = imagew;
	finalimageh = imageh;
	finalclipsz = clipsz;
	finalclipmin = g_clipmin;
	finalclipmax = g_clipmax;
	finalcenter = g_spritecenter;

	//LoadedTex finalsprite;

	if(g_antialias)
	{
		//downsample the sprite
		//and update the clip paramaters.

		int downimagew = imagew / ANTIALIAS_UPSCALE;
		int downimageh = imageh / ANTIALIAS_UPSCALE;
		Vec2i downclipsz = clipsz / ANTIALIAS_UPSCALE;
		Vec2i downclipmin = g_clipmin / ANTIALIAS_UPSCALE;
		Vec2i downclipmax = g_clipmax / ANTIALIAS_UPSCALE;

		AllocTex(finalsprite, downimagew, downimageh, channels);

		for(int x=0; x<downimagew; x++)
			for(int y=0; y<downimageh; y++)
			{
				int index = channels * ( y * downimagew + x );

				if(index < 0 || index >= downimagew * downimageh * 4)
					ErrMess("!1","!1");
#if 0
				sprite.data[index + 0] = transpkey[0];
				sprite.data[index + 1] = transpkey[1];
				sprite.data[index + 2] = transpkey[2];
				sprite.data[index + 3] = 0;
#else
				for(int c=0; c<channels; c++)
					finalsprite->data[index + c] = 0;
#endif
			}

		for(int downx=0; downx<downclipsz.x; downx++)
			for(int downy=0; downy<downclipsz.y; downy++)
			{
				int downindex = channels * ( downy * downimagew + downx );
				//int upindex2 = 4 * ( (y+yoff) * g_width + (x+xoff) );

				unsigned int samples[4] = {0,0,0,0};

				int contributions = 0;

				for(int upx=0; upx<ANTIALIAS_UPSCALE; upx++)
					for(int upy=0; upy<ANTIALIAS_UPSCALE; upy++)
					{
						int upindex = channels * ( (downy*ANTIALIAS_UPSCALE + upy) * imagew + (downx*ANTIALIAS_UPSCALE + upx) );

						unsigned char* uppixel = &sprite.data[upindex];

						//if it's a transparent pixel, we don't want to
						//blend in the transparency key color.
						//instead, blend in black.
						if(channels == 4
						&&
							((uppixel[0] == transpkey[0]
							&& uppixel[1] == transpkey[1]
							&& uppixel[2] == transpkey[2])
							||
							(uppixel[3] == 0))
						)
						{
							samples[0] += 0;
							samples[1] += 0;
							samples[2] += 0;
							samples[3] += 0;
						}
						else
						{
							for(int c=0; c<channels; c++)
								samples[c] += uppixel[c];
							contributions++;
						}
					}

				//average the samples
				if(contributions > 0)
					for(int c=0; c<channels; c++)
						samples[c] /= contributions;
	#if 0
				//should have RGBA in FBO
				if(screen.data[index2+0] == transpkey[0]
				&& screen.data[index2+1] == transpkey[1]
				&& screen.data[index2+2] == transpkey[2])
					continue;
	#endif
				//Log(" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" ");
				//

				for(int c=0; c<channels; c++)
					finalsprite->data[downindex+c] = samples[c];
			}


		finalimagew = downimagew;
		finalimageh = downimageh;
		finalclipsz = downclipsz;
		finalclipmin = downclipmin;
		finalclipmax = downclipmax;
		finalcenter = finalcenter / ANTIALIAS_UPSCALE;
	}
	else
	{
		//no downsampling.
		AllocTex(finalsprite, imagew, imageh, channels);

		for(int x=0; x<imagew; x++)
			for(int y=0; y<imageh; y++)
			{
				int index = channels * ( y * imagew + x );

				if(index < 0 || index >= imagew * imageh * 4)
					ErrMess("!11","!11");

				for(int c=0; c<channels; c++)
					finalsprite->data[index + c] = sprite.data[index + c];
			}
	}

}

void NameSp(char* fullpath, int rendstage)
{
	strcpy(frame, "");
	strcpy(side, "");

	if(g_doframes)
		sprintf(frame, "_fr%03d", g_renderframe);

	if(g_dosides && !g_dorots)
		sprintf(side, "_si%d", g_rendside);
	else if(g_dorots)
		sprintf(side, "_y%dp%dr%d", g_rendyaw, g_rendpitch, g_rendroll);

	incline = "";

	if(g_doinclines)
	{
		if(g_currincline == INC_0000)	incline = "_inc0000";
		else if(g_currincline == INC_0001)	incline = "_inc0001";
		else if(g_currincline == INC_0010)	incline = "_inc0010";
		else if(g_currincline == INC_0011)	incline = "_inc0011";
		else if(g_currincline == INC_0100)	incline = "_inc0100";
		else if(g_currincline == INC_0101)	incline = "_inc0101";
		else if(g_currincline == INC_0110)	incline = "_inc0110";
		else if(g_currincline == INC_0111)	incline = "_inc0111";
		else if(g_currincline == INC_1000)	incline = "_inc1000";
		else if(g_currincline == INC_1001)	incline = "_inc1001";
		else if(g_currincline == INC_1010)	incline = "_inc1010";
		else if(g_currincline == INC_1011)	incline = "_inc1011";
		else if(g_currincline == INC_1100)	incline = "_inc1100";
		else if(g_currincline == INC_1101)	incline = "_inc1101";
		else if(g_currincline == INC_1110)	incline = "_inc1110";
	}

	stage = "";
	
	if(rendstage == RENDSTAGE_TEAM)
		stage = "_team";
	else if(rendstage == RENDSTAGE_DEPTH)
		stage = "_depth";

	//char mm[123];
	//sprintf(mm, "re %d,%d,%d", finalsprite->sizex, finalsprite->sizey, finalsprite->channels);
	//InfoMess("re:",mm);

	char slice[16] = "";

	if(g_nslices>1)
	{
		sprintf(slice, "_sl%d,%d", g_slicex, g_slicey);
	}

	sprintf(fullpath, "sp%s%s%s%s%s%s%s.png", g_spltorend[g_lastRSp-1].relative.c_str(),
		"", side, frame, incline.c_str(), slice, stage.c_str());
}

//int thetex = -1;

void CreateSp(int rendstage, LoadedTex* finalsprite)
{
	char fullpath[SFH_MAX_PATH+1];

	NameSp(fullpath, rendstage);
	
	//sprintf(fullpath, "sp%s%s%s%s%s%s.txt", g_spltorend[g_lastRSp].relative.c_str(),
	//	"", side, frame, incline.c_str(), stage.c_str());
#if 0
	if(g_rendspl >= 0)
	{
		
		SpList* spl = &g_splist[g_rendspl];
		int ci = SpriteRef(spl, g_renderframe, g_currincline, g_rendpitch, imax(g_rendside, g_rendyaw), g_rendroll);

		std::string fpp = std::string(iform(ci)) + fullpath;
		if(strstr(spl->fullpath.c_str(), "trfac"))
		SavePNG2(fpp.c_str(), finalsprite);
	}
#endif

	//3,10,12
#if 0
	char p2[123];
	sprintf(p2, "%d.png", (int)g_currincline);
	if(g_rendertype == RENDER_TERRTILE && rendstage == RENDSTAGE_COLOR)
	SavePNG2(p2, finalsprite);
	sprintf(p2, "%de.png", (int)g_currincline);
	if(g_rendertype == RENDER_TERRTILE && rendstage == RENDSTAGE_ELEV)
	SavePNG2(p2, finalsprite);
	sprintf(p2, "%dd.png", (int)g_currincline);
	if(g_rendertype == RENDER_TERRTILE && rendstage == RENDSTAGE_DEPTH)
	SavePNG2(p2, finalsprite);
	
	sprintf(p2, "%drrd.png", (int)g_currincline);
	if(g_rendertype == RENDER_MODEL && rendstage == RENDSTAGE_DEPTH)
	SavePNG2(p2, finalsprite);
	
	sprintf(p2, "%drrc.png", (int)g_currincline);
	if(g_rendertype == RENDER_MODEL && rendstage == RENDSTAGE_COLOR)
	SavePNG2(p2, finalsprite);
	
	sprintf(p2, "%drre.png", (int)g_currincline);
	if(g_rendertype == RENDER_MODEL && rendstage == RENDSTAGE_ELEV)
	SavePNG2(p2, finalsprite);
#endif
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);

	//if(strstr(g_spltorend[g_lastRSp].relative.c_str(), "lab"))
	{
		////char m[123];
		//sprintf(m, "fp %s \r\n f%d/%d %d", fullpath, g_renderframe, g_renderframes,
		//	g_spltorend[g_lastRSp].nframes);
		//InfoMess(m,m);
	}//

	finalclipmax.x = finalclipmax.x - finalclipmin.x;
	finalclipmax.y = finalclipmax.y - finalclipmin.y;
	finalclipmin.x = 0;
	finalclipmin.y = 0;

	///sprintf(fullpath, "%s%s%s%s.txt", g_renderbasename, side, frame, incline.c_str());
	///std::ofstream ofs(fullpath, std::ios_base::out);
	///ofs<<finalcenter.x<<" "<<finalcenter.y<<std::endl;
	///ofs<<finalimagew<<" "<<finalimageh<<std::endl;
	///ofs<<finalclipsz.x<<" "<<finalclipsz.y<<std::endl;
	///ofs<<finalclipmin.x<<" "<<finalclipmin.y<<" "<<finalclipmax.x<<" "<<finalclipmax.y;

	if(g_rescachefp && !g_rescacheread)
	{
		AddResCache(fullpath);

	//int wrsz = finalsprite->sizex*finalsprite->sizey*finalsprite->channels;

		fwrite(&finalsprite->sizex, sizeof(int), 1, g_rescachefp);
		fwrite(&finalsprite->sizey, sizeof(int), 1, g_rescachefp);
		fwrite(&finalsprite->channels, sizeof(int), 1, g_rescachefp);
		//fwrite(finalsprite->data, sizeof(unsigned char) * //doesn't work vc9 winxp but did vc11 win7//nvm
		fwrite(&finalsprite->data[0], sizeof(unsigned char) * 
			finalsprite->sizex*finalsprite->sizey*finalsprite->channels, 
			1, 
			g_rescachefp);

	//char m[123];
	//sprintf(m, "wrsz%d %d,%d,%d", wrsz,
	//	finalsprite->sizex,finalsprite->sizey,finalsprite->channels);
	//if(rand()%5==1)
	//	InfoMess(m,m);

		///sprintf(fullpath, "sp%s%s%s%s%s%s.txt", g_spltorend[g_lastRSp].relative.c_str(),
		//"", side, frame, incline.c_str(), stage.c_str());
		//AddResCache(fullpath);
		
		if(rendstage == RENDSTAGE_COLOR /* && g_exportdiff */)
		{
			fwrite(&finalcenter.x, sizeof(int), 1, g_rescachefp);
			fwrite(&finalcenter.y, sizeof(int), 1, g_rescachefp);
			fwrite(&finalimagew, sizeof(int), 1, g_rescachefp);
			fwrite(&finalimageh, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipsz.x, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipsz.y, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipmin.x, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipmin.y, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipmax.x, sizeof(int), 1, g_rescachefp);
			fwrite(&finalclipmax.y, sizeof(int), 1, g_rescachefp);
		}
		fflush(g_rescachefp);
	}

	static int spi = -1;
	
	if(rendstage == RENDSTAGE_DUMMY)
	{
		spi = NewSprite();

		if(spi < 0)
		{
			ErrMess("Error", "No more sprite slots");
			EndRender();
		}
	}
	else if(spi >= 0)
	{
		Sprite* sp = &g_sprite[spi];

		sp->on = ectrue;
		if(rendstage == RENDSTAGE_COLOR)
		{
			sp->cropoff[0] = -finalcenter.x;
			sp->cropoff[1] = -finalcenter.y;
			sp->cropoff[2] = (float)finalclipsz.x - (float)finalcenter.x;
			sp->cropoff[3] = (float)finalclipsz.y - (float)finalcenter.y;
			sp->crop[0] = 0;
			sp->crop[1] = 0;
			sp->crop[2] = (float)finalclipsz.x / (float)finalimagew;
			sp->crop[3] = (float)finalclipsz.y / (float)finalimageh;
			sp->offset[0] = -finalcenter.x;
			sp->offset[1] = -finalcenter.y;
			sp->offset[2] = finalimagew - finalcenter.x;
			sp->offset[3] = finalimageh - finalcenter.y;

			sp->difftexi = 0;
			sp->depthtexi = 0;
			sp->teamtexi = 0;
			sp->elevtexi = 0;
		}

#if 0
	fscanf(fp, "%f %f", &centerx, &centery);
	fscanf(fp, "%f %f", &width, &height);
	fscanf(fp, "%f %f", &clipszx, &clipszy);
	fscanf(fp, "%f %f %f %f", &clipminx, &clipminy, &clipmaxx, &clipmaxy);
	
	s->offset[0] = -centerx;
	s->offset[1] = -centery;
	s->offset[2] = s->offset[0] + width;
	s->offset[3] = s->offset[1] + height;

	//s->crop[0] = clipminx / width;
	//s->crop[1] = clipminy / height;
	//s->crop[2] = clipmaxx / width;
	//s->crop[3] = clipmaxy / height;
	//s->crop[2] = (clipminx + clipszx) / width;
	//s->crop[3] = (clipminy + clipszy) / height;
	s->crop[0] = 0;
	s->crop[1] = 0;
	s->crop[2] = clipszx / width;
	s->crop[3] = clipszy / height;

	//s->cropoff[0] = clipminx - centerx;
	//s->cropoff[1] = clipminy - centery;
	//s->cropoff[2] = clipmaxx - centerx;
	//s->cropoff[3] = clipmaxy - centery;
	//s->cropoff[2] = clipminx + clipszx - centerx;
	//s->cropoff[3] = clipminy + clipszy - centery;
	s->cropoff[0] = -centerx;
	s->cropoff[1] = -centery;
	s->cropoff[2] = clipszx - centerx;
	s->cropoff[3] = clipszy - centery;
#endif

		//static int lastci = -1;

		SpList* spl = &g_splist[g_rendspl];
		int ci = SpriteRef(spl, g_renderframe, g_currincline, g_rendpitch, imax(g_rendside, g_rendyaw), g_rendroll,
			g_slicex, g_slicey);

		//if(ci > 0 && spl->sprites[ci-1] != spi-1)
		//if(lastci != ci-1 && ci!=0 && (rendstage ==RENDSTAGE_COLOR || rendstage==RENDSTAGE_DUMMY))
		{
		//	char m[123];
		//	sprintf(m, "sp %d<>%d,%d l%d %d\r\n%d,%d,%d,%d,%d\r\n%d,%d", ci, spl->sprites[ci-1], spi-1, g_rendspl, lastci,
		//		g_renderframe, g_currincline, imax(g_rendside, g_rendpitch), g_rendyaw, g_rendroll,
		//		(int)spl->frames, (int)spl->sides);
		//	ErrMess("Error", m);
		//	lastci = ci;
		}

		spl->sprites[ci] = spi;

		if(ci >= spl->nsp)
		{
			char m[123];
			sprintf(m, "sp %d>%d", ci, spl->nsp);
			ErrMess("Error", m);
		}
 
		//if(strstr(spl->fullpath.c_str(), "lab"))
		//{
		//	char m[123];
		//	sprintf(m, "ci%d", ci);
		//	InfoMess(m,m);
		//}

		//if(g_exportdepth)
		if(rendstage == RENDSTAGE_COLOR)
		{
			sp->pixels = new LoadedTex;
			AllocTex(sp->pixels, finalsprite->sizex, finalsprite->sizey, finalsprite->channels);
			memcpy(sp->pixels->data, finalsprite->data, finalsprite->sizex*finalsprite->sizey*finalsprite->channels);
		}

		int texi = NewTexture();

		if(texi >= 0)
		{
			Texture* tex = &g_texture[texi];
			tex->loaded = ectrue;
			CreateTex(finalsprite, &tex->texname, ecfalse, ecfalse);

			tex->fullpath = fullpath;
			tex->width = finalsprite->sizex;
			tex->height = finalsprite->sizey;
			//tex->
			unsigned int* sti = NULL;
			if(rendstage == RENDSTAGE_COLOR)
				sti = &sp->difftexi;
			else if(rendstage == RENDSTAGE_TEAM)
				sti = &sp->teamtexi;
			else if(rendstage == RENDSTAGE_DEPTH)
				sti = &sp->depthtexi;
			else if(rendstage == RENDSTAGE_ELEV)
				sti = &sp->elevtexi;
			//
			if(sti)
				*sti = texi;

			
		//if(strstr(spl->fullpath.c_str(), "trfac") && rendstage == RENDSTAGE_COLOR)
		//	thetex = texi;

			//DrawImage(tex->texname, 0, 0, g_width, g_height, 0, 0, 1, 1, g_gui.crop);
		}
		else
		{
			ErrMess("Error", "No more texture slots");
		}
	}
	else
	{
		ErrMess("Error", "No more sprite slots");
	}
}

void SpriteRender(int rendstage, Vec3f offset)
{
	//glViewport(0, 0, g_width, g_height);
	if(rendstage == RENDSTAGE_TEAM)
		glClearColor(0,0,0,0);
		//glClearColor(g_transpkey[0],g_transpkey[1],g_transpkey[2],1);
	else if(rendstage == RENDSTAGE_COLOR ||
		rendstage == RENDSTAGE_DUMMY )
		//glClearColor(g_transpkey[0],g_transpkey[1],g_transpkey[2],0);
		glClearColor(0,0,0,0);
	else if(rendstage == RENDSTAGE_DEPTH)
		glClearColor(0,0,0,0);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef DEBUG
	CHECKGLERROR();
	//Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
	//Log(__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
#endif

	{
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection;

		VpWrap* v = &g_viewport[3];
		VpType* t = &g_vptype[v->type];

		ecbool persp = ecfalse;

#if 0
		if(g_projtype == PROJ_PERSP && v->type == VIEWPORT_ANGLE45O)
		{
			projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
			persp = ectrue;
		}
		else if(g_projtype == PROJ_ORTHO || v->type != VIEWPORT_ANGLE45O)
#endif
		{
			projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
		}

		//Vec3f viewvec = g_focus;	//g_cam.view;
		//Vec3f viewvec = g_cam.view;
		Vec3f focusvec = v->focus() + offset;
		//Vec3f posvec = g_focus + t->offset;
		//Vec3f posvec = g_cam.pos;
		Vec3f posvec = v->pos() + offset;

		//if(v->type != VIEWPORT_ANGLE45O)
		//	posvec = g_cam.view + t->offset;

		//viewvec = posvec + Normalize(viewvec-posvec);
		//Vec3f posvec2 = g_cam.lookpos() + t->offset;
		//Vec3f upvec = t->up;
		//Vec3f upvec = g_cam.up;
		Vec3f upvec = v->up();

		//if(v->type != VIEWPORT_ANGLE45O)
		//	upvec = t->up;

		Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.translation(translation);
		modelmat.translation(translation);
		modelview.postmult2(viewmat);

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		Matrix mvpmat;
		mvpmat.set(projection.m);
		mvpmat.postmult2(viewmat);

#if 0
		if(g_rendertype == RENDER_TERRTILE && 
			rendstage == RENDSTAGE_COLOR)
		{
			char m[123];
			sprintf(m, "%dirc.txt", (int)g_currincline);
			FILE* fp = fopen(m, "w");
			fprintf(fp, "%f,%f->%f,%f",
				(float)g_clipmin.x,
				(float)g_clipmin.y,
				(float)g_clipmax.x,
				(float)g_clipmax.y);
			fclose(fp);
		}
#endif

		//SDL_GL_SwapWindow(g_window);

#if 1
	//	if(v->type == VIEWPORT_ANGLE45O)
		{
			//RenderToShadowMap(projection, viewmat, modelmat, g_focus);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			//RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
			//if(rendstage == RENDSTAGE_COLOR)
			if(rendstage == RENDSTAGE_SHADOW)
			{
				RenderToShadowMap(projection, viewmat, modelmat, g_cam.view, DrawSceneDepth);
			}
			glClearColor(0,0,0,0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			//if(rendstage == RENDSTAGE_DUMMY)
			//	RenderToShadowMap(projection, viewmat, modelmat, g_cam.view, DrawSceneDepth);
			if( (rendstage == RENDSTAGE_COLOR || rendstage == RENDSTAGE_DUMMY ) //&&
				//g_exportdiff
				)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
			else if(rendstage == RENDSTAGE_TEAM)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawSceneTeam);
			else if(rendstage == RENDSTAGE_DEPTH)
				RenderToDepthMap(projection, viewmat, modelmat, g_cam.view, DrawSceneDepth);
			else if(rendstage == RENDSTAGE_ELEV)
				RenderToElevMap(projection, viewmat, modelmat, g_cam.view, DrawSceneElev);
		}
#endif
	}
	
	//glViewport(0, 0, g_origwidth, g_origheight);

#ifdef DEBUG
	CHECKGLERROR();
#endif
	glFlush();
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glFinish();
#ifdef DEBUG
	CHECKGLERROR();
#endif
}

void RotateView()
{
	if(g_nrendsides <= 0)
		g_nrendsides = 1;
	
	if(!g_dorots)
	{
		g_lightPos = Rotate(g_lightPos, g_rendside*(DEGTORAD(360.0)/(float)g_nrendsides), 0, 1, 0);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendside*(DEGTORAD(360.0)/(float)g_nrendsides), 0, 1, 0);
	}
	else
	{
		Vec3f pitchaxis(1,0,0);
		Vec3f yawaxis(0,1,0);
		Vec3f rollaxis(0,0,1);
		
		//g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderyaw), 0, 1, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderpitch), 1, 0, 0);

#if 0
		//g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);
		
		pitchaxis = Rotate(pitchaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		yawaxis = Rotate(yawaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		rollaxis = Rotate(rollaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		
		pitchaxis = Rotate(pitchaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
		yawaxis = Rotate(yawaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
		rollaxis = Rotate(rollaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
#endif
		
#if 0
		char m[123];
		sprintf(m, "before (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
			g_cam.pos.x, g_cam.pos.y, g_cam.pos.z,
			g_cam.view.x, g_cam.view.y, g_cam.view.z,
			g_cam.up2().x, g_cam.up2().y, g_cam.up2().z);
		InfoMess(m,m);
#endif

		//rollaxis = Rotate(rollaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		pitchaxis = Rotate(pitchaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		yawaxis = Rotate(yawaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		
#if 0
		sprintf(m, "about (%f,%f,%f),%f", 
			rollaxis.x, rollaxis.y, rollaxis.z, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides));
		InfoMess(m,m);
#endif

#if 1
		rollaxis = Rotate(rollaxis, g_rendpitch*((float)DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		//pitchaxis = Rotate(pitchaxis, g_rendpitch*((float)DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		yawaxis = Rotate(yawaxis, g_rendpitch*((float)DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendpitch*((float)DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendpitch*((float)DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);

		rollaxis = Rotate(rollaxis, g_rendyaw*((float)DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		pitchaxis = Rotate(pitchaxis, g_rendyaw*((float)DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		//yawaxis = Rotate(yawaxis, g_rendyaw*((float)DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendyaw*((float)DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendyaw*((float)DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
#endif
	
		//g_cam.rotateabout(Vec3f(0,0,0), -(float)DEGTORAD(g_defrenderpitch), 1, 0, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), (float)DEGTORAD(g_defrenderyaw), 0, 1, 0);
		
#if 0
		//char m[123];
		sprintf(m, "after (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
			g_cam.pos.x, g_cam.pos.y, g_cam.pos.z,
			g_cam.view.x, g_cam.view.y, g_cam.view.z,
			g_cam.up2().x, g_cam.up2().y, g_cam.up2().z);
		InfoMess(m,m);
#endif
	}
}

void Draw2()
{
#ifdef DEBUG
	//Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
    CHECKGLERROR();

    if(g_appmode == APPMODE_LOADING)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else if(g_appmode == APPMODE_EDITOR)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else if(g_appmode == APPMODE_RENDERING)
        //glClearColor(g_transpkey[0], g_transpkey[1], g_transpkey[2], 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

#ifdef DEBUG
	//Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

    //g_gui.frameupd();

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

#ifdef DEBUG
	//Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

	///Ortho(g_width,g_height,1,1,1,1);
	//DrawViewport(3, 0, 0, g_width, g_height);
    //g_gui.draw();
    //DrawEdMap(&g_edmap);
	///EndS();

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CHECKGLERROR();
#endif

    if(g_appmode == APPMODE_EDITOR)
    {
#ifdef DEBUG
        LastNum(__FILE__, __LINE__);
#endif
        Ortho(g_width, g_height, 1, 1, 1, 1);
        char dbgstr[128];
        sprintf(dbgstr, "b's:%d", (int)g_edmap.brush.size());
        RichText rdbgstr(dbgstr);
        DrawShadowedText(MAINFONT8, 0, g_height-16, &rdbgstr);
        EndS();
    }

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	//("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

    SDL_GL_SwapWindow(g_window);
}

void UpdRend()
{
	SpList* spl = &g_splist[g_rendspl];

	g_dosides = spl->sides;
	g_dorots = spl->rotations;
	g_doinclines = spl->inclines;
	g_doframes = spl->frames;
	g_nrendsides = spl->nsides;
	g_renderframes = spl->nframes;
	g_nslices = spl->nslices;
	
	g_appmode = APPMODE_RENDERING;
	AdjustFrame();

	//if(g_nrendsides == 8 && g_renderframes == 3)
	//{
	//	static int i=0;

	//	++i;

	//	char m[123];
	//	sprintf(m, "i%d f%d s%d %s", i, g_renderframe, g_rendside, spl->fullpath.c_str());
	//	InfoMess(m,m);
	//}

	if(!g_rescachefp || !g_rescacheread)
	{
		ResetView(ectrue);

		RotateView();

		SortEdB(&g_edmap, g_cam.view, g_cam.pos);

		/////AdjustFrame(ecfalse);
		//AllScreenMinMax needs to be called again because the pixels center of rendering depends on the window width and height, influencing the clip min/max
		AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);
		//FitFocus(g_clipmin, g_clipmax);
		//AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);

	#if 0
		char msg[128];
		sprintf(msg, "clip %d,%d->%d,%d", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y);
		InfoMess(msg, msg);
	#endif

		//Because we're always centered on origin, we can do this:
		g_spritecenter.x = g_width/2 - g_clipmin.x;
		g_spritecenter.y = g_height/2 - g_clipmin.y;

		glEnable(GL_BLEND);

		//int oldmode = g_appmode;
		//g_appmode = APPMODE_EDITOR;
		//Draw2();
		///Draw2();	//double buffered
		//g_appmode = oldmode;
	#if 0
		Draw();
	#elif 0
		Ortho(g_width, g_height, 1, 1, 1, 1);
		DrawViewport(3, 0, 0, g_width, g_height);
		EndS();
	#else
		//glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
	#ifdef DEBUG
	//	Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
		CHECKGLERROR();
	#endif
	#ifdef DEBUG
		CHECKGLERROR();
	#endif
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glEnable(GL_DEPTH_TEST);
	#ifdef DEBUG
		CHECKGLERROR();
	#endif

		//get clip coordinates now that we've adjusted screen size (?)
		//AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);

	#if 0
		char msg[128];
		sprintf(msg, "clip %d,%d->%d,%d", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y);
		InfoMess(msg, msg);
	#endif
	}

	Vec3f offset;

	//g_1tilewidth
	//TILE_RISE

	GLint drawFboId = -1;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &drawFboId);

	//g_basedepth = MAG_VEC3F((g_cam.view - g_cam.pos));
	//g_basedepth = ;

	LoadedTex finalsprite;

	if(g_rescachefp && g_rescacheread)
	{
			LoadRender(RENDSTAGE_DUMMY, &finalsprite);
			//if(g_exportdiff)
				LoadRender(RENDSTAGE_COLOR, &finalsprite);
			if(g_rendertype != RENDER_TERRTILE &&
				g_exportteam)
				LoadRender(RENDSTAGE_TEAM, &finalsprite);
			if(g_exportdepth)
				LoadRender(RENDSTAGE_DEPTH, &finalsprite);
			if(g_exportelev)
				LoadRender(RENDSTAGE_ELEV, &finalsprite);
	}
	else
	{
		//if(!g_antialias)
		{
			//MakeFBO(0, RENDSTAGE_DUMMY);
			//glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			SpriteRender(RENDSTAGE_SHADOW, offset);
			//SDL_GL_SwapWindow(g_window);
			//SpriteRender(RENDSTAGE_SHADOW, offset);
			//SDL_GL_SwapWindow(g_window);
			SaveRender(RENDSTAGE_SHADOW, &finalsprite);
			//DelFBO(0);
			//glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
			//CreateSp(RENDSTAGE_SHADOW, &finalsprite);
			finalsprite.destroy();

			//dummy prep render
			MakeFBO(0, RENDSTAGE_DUMMY);
			glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			glViewport(0, 0, g_width, g_height);
			SpriteRender(RENDSTAGE_DUMMY, offset);
			//SDL_GL_SwapWindow(g_window);
			//SpriteRender(RENDSTAGE_DUMMY, offset);
			//SDL_GL_SwapWindow(g_window);
			SaveRender(RENDSTAGE_DUMMY, &finalsprite);
			DelFBO(0);
			glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
			CreateSp(RENDSTAGE_DUMMY, &finalsprite);
			finalsprite.destroy();

			MakeFBO(0, RENDSTAGE_COLOR);
			glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			glViewport(0, 0, g_width, g_height);
			SpriteRender(RENDSTAGE_COLOR, offset);
			//SDL_GL_SwapWindow(g_window);
			//SpriteRender(RENDSTAGE_COLOR, offset);
			//SDL_GL_SwapWindow(g_window);
			SaveRender(RENDSTAGE_COLOR, &finalsprite);
			DelFBO(0);
			glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
			CreateSp(RENDSTAGE_COLOR, &finalsprite);
#if 0
			if(g_rendertype == RENDER_UNSPEC && 
				g_currincline == 0 &&
				g_dosides)
			{
				SavePNG2("asdasd.png", &finalsprite);
				SDL_Delay(100000);
			}
#endif
			finalsprite.destroy();

		#ifdef DEBUG
			CHECKGLERROR();
		#endif
			if(g_rendertype != RENDER_TERRTILE &&
				g_exportteam)
			{
				MakeFBO(0, RENDSTAGE_TEAM);
				glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
				glViewport(0, 0, g_width, g_height);
				SpriteRender(RENDSTAGE_TEAM, offset);
				//SDL_GL_SwapWindow(g_window);
				//SpriteRender(RENDSTAGE_TEAM, offset);
				//SDL_GL_SwapWindow(g_window);
				SaveRender(RENDSTAGE_TEAM, &finalsprite);
				DelFBO(0);
				glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
				CreateSp(RENDSTAGE_TEAM, &finalsprite);
				finalsprite.destroy();
			}
		#ifdef DEBUG
			CHECKGLERROR();
		#endif
			
			if(g_exportdepth)
			{
				MakeFBO(0, RENDSTAGE_DEPTH);
				glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
				glViewport(0, 0, g_width, g_height);
				SpriteRender(RENDSTAGE_DEPTH, offset);
				//SDL_GL_SwapWindow(g_window);
				//SpriteRender(RENDSTAGE_DEPTH, offset);
				//SDL_GL_SwapWindow(g_window);
				SaveRender(RENDSTAGE_DEPTH, &finalsprite);
				DelFBO(0);
				glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
				CreateSp(RENDSTAGE_DEPTH, &finalsprite);
				finalsprite.destroy();
			}

		#endif
			if(g_exportelev)
			{
				MakeFBO(0, RENDSTAGE_ELEV);
				glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
				glViewport(0, 0, g_width, g_height);
				SpriteRender(RENDSTAGE_ELEV, offset);
				//SDL_GL_SwapWindow(g_window);
				//SpriteRender(RENDSTAGE_ELEV, offset);
				//SDL_GL_SwapWindow(g_window);
				SaveRender(RENDSTAGE_ELEV, &finalsprite);
				DelFBO(0);
				glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
				CreateSp(RENDSTAGE_ELEV, &finalsprite);
				finalsprite.destroy();
			}
		}

		glViewport(0, 0, g_origwidth, g_origheight);
	}
#if 0
	if(g_rendertype == RENDER_UNIT ||
		g_rendertype == RENDER_BUILDING ||
		g_doframes)
		g_renderframe++;
	else if(g_rendertype == RENDER_TERRTILE ||
		g_rendertype == RENDER_ROAD ||
		g_doinclines)
		g_currincline++;

	//If we're continuing the next render side or frame, or the end in other render types
	if(((g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING) && g_renderframe >= g_renderframes) ||
		((g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD) && g_currincline >= INCLINES))
	{
		if(g_rendside < g_nrendsides && g_rendertype == RENDER_UNIT)
		{
			g_renderframe = 0;
			g_rendside++;
			AdjustFrame();
		}
		else if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
		{
			g_currincline = 0;
			EndRender();
		}
		else
		{
			EndRender();
		}
	}
	else
	{
		AdjustFrame();
	}
#else
	//Advance render states
	//Are we doing sides and if so will the current side + 1 be valid? If so, advance
	if(g_rendertype == RENDER_TERRTILE && g_nslices > 1 && g_slicex+1 < g_nslices)
	{
		g_slicex++;
		AdjustFrame();
	}
	else if(g_rendertype == RENDER_TERRTILE && g_nslices > 1 && g_slicey+1 < g_nslices)
	{
		g_slicex = 0;
		g_slicey++;
		AdjustFrame();
	}
	else if(g_dosides && !g_dorots && g_rendside+1 < g_nrendsides)
	{
		g_slicex = 0;
		g_slicey = 0;
#if 0
		if(g_doframes && g_renderframes == 3)
		{
			char m[123];
			sprintf(m, "33 r1ss%d,rs%d,ds%d",
				g_nrendsides, g_rendside,
				(int)g_dosides);
			InfoMess("33",m);
		}
#endif	
		g_rendside++;
		AdjustFrame();
	}
	//Are we doing rotations?
	else if(g_dorots && g_rendroll+1 < g_nrendsides)
	{
		g_slicex = 0;
		g_slicey = 0;
		g_rendroll++;
		g_rendside = 0;
		AdjustFrame();
	}
	else if(g_dorots && g_rendpitch+1 < g_nrendsides)
	{
		g_slicex = 0;
		g_slicey = 0;
		g_rendpitch++;
		g_rendroll = 0;
		g_rendside = 0;
		AdjustFrame();
	}
	else if(g_dorots && g_rendyaw+1 < g_nrendsides)
	{
		g_slicex = 0;
		g_slicey = 0;
		g_rendyaw++;
		g_rendpitch = 0;
		g_rendroll = 0;
		g_rendside = 0;
		AdjustFrame();
	}
	//Else, advance some other variable
	else
	{
		g_slicex = 0;
		g_slicey = 0;
		g_rendside = 0;
		g_rendroll = 0;
		g_rendpitch = 0;
		g_rendyaw = 0;


#if 0
		if(g_doframes && g_renderframes == 3)
		{
			char m[123];
			sprintf(m, "33 rss%d,rs%d,ds%d",
				g_nrendsides, g_rendside,
				(int)g_dosides);
			InfoMess("33",m);
		}
#endif	

		if(g_doframes && g_renderframe+1 < g_renderframes)
		{
			g_renderframe++;
			AdjustFrame();
		}
		else
		{
			g_renderframe = 0;

			if(g_doinclines && g_currincline+1 < INCLINES)
			{
				g_currincline++;
				AdjustFrame();
			}
			else
			{
				g_currincline = 0;
				EndRender();
			}
		}
	}
#endif

	//Resize(g_origwidth, g_origheight);
	//glViewport(0, 0, g_origwidth, g_origheight);
}

void EndRender()
{
#if 0
	if(//
		SpriteRef(&g_splist[g_rendspl], g_renderframes-1, INCLINES-1, g_nrendsides-1, g_nrendsides-1, g_nrendsides-1)+1 <
		g_splist[g_rendspl].nsp)
	{
		char m[123];
		sprintf(m,"m%d<%d", 
			SpriteRef(&g_splist[g_rendspl], g_renderframes-1, INCLINES-1, g_nrendsides-1, g_nrendsides-1, g_nrendsides-1)+1,
		g_splist[g_rendspl].nsp);
		ErrMess("asd",m);
	}
#endif
#if 0
	for(int i=0; i<g_splist[g_rendspl].nsp; i++)
	{
		if(g_splist[g_rendspl].on)
		if(!g_splist[g_rendspl].sprites[i])
		{
			char m[123];
			sprintf(m, "m%d ! %d", i, g_splist[g_rendspl].sprites[i]);
		ErrMess("asd",m);

		}
	}
#endif
	FreeTile();
	FreeEdMap(&g_edmap);
	g_renderframe = 0;
	g_nslices = 1;
	g_lightPos = g_origlightpos;
	g_cam = g_origcam;
	CallResize(g_origwidth, g_origheight);
	ResetView(ecfalse);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
	//g_gui.reframe();
	
	//CallResize(g_origwidth, g_origheight);
	//InfoMess("asd<","asd");
	//char m[123];
	//sprintf(m, "5asd%d,%d", g_origwidth,g_origheight);
	//InfoMess("asd",m);
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0,g_origwidth,g_origheight);
	g_appmode = APPMODE_LOADING;
	//Widget *gui = (Widget*)&g_gui;
	//gui->hideall();
	//gui->show("render");
	//gui->show("loading");
	//glClearColor(0,0,0,0);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//gui->get("loading")->show();
	//Widget_reframe(gui);
	//InfoMess("asd","asd");
	EndS();
	//Ortho(g_width, g_height,1,1,1,1);
	SkipLogo();
	glClearColor(0,0,0,0);
}
