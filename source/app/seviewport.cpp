//
// weviewport.cpp
//
//

#include "seviewport.h"
#include "../render/shader.h"
#include "../gui/gui.h"
#include "../math/3dmath.h"
#include "../window.h"
#include "../platform.h"
#include "../gui/font.h"
#include "../math/camera.h"
#include "../math/matrix.h"
#include "../render/shadow.h"
#include "../sim/map.h"
#include "sesim.h"
#include "../math/vec4f.h"
#include "segui.h"
#include "../bsp/brush.h"
#include "../render/sortb.h"
#include "undo.h"
#include "../math/frustum.h"
#include "appmain.h"
#include "../sim/explocrater.h"
#include "../tool/rendersprite.h"
#include "../debug.h"

VpType g_vptype[VIEWPORT_TYPES];
VpWrap g_viewport[4];
//Vec3f g_focus;
static Vec3f accum(0,0,0);
ecbool g_changed = ecfalse;
UndoH g_beforechange;

VpType::VpType(Vec3f offset, Vec3f up, const char* label)
{
	offset = offset;
	up = up;
	strcpy(label, label);
}

VpWrap::VpWrap()
{
	drag = ecfalse;
	ldown = ecfalse;
	rdown = ecfalse;
}

VpWrap::VpWrap(int type)
{
	drag = ecfalse;
	ldown = ecfalse;
	rdown = ecfalse;
	mdown = ecfalse;
	type = type;
}

Vec3f VpWrap::up()
{
	Vec3f upvec = g_cam.up;
	VpType* t = &g_vptype[type];

	if(type != VIEWPORT_ANGLE45O)
		upvec = t->up;

	return upvec;
}

Vec3f VpWrap::up2()
{
	Vec3f upvec = g_cam.up2();
	VpType* t = &g_vptype[type];

	if(type != VIEWPORT_ANGLE45O)
		upvec = t->up;

	return upvec;
}

Vec3f VpWrap::strafe()
{
	Vec3f upvec = up();
	VpType* t = &g_vptype[type];
	Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->offset, upvec));

	if(type == VIEWPORT_ANGLE45O)
		sidevec = g_cam.strafe;

	return sidevec;
}

Vec3f VpWrap::focus()
{
	Vec3f viewvec = g_cam.view;
	return viewvec;
}

Vec3f VpWrap::viewdir()
{
	Vec3f focusvec = focus();
	Vec3f posvec = pos();
	//Vec3f viewvec = posvec + Normalize(focusvec-posvec);
	//return viewvec;
	return focusvec-posvec;
}

Vec3f VpWrap::pos()
{
	Vec3f posvec = g_cam.pos;

	if(g_projtype == PROJ_PERSP && type == VIEWPORT_ANGLE45O)
	{
		Vec3f dir = Normalize( g_cam.view - g_cam.pos );
		//posvec = g_cam.view - dir * 1000.0f / g_zoom;
		posvec = g_cam.view - dir * 100000.0f / g_zoom;
	}

	VpType* t = &g_vptype[type];

	if(type != VIEWPORT_ANGLE45O)
		posvec = g_cam.view + t->offset;

	return posvec;
}

// draw grid of dots and crosses
// for ease of placement/alignment
void DrawGrid(Vec3f vmin, Vec3f vmax)
{
	Shader* s = g_sh+g_curS;
	glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], 0.5f, 0.5f, 1.0f, 1.0f);

	// draw x axis
	float xaxisverts[] = {vmin.x, 0.0f, 0.0f, vmax.x, 0.0f, 0.0f};
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, xaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, xaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	// draw y axis
	float yaxisverts[] = {0.0f, vmin.y, 0.0f, 0.0f, vmax.y, 0.0f};
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, yaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, yaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	// draw z axis
	float zaxisverts[] = {0.0f, 0.0f, vmin.z, 0.0f, 0.0f, vmax.z};
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, zaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, zaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	//float interval = (10.0f/g_zoom);
	//float interval = log10(g_zoom * 100.0f) * 10.0f;
	//float interval = (int)(1.0f/g_zoom/10.0f)*10.0f;

	//float base = PROJ_RIGHT/20*2;
	//float invzoom = base/g_zoom;
	//int power = log(invzoom)/log(base);
	//float interval = pow(base, power);

	//STOREY_HEIGHT=250
	//float baseinterval = STOREY_HEIGHT / 5.0f;
	//float screenfit = PROJ_RIGHT*2/g_zoom;
	//int scale =

	//-  float invzoom = 2.0f/g_zoom;
	//-  int tenpower = log(invzoom)/log(2);
	//-  float interval = pow(2, tenpower);
	//+
	//+  float base = PROJ_RIGHT/15*2;
	//+  float invzoom = base/g_zoom;
	//+  int power = log(invzoom)/log(base);
	//+  float interval = pow(base, power);

	float base = 50.0f;
	//float invzoom = base/g_zoom;
	//int power = log(invzoom)/log(base);
	//float interval = pow(base, power);

	float interval;

	// zoom 1 -> interval 50
	// zoom 0.5 -> interval = 100
	// zoom 0.25 -> interval = 200
	// zoom 0.125 -> interval = 400
	// zoom 0.0625 -> interval 800

	//if(power == 0)
	//if(g_zoom > 1.0f)
	{
		//	interval = invzoom;
		//interval = base / pow(2, (int)g_zoom-1);
		int power2 = log(g_zoom) / log(2.0f);
		interval = base / pow(2.0f, (float)power2);
	}

	// zoom 1 -> interval 50 = 50 / 1 = 50 / 2^0
	// zoom 2 -> interval 25 = 50 / 2 = 50 / 2^1
	// zoom 3 -> interval 12.5 = 50 / 4 = 50 / 2^2
	// zoom 4 -> interval 6.25 = 50 / 8 = 50 / 2^3
	// zoom 5 -> interval 3.125 = 50 / 16 = 50 / 2^4
	// wrong


	Vec3f start = Vec3f( (int)(vmin.x/interval)*interval, (int)(vmin.y/interval)*interval, (int)(vmin.z/interval)*interval );
	Vec3f end = Vec3f( (int)(vmax.x/interval)*interval, (int)(vmax.y/interval)*interval, (int)(vmax.z/interval)*interval );

	//dots
	glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], 0.3f, 0.3f, 0.3f, 1.0f);
	for(float x=start.x; x<=end.x; x+=interval)
		for(float y=start.y; y<=end.y; y+=interval)
			for(float z=start.z; z<=end.z; z+=interval)
			{
				float point[] = {x, y, z};
				//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, point);
                glVertexPointer(3, GL_FLOAT, 0, point);
				glDrawArrays(GL_POINTS, 0, 1);
			}

			// crosses more spaced out
			float interval2 = interval * 5.0f;

			start = Vec3f( (int)(vmin.x/interval2)*interval2, (int)(vmin.y/interval2)*interval2, (int)(vmin.z/interval2)*interval2 );
			end = Vec3f( (int)(vmax.x/interval2)*interval2, (int)(vmax.y/interval2)*interval2, (int)(vmax.z/interval2)*interval2 );

			for(float x=start.x; x<=end.x; x+=interval2)
				for(float y=start.y; y<=end.y; y+=interval2)
					for(float z=start.z; z<=end.z; z+=interval2)
					{
						float xline[] = {x-interval/2.0f, y, z, x+interval/2.0f, y, z};
						//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, xline);
                        glVertexPointer(3, GL_FLOAT, 0, xline);
						glDrawArrays(GL_LINES, 0, 2);

						float yline[] = {x, y-interval/2.0f, z, x, y+interval/2.0f, z};
						//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, yline);
                        glVertexPointer(3, GL_FLOAT, 0, yline);
						glDrawArrays(GL_LINES, 0, 2);

						float zline[] = {x, y, z-interval/2.0f, x, y, z+interval/2.0f};
						//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, zline);
                        glVertexPointer(3, GL_FLOAT, 0, zline);
						glDrawArrays(GL_LINES, 0, 2);
					}
}

void DrawViewport(int which, int x, int y, int width, int height)
{

	//return;

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	EndS();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	ecbool persp = ecfalse;

	if(g_projtype == PROJ_PERSP && v->type == VIEWPORT_ANGLE45O)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else if(g_projtype == PROJ_ORTHO || v->type != VIEWPORT_ANGLE45O)
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	g_camproj = projection;

	//Vec3f viewvec = g_focus;	//g_cam.view;
	//Vec3f viewvec = g_cam.view;
	Vec3f focusvec = v->focus();
	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	posvec = g_cam.view + t->offset;

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up;
	Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

#if 0
	if(g_appmode == APPMODE_PRERENDADJFRAME || g_appmode == APPMODE_RENDERING)
	{
		focusvec = g_cam.view;
		posvec = g_cam.pos;
		Vec3f sidevec = Cross(Normalize(focusvec-posvec), Vec3f(0,1,0));
		upvec = Cross(sidevec, Normalize(focusvec-posvec));
	}
#endif

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

	g_camview = viewmat;

	Matrix modelview;
	Matrix modelmat;
	float translation[] = {0, 0, 0};
	modelview.translation(translation);
	modelmat.translation(translation);
	//modelview.postmult(viewmat);
	modelview.postmult(viewmat);

	g_cammodelview = modelview;

	Matrix mvpmat;
	mvpmat.set(projection.m);
	if(persp)
		mvpmat.postmult(viewmat);
	else
		mvpmat.postmult2(viewmat);
	g_cammvp = mvpmat;

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;
	//Vec3f vmin = g_focus - Vec3f(extentx, extenty, extentx);
	//Vec3f vmax = g_focus + Vec3f(extentx, extenty, extentx);
	Vec3f vmin = g_cam.view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.view + Vec3f(extentx, extenty, extentx);

	//draw only one layer of grid dots ...
	if(v->type == VIEWPORT_FRONT)	// ... on the x and y axises
	{
		vmin.z = 0;
		vmax.z = 0;
	}
	else if(v->type == VIEWPORT_LEFT)	// ... on the y and z axises
	{
		vmin.x = 0;
		vmax.x = 0;
	}
	else if(v->type == VIEWPORT_TOP)	// ... on the x and z axises
	{
		vmin.y = 0;
		vmax.y = 0;
	}

#if 1
	if(v->type == VIEWPORT_ANGLE45O)
	{
		EndS();
		//RenderToShadowMap(projection, viewmat, modelmat, g_focus);
#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif
		RenderToShadowMap(projection, viewmat, modelmat, g_cam.view, DrawSceneDepth);
#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif
		RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif
	//EndS();
	//Log("sh at t p:"<<g_curS<<std::endl;
	//

#if 1
	if(v->type == VIEWPORT_FRONT || v->type == VIEWPORT_LEFT || v->type == VIEWPORT_TOP)
	{
		UseS(SHADER_COLOR3D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		Shader* s = g_sh+g_curS;

		//if(s->slot[SSLOT_PROJECTION] == -1)	ErrMess("po", "po");
		//if(s->slot[SSLOT_VIEWMAT] == -1)	ErrMess("v", "v");
		//if(s->slot[SSLOT_MODELMAT] == -1)	ErrMess("m", "m");

#if 0
		for(int i=0; i<16; i+=4)
		{
			float* m = projection.m;
			Log("pr");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}

		for(int i=0; i<16; i+=4)
		{
			float* m = viewmat.m;
			Log("vm");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}

		for(int i=0; i<16; i+=4)
		{
			float* m = modelmat.m;
			Log("mm");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}
#endif

		glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m);
		glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m);
		glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m);
		glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m);
		//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
		DrawGrid(vmin, vmax);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		EndS();
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	//EndS();

#if 1
	//if(v->type == VIEWPORT_FRONT || v->type == VIEWPORT_LEFT || v->type == VIEWPORT_TOP)
	{
		Shader* s = g_sh+g_curS;

#if 1
		if(persp)
		//if(v->type == VIEWPORT_ANGLE45O)
		{
			UseS(SHADER_COLOR3DPERSP);
			s = g_sh+g_curS;
			glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m);
			glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m);
			glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m);
			glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m);
			//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
		}
		else
#endif
		{
			UseS(SHADER_COLOR3D);
			s = g_sh+g_curS;
			glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m);
			glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m);
			glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m);
			glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m);
		}

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		DrawFilled(&g_edmap, g_modelholder);
#ifdef DEBUG
		CHECKGLERROR();
		LastNum(__FILE__, __LINE__);
#endif

		if(v->type != VIEWPORT_ANGLE45O)
			DrawOutlines(&g_edmap, g_modelholder);
		else
			DrawSelOutlines(&g_edmap, g_modelholder);
		glEnable(GL_DEPTH_TEST);
		EndS();

		UseS(SHADER_COLOR2D);
		s = g_sh+g_curS;
		glUniform1f(s->slot[SSLOT_WIDTH], (float)width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)height);
		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
		//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
		DrawDrag(&g_edmap, &mvpmat, width, height, persp);
#ifdef GLDEBUG
		CHECKGLERROR();
#endif
		EndS();

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		if(g_edtool == EDTOOL_CUT && v->ldown && !g_keys[SDL_SCANCODE_LCTRL] && !g_keys[SDL_SCANCODE_RCTRL])
		{
			UseS(SHADER_COLOR2D);
			glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)width);
			glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)height);
			glUniform4f(g_sh[SHADER_COLOR2D].slot[SSLOT_COLOR], 1, 0, 0, 1);
			//glEnableVertexAttribArray(g_sh[SHADER_COLOR2D].slot[SSLOT_POSITION]);
			//glEnableVertexAttribArray(g_sh[SHADER_COLOR2D].slot[SSLOT_TEXCOORD0]);
			/*
			Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->offset, t->up));
			float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;
			Vec3f last = t->up*(float)v->lastmouse.y*screenratio + strafe*(float)-v->lastmouse.x*screenratio;
			Vec3f cur = t->up*(float)v->curmouse.y*screenratio + strafe*(float)-v->curmouse.x*screenratio;
			*/
			Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->offset, upvec));
			//last and current cursor positions relative to viewport top,left corner
			Vec3f last = OnNear(v->lastmouse.x, v->lastmouse.y, width, height, posvec, sidevec, upvec);
			Vec3f cur = OnNear(v->curmouse.x, v->curmouse.y, width, height, posvec, sidevec, upvec);
			//snap the last and current cursor positions to the nearest grid point (grid size is variable)
			last.x = SnapNearest(g_snapgrid, last.x);
			last.y = SnapNearest(g_snapgrid, last.y);
			last.z = SnapNearest(g_snapgrid, last.z);
			cur.x = SnapNearest(g_snapgrid, cur.x);
			cur.y = SnapNearest(g_snapgrid, cur.y);
			cur.z = SnapNearest(g_snapgrid, cur.z);
			//Vec4f last4 = Vec4f(last, 1);
			//Vec4f cur4 = Vec4f(cur, 1);
			//last4.transform(mvpmat);
			//cur4.transform(mvpmat);

			// get xyzw vector (vec4f) for pixel coordinates of cursor pos's
			Vec4f last4 = ScreenPos(&mvpmat, last, width, height, persp);
			Vec4f cur4 = ScreenPos(&mvpmat, cur, width, height, persp);

			float line[] = {last4.x, last4.y, 0, cur4.x, cur4.y, 0};
			//glVertexAttribPointer(g_sh[SHADER_COLOR2D].slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, line);
            glVertexPointer(3, GL_FLOAT, 0, line);
#ifdef GLDEBUG
			//CHECKGLERROR();
			CHECKGLERROR();
#endif
			glDrawArrays(GL_LINES, 0, 2);
			//CHECKGLERROR();

#ifdef GLDEBUG
			//CHECKGLERROR();
			CHECKGLERROR();
#endif
			//Log("cut draw "<<v->lastmouse.x<<","<<v->lastmouse.y<<"->"<<v->curmouse.x<<","<<v->curmouse.y<<std::endl;
			//Log("cut draw2 "<<last4.x<<","<<last4.y<<"->"<<cur4.x<<","<<cur4.y<<std::endl;
			//Log("cut draw3 "<<last.x<<","<<last.y<<","<<last.z<<"->"<<cur.x<<","<<cur.y<<","<<cur.z<<std::endl;
			//
			EndS();
		}
#ifdef GLDEBUG
		CHECKGLERROR();
#endif
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	Ortho(width, height, 1, 0, 0, 1);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif
	//glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	if(g_appmode != APPMODE_RENDERING)
	{
		RichText rlabel(t->label);
		DrawShadowedText(MAINFONT8, 0, 0, &rlabel, NULL, -1);
	}

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	glFlush();

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	//EndS();
}

ecbool ViewportLDown(int which, int relx, int rely, int width, int height)
{
	//return ecfalse;

	accum = Vec3f(0,0,0);
	g_changed = ecfalse;
	WriteH(&g_beforechange);	//write undo history
	VpWrap* v = &g_viewport[which];
	v->ldown = ectrue;
	v->lastmouse = Vec2i(relx, rely);
	v->curmouse = Vec2i(relx, rely);

	if(g_edtool != EDTOOL_NONE)
	{
		g_dragS = -1;
		g_dragV = -1;
		g_dragW = ecfalse;
		g_dragD = -1;
		return ectrue;
	}

	VpType* t = &g_vptype[v->type];

	//Log("vp["<<which<<"] l down");
	//

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	ecbool persp = ecfalse;

	if(v->type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = ectrue;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	//Vec3f viewvec = g_focus; //g_cam.view;
	//Vec3f viewvec = g_cam.view;
	Vec3f focusvec = v->focus();
	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
		//	posvec = g_cam.view + t->offset;
		//viewvec = posvec + Normalize(g_cam.view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up;
	Vec3f upvec = v->up();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m);
	//mvpmat.postmult(viewmat);
	mvpmat.postmult(viewmat);

	SelectDrag(&g_edmap, &mvpmat, width, height, relx, rely, posvec, persp);

	return ectrue;
}


ecbool ViewportRDown(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	v->rdown = ectrue;
	v->lastmouse = Vec2i(relx, rely);
	v->curmouse = Vec2i(relx, rely);

	return ectrue;
}

void CutBrushes(Plane3f cuttingp)
{
	EdMap* map = &g_edmap;

	Vec3f pop = PointOnPlane(cuttingp);
	BrushSide news0(cuttingp.normal, pop);
	BrushSide news1(Vec3f(0,0,0)-cuttingp.normal, pop);

	std::vector<Brush*> newsel;

	for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); )
	{
		Brush* b = *i;

		Brush newb0 = *b;
		Brush newb1 = *b;

		newb0.add(news0);
		newb1.add(news1);

		newb0.collapse();
		newb1.collapse();

		newb0.remaptex();
		newb1.remaptex();

		g_edmap.brush.push_back(newb0);
		std::list<Brush>::reverse_iterator j = map->brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}

		g_edmap.brush.push_back(newb1);
		j = map->brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}

		for(std::list<Brush>::iterator k=map->brush.begin(); k!=map->brush.end(); k++)
		{
			if(&*k == b)
			{
				map->brush.erase(k);
				break;
			}
		}

		i = g_selB.erase( i );
	}

	g_selB.clear();
	g_selB = newsel;
	g_sel1b = NULL;

	//g_selB.push_back( &*map->brush.rbegin() );
}

void ViewportLUp_CutBrush(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	//Vec3f viewvec = g_focus;	//g_cam.view;
	//Vec3f viewvec = g_cam.view - g_cam.pos;
	//Vec3f viewdir = v->viewdir();
	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
		//posvec = g_cam.view + t->offset;
		//viewvec = Normalize(Vec3f(0,0,0)-t->offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewdir, upv2ec));
	Vec3f sidevec = v->strafe();

	Vec3f last = OnNear(v->lastmouse.x, v->lastmouse.y, width, height, posvec, sidevec, up2vec);
	Vec3f cur = OnNear(v->curmouse.x, v->curmouse.y, width, height, posvec, sidevec, up2vec);
	last.x = SnapNearest(g_snapgrid, last.x);
	last.y = SnapNearest(g_snapgrid, last.y);
	last.z = SnapNearest(g_snapgrid, last.z);
	cur.x = SnapNearest(g_snapgrid, cur.x);
	cur.y = SnapNearest(g_snapgrid, cur.y);
	cur.z = SnapNearest(g_snapgrid, cur.z);

	if(last != cur)
	{
		LinkPrevUndo();
		Vec3f crossaxis[2];
		crossaxis[0] = Normalize(cur - last);
		crossaxis[1] = Normalize(t->offset);
		Plane3f cuttingp;
		cuttingp.normal = Normalize( Cross(crossaxis[0], crossaxis[1]) );
		cuttingp.d = PlaneDistance(cuttingp.normal, last);
		CutBrushes(cuttingp);
		//LinkLatestUndo();
	}

	g_edtool = EDTOOL_NONE;
}

void ViewportLUp_Explosion(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	//Vec3f viewvec = g_focus;	//g_cam.view;
	//Vec3f viewvec = g_cam.view - g_cam.pos;
	//Vec3f viewdir = v->viewdir();
	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
		//posvec = g_cam.view + t->offset;
		//viewvec = Normalize(Vec3f(0,0,0)-t->offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewdir, upv2ec));
	Vec3f sidevec = v->strafe();

	float aspect = fabsf((float)width / (float)height);

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;

	Vec3f vmin = g_cam.view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.view + Vec3f(extentx, extenty, extentx);

	Vec3f line[2];
	//Log("===========");
	//Log("t->offset = "<<t->offset.x<<","<<t->offset.y<<","<<t->offset.z<<std::endl;
	line[0] = OnNear(relx, rely, width, height, posvec, sidevec, up2vec);
	line[1] = line[0] - t->offset*2.0f;

	if(v->type == VIEWPORT_ANGLE45O)
	{
		line[1] = line[0] + Normalize(v->viewdir()) * (MAX_DISTANCE / 2.0f);
	}

	if(v->type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		line[0] = posvec;
		line[1] = posvec + ScreenPerspRay(relx, rely, width, height, posvec, sidevec, up2vec, v->viewdir(), FIELD_OF_VIEW) * (MAX_DISTANCE / 2.0f);
	}

	ExplodeCrater(&g_edmap, line, vmin, vmax);

	g_edtool = EDTOOL_NONE;
}

void ViewportLUp_SelectBrush(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	float aspect = fabsf((float)width / (float)height);

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;
	//Vec3f vmin = g_focus - Vec3f(extentx, extenty, extentx);
	//Vec3f vmax = g_focus + Vec3f(extentx, extenty, extentx);
	Vec3f vmin = g_cam.view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.view + Vec3f(extentx, extenty, extentx);

	//Vec3f viewvec = g_focus;	//g_cam.view;
	//Vec3f viewvec = g_cam.view - g_cam.pos;

	//Vec3f posvec = g_focus + t->offset;
	//Vec3f posvec = g_cam.pos;
	Vec3f posvec = v->pos();

	//if(v->type != VIEWPORT_ANGLE45O)
	{
		//	posvec = g_cam.view + t->offset;
		//	viewvec = Normalize(Vec3f(0,0,0)-t->offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->offset;
	//Vec3f upvec = t->up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->type != VIEWPORT_ANGLE45O)
	//	upvec = t->up;

	//Vec3f vCross = Cross(view - pos, up);
	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewvec, upvec));
	Vec3f sidevec = v->strafe();

	//Log("viewvec "<<viewvec.x<<","<<viewvec.y<<","<<viewvec.z<<std::endl;
	//Log("upvec "<<upvec.x<<","<<upvec.y<<","<<upvec.z<<std::endl;
	//Log("sidevec "<<sidevec.x<<","<<sidevec.y<<","<<sidevec.z<<std::endl;
	//

#if 0
	// pass frustum to SelectBrush to cull possible selection?
	if(v->type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSEP)
	{
		Frustum frust;
		frust.CalculateFrustum(proj, modl);
	}
#endif

	Vec3f line[2];
	//Log("===========");
	//Log("t->offset = "<<t->offset.x<<","<<t->offset.y<<","<<t->offset.z<<std::endl;
	line[0] = OnNear(relx, rely, width, height, posvec, sidevec, up2vec);
	line[1] = line[0] - t->offset*2.0f;

	if(v->type == VIEWPORT_ANGLE45O)
	{
		line[1] = line[0] + Normalize(v->viewdir()) * (MAX_DISTANCE / 2.0f);
	}

	if(v->type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		line[0] = posvec;
		line[1] = posvec + ScreenPerspRay(relx, rely, width, height, posvec, sidevec, up2vec, v->viewdir(), FIELD_OF_VIEW) * (MAX_DISTANCE / 2.0f);
	}

	SelectBrush(&g_edmap, line, vmin, vmax);
	//Log("============");
}

ecbool ViewportLUp(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	if(v->ldown)
	{
		//return ectrue;
		v->ldown = ecfalse;

		if(g_changed)
		{
			LinkPrevUndo(&g_beforechange);
			g_changed = ecfalse;
			//LinkLatestUndo();
		}

		//Log("vp["<<which<<"] l up = ecfalse");
		//

		if(!g_keys[SDL_SCANCODE_LCTRL] && !g_keys[SDL_SCANCODE_RCTRL])
		{
			if(g_edtool == EDTOOL_CUT)
			{
				ViewportLUp_CutBrush(which, relx, rely, width, height);
			}
			else if(g_edtool == EDTOOL_EXPLOSION)
			{
				ViewportLUp_Explosion(which, relx, rely, width, height);
			}
			else if(g_sel1b == NULL && g_sel1m == NULL && g_dragV < 0 && g_dragS < 0)
			{
				ViewportLUp_SelectBrush(which, relx, rely, width, height);
			}
		}
	}

	//g_sel1b = NULL;
	//g_dragV = -1;
	//g_dragS = -1;

	return ecfalse;
}

ecbool ViewportRUp(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	v->rdown = ecfalse;

	return ecfalse;
}

ecbool ViewportMousewheel(int which, int delta)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	//if(v->type == VIEWPORT_ANGLE45O)
	{
		g_zoom *= 1.0f + (float)delta / 10.0f;
		return ectrue;
	}

	return ecfalse;
}

void ViewportTranslate(int which, int dx, int dy, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	//Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->offset, t->up));

	float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;

	//Vec3f move = t->up*(float)dy*screenratio + strafe*(float)-dx*screenratio;

	//Vec3f up = Normalize(Cross(strafe, Vec3f(0,0,0)-t->offset));
	Vec3f up2 = v->up2();

	//if(v->type == VIEWPORT_ANGLE45O)
	//{
	//	strafe = g_cam.strafe;
	//	up = g_cam.up2();
	//}

	Vec3f strafe = v->strafe();

	Vec3f move = up2*(float)dy*screenratio + strafe*(float)-dx*screenratio;
	//g_cam.move(move);
	//g_focus = g_focus + move;
	g_cam.move(move);
}


void ViewportRotate(int which, int dx, int dy)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];
	/*
	Vec3f view = Normalize( Vec3f(0,0,0)-t->offset );

	Vec3f strafe = Normalize(Cross(view, t->up));

	if(MAG_VEC3F(view - t->up) <= EPSILON || MAG_VEC3F(Vec3f(0,0,0) - view - t->up) <= EPSILON)
	{
	strafe = Vec3f(1,0,0);
	t->offset = Vec3f(1000.0f/3, 1000.0f/3, 1000.0f/3);
	}*/

	//t->offset = RotateAround(t->offset, g_focus, dy / 100.0f, strafe.x, strafe.y, strafe.z);
	//t->offset = RotateAround(t->offset, g_focus, dx / 100.0f, t->up.x, t->up.y, t->up.z);

	g_cam.rotateabout(g_cam.view, dy / 100.0f, g_cam.strafe.x, g_cam.strafe.y, g_cam.strafe.z);
	g_cam.rotateabout(g_cam.view, dx / 100.0f, g_cam.up.x, g_cam.up.y, g_cam.up.z);

	//Log("rotate "<<dx/10.0f<<","<<dy/10.0f<<std::endl;
	//
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
}

//#define DRAGV_DEBUG

void DragV(Brush* b, BrushSide* s, int j, Vec3f& newv, ecbool& mergedv, ecbool* invalidv, ecbool& remove)
{
	remove = ecfalse;
	Vec3f movev = b->sharedv[ s->vindices[j] ];

	if(!mergedv)
	{
		for(int i=0; i<b->nsharedv; i++)
		{
			if(i == s->vindices[j])
				continue;

			Vec3f thisv = b->sharedv[i];
			float mag = MAG_VEC3F( newv - thisv );

			if(mag <= MERGEV_D)
			{
				newv = thisv;
				break;
			}
		}

		mergedv = ectrue;
	}

	//if(s->outline.edv.size() % 2 == 0)
	//if(ectrue)
	if(!g_keys[SDL_SCANCODE_LSHIFT] && !g_keys[SDL_SCANCODE_RSHIFT])
	{
		Vec3f farthestv = movev;
		float farthestD = 0;

		for(int i=0; i<s->ntris+2; i++)
		{
			Vec3f thisv = b->sharedv[ s->vindices[i] ];
			float mag = MAG_VEC3F( thisv - movev );

			if(mag > farthestD)
			{
				farthestD = mag;
				farthestv = thisv;
			}
		}

		//return Normalize( Cross( strafe, view - pos ) );

		Vec3f newline = Normalize(newv - farthestv);
		Vec3f crossaxis = Normalize(Cross(newline, s->plane.normal));
		s->plane.normal = Normalize(Cross(crossaxis, newline));
		s->plane.d = PlaneDistance(s->plane.normal, newv);

#ifdef DRAGV_DEBUG
		Log("crossaxis = "<<crossaxis.x<<","<<crossaxis.y<<","<<crossaxis.z<<std::endl;
		//Log("midv = "<<midv.x<<","<<midv.y<<","<<midv.z<<std::endl;
		//Log("crossaxis2 = "<<crossaxis2.x<<","<<crossaxis2.y<<","<<crossaxis2.z<<std::endl;
		
#endif
	}
	else
	{
		float farthestd[] = {0, 0};
		Vec3f farthestv[] = {movev, movev};

#ifdef DRAGV_DEBUG
		Log("--------------move v side"<<(s-b->sides)<<"------------------");
		
#endif

		for(int i=0; i<s->ntris+2; i++)
		{
			//if(i == j)
			//	continue;

			//if(invalidv[i])
			//	continue;

			Vec3f thisv = b->sharedv[ s->vindices[i] ];

			if(thisv == movev)
				continue;

			//if(thisv == newv)
			//	continue;

			float mag = MAG_VEC3F( thisv - movev );

			if(mag <= EPSILON)
				continue;

#ifdef DRAGV_DEBUG
			Log("thisv="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			Log("nearestd[0]="<<nearestd[0]<<std::endl;
			
#endif

			if(mag >= farthestd[0] || farthestd[0] <= 0)
			{
#ifdef DRAGV_DEBUG
				Log("closer vert0="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
				
#endif

				farthestd[0] = mag;
				farthestv[0] = thisv;
			}
		}

		for(int i=0; i<s->ntris+2; i++)
		{
			//if(i == j)
			//	continue;

			//if(invalidv[i])
			//	continue;

			Vec3f thisv = b->sharedv[ s->vindices[i] ];

			if(thisv == movev)
				continue;

			//if(thisv == newv)
			//	continue;

			if(thisv == farthestv[0])
				continue;

			float mag = MAG_VEC3F( thisv - movev );

			if(mag <= EPSILON)
				continue;

#ifdef DRAGV_DEBUG
			Log("thisv="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			Log("nearestd[]="<<nearestd[1]<<std::endl;
			
#endif

			if(mag >= farthestd[1] || farthestd[1] <= 0)
			{
#ifdef DRAGV_DEBUG
				Log("closer vert1="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
				
#endif

				farthestd[1] = mag;
				farthestv[1] = thisv;
			}
		}

		Vec3f tri[3];
		tri[0] = movev;
		//tri[0] = newv;
		tri[1] = farthestv[0];
		tri[2] = farthestv[1];

		if(Close(tri[0], tri[1]) || Close(tri[0], tri[2]) || Close(tri[1], tri[2]))
		{
			remove = ectrue;
			return;	//invalid side should be discarded
		}

		Vec3f norm = Normal(tri);

		//Log("tri = ("<<tri[0].x<<","<<tri[0].y<<","<<tri[0].z<<"),("<<tri[1].x<<","<<tri[1].y<<","<<tri[1].z<<"),("<<tri[2].x<<","<<tri[2].y<<","<<tri[2].z<<")");
		//Log("tri norm="<<norm.x<<","<<norm.y<<","<<norm.z<<"    plane norm="<<s->plane.normal.x<<","<<s->plane.normal.y<<","<<s->plane.normal.z<<std::endl;

		if(MAG_VEC3F( norm - s->plane.normal ) > MAG_VEC3F( Vec3f(0,0,0) - norm - s->plane.normal ))
		{
			//Log("flip vertex order YES");
			//
			Vec3f tempv = farthestv[0];
			farthestv[0] = farthestv[1];
			farthestv[1] = tempv;
		}
		else
		{
#ifdef DRAGV_DEBUG
			Log("flip vertex order NO");
			
#endif
		}

		//return Normalize( Cross( strafe, view - pos ) );

		Vec3f crossaxis = Normalize(farthestv[0] - farthestv[1]);
		Vec3f midv = (farthestv[0] + farthestv[1])/2.0f;
		Vec3f crossaxis2 = Normalize(newv - midv);

		//Vec3f crossaxis2 = Normalize(newv - s->centroid);
		//Vec3f crossaxis = Normalize(Cross( crossaxis2, s->plane.normal ));

		s->plane.normal = Normalize(Cross(crossaxis, crossaxis2));
		s->plane.d = PlaneDistance(s->plane.normal, newv);

#ifdef DRAGV_DEBUG
		Log("crossaxis = "<<crossaxis.x<<","<<crossaxis.y<<","<<crossaxis.z<<std::endl;
		Log("midv = "<<midv.x<<","<<midv.y<<","<<midv.z<<std::endl;
		Log("crossaxis2 = "<<crossaxis2.x<<","<<crossaxis2.y<<","<<crossaxis2.z<<std::endl;
		
#endif
	}
}

void Drag_Brush(Brush* b, Vec3f newmove)
{
	std::list<float> oldus;
	std::list<float> oldvs;

	for(int i=0; i<b->nsides; i++)
	{
		BrushSide* s = &b->sides[i];

		//Vec3f sharedv = b->sharedv[ s->vindices[0] ];
		float oldu = s->centroid.x*s->tceq[0].normal.x + s->centroid.y*s->tceq[0].normal.y + s->centroid.z*s->tceq[0].normal.z + s->tceq[0].d;
		float oldv = s->centroid.x*s->tceq[1].normal.x + s->centroid.y*s->tceq[1].normal.y + s->centroid.z*s->tceq[1].normal.z + s->tceq[1].d;
		//Vec3f axis = s->plane.normal;
		//float radians = DEGTORAD(degrees);
		//s->tceq[0].normal = Rotate(s->tceq[0].normal, radians, axis.x, axis.y, axis.z);
		//s->tceq[1].normal = Rotate(s->tceq[1].normal, radians, axis.x, axis.y, axis.z);

		oldus.push_back(oldu);
		oldvs.push_back(oldv);

		Vec3f pop = PointOnPlane(s->plane);
		pop = pop - newmove;
		s->plane.d = PlaneDistance(s->plane.normal, pop);

	}

	b->collapse();

	std::list<float>::iterator oldu = oldus.begin();
	std::list<float>::iterator oldv = oldvs.begin();

	for(int i=0; i<b->nsides; i++, oldu++, oldv++)
	{
		BrushSide* s = &b->sides[i];

		//Vec3f newsharedv = b->sharedv[ s->vindices[0] ];
		float newu = s->centroid.x*s->tceq[0].normal.x + s->centroid.y*s->tceq[0].normal.y + s->centroid.z*s->tceq[0].normal.z + s->tceq[0].d;
		float newv = s->centroid.x*s->tceq[1].normal.x + s->centroid.y*s->tceq[1].normal.y + s->centroid.z*s->tceq[1].normal.z + s->tceq[1].d;
		float changeu = newu - *oldu;
		float changev = newv - *oldv;
		s->tceq[0].d -= changeu;
		s->tceq[1].d -= changev;
	}

	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
}

void Drag_BrushVert(Brush* b, Vec3f newmove)
{
	ecbool* invalidv = new ecbool[b->nsharedv];
	for(int i=0; i<b->nsharedv; i++)
		invalidv[i] = ecfalse;

	Vec3f movev = b->sharedv[ g_dragV ];
	Vec3f newv = movev - newmove;
	ecbool mergedv = ecfalse;

	for(int i=0; i<b->nsides; i++)
	{
		BrushSide* s = &b->sides[i];
		for(int j=0; j<s->ntris+2; j++)
		{
			if(s->vindices[j] == g_dragV)
			{
				ecbool remove;
				DragV(b, s, j, newv, mergedv, invalidv, remove);

				//s->gentexeq();
				s->remaptex();
				b->prunev(invalidv);

				//if(invalidv[ s->vindices[j] ])
				//	g_dragV = -1;

				//if(remove)
				{
					//	b->removeside(i);
					//	i--;
					//	break;
				}
			}
		}
	}
	delete [] invalidv;
	b->collapse();
	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
}

void Drag_BrushSide(Brush* b, Vec3f newmove)
{
	BrushSide* s = &b->sides[g_dragS];
	Vec3f pop = PointOnPlane(s->plane);
	pop = pop - newmove;
	s->plane.d = PlaneDistance(s->plane.normal, pop);
	b->collapse();
	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->offset);
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);
}

void Drag_BrushDoor(Brush* b, Vec3f newmove)
{
	EdDoor* door = b->door;

	if(g_dragD == DRAG_DOOR_POINT)
	{
		door->point = door->point - newmove;
	}
	else if(g_dragD == DRAG_DOOR_AXIS)
	{
		door->axis = door->axis - newmove;
	}
}

void Drag_Model(ModelHolder* mh, Vec3f newmove)
{
	mh->translation = mh->translation + newmove;
	mh->absmin = mh->absmin + newmove;
	mh->absmax = mh->absmax + newmove;
}

void Drag_ModelSide(ModelHolder* mh, Vec3f newmove)
{
	int side = g_dragS;

	if(side < 0)
		return;

	Vec3f scalechange(1,1,1);
	Vec3f span = mh->absmax - mh->absmin;

	if(span.x <= 0.0f)
		span.x = 1;
	if(span.y <= 0.0f)
		span.y = 1;
	if(span.z <= 0.0f)
		span.z = 1;

	if(side == DRAG_TOP)
	{
		scalechange.y = 1 + (newmove.y / span.y);
		mh->translation.y = mh->translation.y + newmove.y/2;
	}
	else if(side == DRAG_BOTTOM)
	{
		scalechange.y = 1 - (newmove.y / span.y);
		mh->translation.y = mh->translation.y + newmove.y/2;
	}
	else if(side == DRAG_LEFT)
	{
		scalechange.x = 1 - (newmove.x / span.x);
		mh->translation.x = mh->translation.x + newmove.x/2;
	}
	else if(side == DRAG_RIGHT)
	{
		scalechange.x = 1 + (newmove.x / span.x);
		mh->translation.x = mh->translation.x + newmove.x/2;
	}
	else if(side == DRAG_NEAR)
	{
		scalechange.z = 1 + (newmove.z / span.z);
		mh->translation.z = mh->translation.z + newmove.z/2;
	}
	else if(side == DRAG_FAR)
	{
		scalechange.z = 1 - (newmove.z / span.z);
		mh->translation.z = mh->translation.z + newmove.z/2;
	}

	//char msg[1024];
	//sprintf(msg, "newmove %f,%f,%f \n span %f,%f,%f \n scale cahge %f,%f,%f", newmove.x, newmove.y, newmove.z, span.x, span.y, span.z, scalechange.x, scalechange.y, scalechange.z);
	//MessageBox(g_hWnd, msg, "asd", NULL);

	mh->scale = mh->scale * scalechange;

	if(mh->scale.x <= 0.0f)
		mh->scale.x = 1;
	if(mh->scale.y <= 0.0f)
		mh->scale.y = 1;
	if(mh->scale.z <= 0.0f)
		mh->scale.z = 1;

	mh->retransform();
}

void Drag(int which, int dx, int dy, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->type];

	//Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->offset, t->up));
	Vec3f strafe = v->strafe();
	Vec3f up2 = v->up2();

	float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;

	Vec3f move = up2*(float)dy*screenratio + strafe*(float)-dx*screenratio;
	Vec3f newmove;
	newmove.x = Snap(g_snapgrid, move.x + accum.x);
	newmove.y = Snap(g_snapgrid, move.y + accum.y);
	newmove.z = Snap(g_snapgrid, move.z + accum.z);
	accum = accum + move - newmove;

	//Log("move = "<<move.x<<","<<move.y<<","<<move.z<<std::endl;
	//Log("newmove = "<<newmove.x<<","<<newmove.y<<","<<newmove.z<<std::endl;
	//Log("accum = "<<accum.x<<","<<accum.y<<","<<accum.z<<std::endl;
	//

	if(newmove != Vec3f(0,0,0))
		g_changed = ectrue;

	if(g_sel1b)
	{
		Brush* b = g_sel1b;

		if(g_dragW)
		{
			Drag_Brush(b, newmove);
		}
		else if(g_dragV >= 0)
		{
			Drag_BrushVert(b, newmove);
		}
		else if(g_dragS >= 0)
		{
			Drag_BrushSide(b, newmove);
		}
		else if(g_dragD >= 0)
		{
			Drag_BrushDoor(b, newmove);
		}
	}
	else if(g_sel1m)
	{
		ModelHolder* mh = g_sel1m;

		//MessageBox(g_hWnd, "drag m", "asd", NULL);

		Vec3f modelnewmove = Vec3f(0,0,0) - newmove;

		if(g_dragW)
		{
			//MessageBox(g_hWnd, "drag m w", "asd", NULL);
			Drag_Model(mh, modelnewmove);
		}
		else if(g_dragS >= 0)
		{
			Drag_ModelSide(mh, modelnewmove);
		}
	}
}

ecbool ViewportMousemove(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];

	if(v->ldown)
	{
		//Log("vp["<<which<<"] down mouse move l");
		//

		if(g_sel1b || g_sel1m)
		{
			Drag(which, relx - v->lastmouse.x, rely - v->lastmouse.y, width, height);
		}
		else if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		{
			ViewportTranslate(which, relx - v->lastmouse.x, rely - v->lastmouse.y, width, height);
		}

		if((g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL]) || g_edtool != EDTOOL_CUT || !v->ldown)
			v->lastmouse = Vec2i(relx, rely);
		v->curmouse = Vec2i(relx, rely);
		return ectrue;
	}
	else if(v->rdown)
	{
		if(v->type == VIEWPORT_ANGLE45O)
		{
			ViewportRotate(which, relx - v->lastmouse.x, rely - v->lastmouse.y);
		}
		v->lastmouse = Vec2i(relx, rely);
		v->curmouse = Vec2i(relx, rely);
		return ectrue;
	}

	if((g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL]) || g_edtool != EDTOOL_CUT || (!v->ldown && !v->rdown))
		v->lastmouse = Vec2i(relx, rely);

	v->curmouse = Vec2i(relx, rely);

	return ecfalse;
}
