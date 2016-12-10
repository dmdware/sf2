










#include "../platform.h"
#include "../render/shader.h"
#include "draw2d.h"
#include "../utils.h"
#include "../window.h"
#include "../sim/map.h"
#include "../math/isomath.h"

//float g_basedepth = 7*256*256+180*256+173;
float g_basedepth = (28 + 256 * 200 + 256 * 256 * 7);

void DrawImage(unsigned int tex, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom, float *crop)
{
	Shader *s;
	float v[2*6], t[2*6], newleft, newtop, newright, newbottom, newtexleft, newtextop, newtexright, newtexbottom;

	if(crop[0] >= crop[2])
		return;
	if(crop[1] >= crop[3])
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	s = g_sh+g_curS;
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	newleft = left;
	newtop = top;
	newright = right;
	newbottom = bottom;
	newtexleft = texleft;
	newtextop = textop;
	newtexright = texright;
	newtexbottom = texbottom;

	if(newleft < crop[0])
	{
		newtexleft = texleft+(crop[0]-left)*(texright-texleft)/(right-left);
		newleft = crop[0];
	}
	else if(newleft > crop[2])
		return;

	if(newright < crop[0])
		return;
	else if(newright > crop[2])
	{
		newtexright = texleft+(crop[2]-left)*(texright-texleft)/(right-left);
		newright = crop[2];
	}

	if(newtop < crop[1])
	{
		newtextop = textop+(crop[1]-top)*(texbottom-textop)/(bottom-top);
		newtop = crop[1];
	}
	else if(newtop > crop[3])
		return;

	if(newbottom < crop[1])
		return;
	else if(newbottom > crop[3])
	{
		newtexbottom = textop+(crop[3]-top)*(texbottom-textop)/(bottom-top);
		newbottom = crop[3];
	}

	v[0] = newleft;
	v[1] = newtop;
	v[2] = newright;
	v[3] = newtop;
	v[4] = newright;
	v[5] = newbottom;
	v[6] = newright;
	v[7] = newbottom;
	v[8] = newleft;
	v[9] = newbottom;
	v[10] = newleft;
	v[11] = newtop;

	t[0] = newtexleft;
	t[1] = newtextop;
	t[2] = newtexright;
	t[3] = newtextop;
	t[4] = newtexright;
	t[5] = newtexbottom;
	t[6] = newtexright;
	t[7] = newtexbottom;
	t[8] = newtexleft;
	t[9] = newtexbottom;
	t[10] = newtexleft;
	t[11] = newtextop;

#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &t[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Rotate2(float cx, float cy, float angle, float* p)
{
  float s = sin(angle);
  float c = cos(angle);

  // translate point back to origin:
  p[0] -= cx;
  p[1] -= cy;

  // rotate point
  float xnew = p[0] * c - p[1] * s;
  float ynew = p[0] * s + p[1] * c;

  // translate point back:
  p[0] = xnew + cx;
  p[1] = ynew + cy;
}

void DrawSphericalBlend(unsigned int difftex, unsigned int depthtex, unsigned int renderdepthtex, unsigned int renderfb, float basedepth,
						float cx, float cy,
						float pixradius, float angle,
						float texleft, float textop, float texright, float texbottom)
{
	Shader *s;
	float v[2*6], t[2*6];

	cy = g_height - cy;
	
	float top = cy - pixradius;
	float bottom = cy + pixradius;
	float left = cx - pixradius;
	float right = cx + pixradius;

	float topleft[2] = {left,top};
	float bottomleft[2] = {left,bottom};
	float topright[2] = {right,top};
	float bottomright[2] = {right,bottom};
	
	Rotate2(cx, cy, angle, topleft);
	Rotate2(cx, cy, angle, bottomleft);
	Rotate2(cx, cy, angle, topright);
	Rotate2(cx, cy, angle, bottomright);

#if 0
	v[0] = left;
	v[1] = top;
	v[2] = right;
	v[3] = top;
	v[4] = right;
	v[5] = bottom;
	v[6] = right;
	v[7] = bottom;
	v[8] = left;
	v[9] = bottom;
	v[10] = left;
	v[11] = top;
#else
	v[0] = topleft[0];
	v[1] = topleft[1];
	v[2] = topright[0];
	v[3] = topright[1];
	v[4] = bottomright[0];
	v[5] = bottomright[1];
	v[6] = bottomright[0];
	v[7] = bottomright[1];
	v[8] = bottomleft[0];
	v[9] = bottomleft[1];
	v[10] = topleft[0];
	v[11] = topleft[1];
#endif

	t[0] = texleft;
	t[1] = textop;
	t[2] = texright;
	t[3] = textop;
	t[4] = texright;
	t[5] = texbottom;
	t[6] = texright;
	t[7] = texbottom;
	t[8] = texleft;
	t[9] = texbottom;
	t[10] = texleft;
	t[11] = textop;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLint oldfb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);

	
	Vec3f ray, point;
	IsoToCart(g_scroll + Vec2i(g_width,g_height), &ray, &point);
	ray = Normalize(ray);
	float highz = TILE_RISE * 15;
	float offz = highz - point.z;
	float ratioz = offz / ray.z;
	point = point + ray * ratioz;
	float neardepth = Dot( ray, point );
	//neardepth = 0;
	//Log("dpeth: "<<basedepth<<" neardepth:"<<neardepth<<" final:"<<(basedepth-neardepth));
	
	const float speddepth = SPEDDEPTH;

	UseS(SHADER_PARTICLEBLEND);
	//UseS(SHADER_ORTHO);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_height);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));
	//glUniform1f(s->slot[SSLOT_BASEDEPTH], 0);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	glUniform1i(s->slot[SSLOT_SCREENDEPTH], 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

#if 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);
#endif
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &t[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	
	
	EndS();

#if 0
	glEnable(GL_DEPTH_TEST);
	//glBindFramebuffer(GL_FRAMEBUFFER, renderfb);

	UseS(SHADER_PARTICLEBLENDDEPTH);
	//UseS(SHADER_ORTHO);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)2048);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)2048);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	glUniform1i(s->slot[SSLOT_SCREENDEPTH], 2);

	s = g_sh+g_curS;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

#if 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);
#endif
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &t[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	EndS();
	glDisable(GL_DEPTH_TEST);
#endif
}

void DrawDepth(unsigned int difftex, unsigned int depthtex, unsigned int renderdepthtex, unsigned int renderfb, int basedepth, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
	Shader *s;
	float v[2*6], t[2*6];

	v[0] = left;
	v[1] = top;
	v[2] = right;
	v[3] = top;
	v[4] = right;
	v[5] = bottom;
	v[6] = right;
	v[7] = bottom;
	v[8] = left;
	v[9] = bottom;
	v[10] = left;
	v[11] = top;

	//flip vertically because OpenGL has 0->1 upwards
	//texbottom = 1.0f - texbottom;
	//textop = 1.0f - textop;

	t[0] = texleft;
	t[1] = textop;
	t[2] = texright;
	t[3] = textop;
	t[4] = texright;
	t[5] = texbottom;
	t[6] = texright;
	t[7] = texbottom;
	t[8] = texleft;
	t[9] = texbottom;
	t[10] = texleft;
	t[11] = textop;
	
#if 1
	//actually, this one could also be done using the normal depth test
	//glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, renderfb);

	//UseS(SHADER_DEEPORTHO);
	//UseS(SHADER_ORTHO);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_width);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	glUniform1i(s->slot[SSLOT_SCREENDEPTH], 2);

	s = g_sh+g_curS;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

#if 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);
#endif

#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &t[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//EndS();
	
#endif

	return;

#if 0
	//flip vertically because OpenGL has 0->1 upwards
	texbottom = 1.0f - texbottom;
	textop = 1.0f - textop;

	t[0] = texleft;
	t[1] = textop;
	t[2] = texright;
	t[3] = textop;
	t[4] = texright;
	t[5] = texbottom;
	t[6] = texright;
	t[7] = texbottom;
	t[8] = texleft;
	t[9] = texbottom;
	t[10] = texleft;
	t[11] = textop;
#endif

	//to avoid using the render depth texture while writing to it, use the normal depth test
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, renderfb);

	UseS(SHADER_DEEP);
	//UseS(SHADER_ORTHO);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)2048);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)2048);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	//glUniform1i(s->slot[SSLOT_SCREENDEPTH], 2);

	s = g_sh+g_curS;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

#if 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);
#endif
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &t[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	EndS();

	glDisable(GL_DEPTH_TEST);
}

void DrawDeep(unsigned int difftex, unsigned int depthtex, unsigned int teamtex, int basedepth, float baseelev,
			  unsigned int renderdepthtex, unsigned int renderfb,
			  float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
	Shader *s;
	float v[2*6], t[2*6];
	
	//make starting depth 0 in the bottom, high corner of the scrolled view area

	GLint oldfb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);

	Vec3f ray, point;
	IsoToCart(g_scroll + Vec2i(g_width,g_height), &ray, &point);
	ray = Normalize(ray);
	float highz = TILE_RISE * 15;
	float offz = highz - point.z;
	float ratioz = offz / ray.z;
	point = point + ray * ratioz;
	float neardepth = Dot( ray, point );
	//neardepth = 0;
	//Log("dpeth: "<<basedepth<<" neardepth:"<<neardepth<<" final:"<<(basedepth-neardepth));
	
	const float speddepth = SPEDDEPTH;


	//UseS(SHADER_PARTICLEBLENDDEPTH);
//	glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
#if 00
	s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glDisable(GL_DEPTH_TEST);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_width);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));
	glUniform1f(s->slot[SSLOT_BASEELEV], baseelev);

	DrawDepth(difftex,
		depthtex,
		renderdepthtex,
		renderfb,
		basedepth,
		left,top,right,bottom,
		texleft,textop,texright,texbottom);
#endif
#if 01
	UseS(SHADER_DEEPTEAMELEV);
	//glBindFramebuffer(GL_FRAMEBUFFER, oldfb);

	s = g_sh+g_curS;
	
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_width);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));
	glUniform1f(s->slot[SSLOT_BASEELEV], baseelev);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	glUniform1i(s->slot[SSLOT_SCREENDEPTH], 5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, teamtex);
	glUniform1i(s->slot[SSLOT_OWNERMAP], 2);
	
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, elevtex);
	//glUniform1i(s->slot[SSLOT_ELEVMAP], 3);

#ifndef PLATFORM_MOBILE
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);


	//glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - neardepth);

#if 0
	static float nearest = basedepth - neardepth;
	static float farthest = basedepth - neardepth;
	
	if(basedepth - neardepth < nearest)
		nearest = basedepth - neardepth;
	if(basedepth - neardepth > farthest)
		farthest = basedepth - neardepth;

	//Log("farthest: "<<farthest<<"      nearest: "<<nearest);
#endif

	//glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - speddepth);
#endif

	v[0] = left;
	v[1] = top;
	v[2] = right;
	v[3] = top;
	v[4] = right;
	v[5] = bottom;
	v[6] = right;
	v[7] = bottom;
	v[8] = left;
	v[9] = bottom;
	v[10] = left;
	v[11] = top;

	t[0] = texleft;
	t[1] = textop;
	t[2] = texright;
	t[3] = textop;
	t[4] = texright;
	t[5] = texbottom;
	t[6] = texright;
	t[7] = texbottom;
	t[8] = texleft;
	t[9] = texbottom;
	t[10] = texleft;
	t[11] = textop;
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, v);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, t);
#endif
	//crash here
	//glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindFramebuffer(GL_FRAMEBUFFER, oldfb);
	glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
}

void DrawDeep2(unsigned int difftex, unsigned int depthtex, unsigned int teamtex, unsigned int elevtex, 
			   unsigned int renderdepthtex, unsigned int renderfb,
			   int basedepth, float baseelev,
			   float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
	Shader *s;
	float v[2*6], t[2*6];

	//make starting depth 0 in the bottom, high corner of the scrolled view area

	GLint oldfb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);

	Vec3f ray, point;
	IsoToCart(g_scroll + Vec2i(g_width,g_height), &ray, &point);
	ray = Normalize(ray);
	float highz = TILE_RISE * 15;
	float offz = highz - point.z;
	float ratioz = offz / ray.z;
	point = point + ray * ratioz;
	float neardepth = Dot( ray, point );
	//neardepth = 0;
	//Log("dpeth: "<<basedepth<<" neardepth:"<<neardepth<<" final:"<<(basedepth-neardepth));
	
	const float speddepth = SPEDDEPTH;

	//UseS(SHADER_PARTICLEBLENDDEPTH);
	//glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
#if 0
	s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glDisable(GL_DEPTH_TEST);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_width);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));
	glUniform1f(s->slot[SSLOT_MIND], (float)MIN_DISTANCE);
	glUniform1f(s->slot[SSLOT_MAXD], (float)MAX_DISTANCE);

	DrawDepth(difftex,
		depthtex,
		renderdepthtex,
		renderfb,
		basedepth,
		left,top,right,bottom,
		texleft,textop,texright,texbottom);
#endif
#if 01
	//EndS();
	UseS(SHADER_DEEPTEAMELEV);
	s = g_sh+g_curS;
	
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	//glUniform1f(s->slot[SSLOT_WIDTH], (float)512);
	//glUniform1f(s->slot[SSLOT_HEIGHT], (float)512);
	glUniform1f(s->slot[SSLOT_SCREENMAPWIDTH], (float)g_height);
	glUniform1f(s->slot[SSLOT_SCREENMAPHEIGHT], (float)g_width);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));
	glUniform1f(s->slot[SSLOT_BASEELEV], baseelev);
	glUniform1f(s->slot[SSLOT_MIND], (float)MIN_DISTANCE);
	glUniform1f(s->slot[SSLOT_MAXD], (float)MAX_DISTANCE);
	
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, renderdepthtex);
	glUniform1i(s->slot[SSLOT_SCREENDEPTH], 5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, difftex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, teamtex);
	glUniform1i(s->slot[SSLOT_OWNERMAP], 2);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, elevtex);
	glUniform1i(s->slot[SSLOT_ELEVMAP], 3);

#ifndef PLATFORM_MOBILE
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthtex);
	glUniform1i(s->slot[SSLOT_SPRITEDEPTH], 1);

	//glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - neardepth);

#if 0
	static float nearest = basedepth - neardepth;
	static float farthest = basedepth - neardepth;
	
	if(basedepth - neardepth < nearest)
		nearest = basedepth - neardepth;
	if(basedepth - neardepth > farthest)
		farthest = basedepth - neardepth;

	//Log("farthest: "<<farthest<<"      nearest: "<<nearest);
#endif

	//glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - speddepth);
#endif

	v[0] = left;
	v[1] = top;
	v[2] = right;
	v[3] = top;
	v[4] = right;
	v[5] = bottom;
	v[6] = right;
	v[7] = bottom;
	v[8] = left;
	v[9] = bottom;
	v[10] = left;
	v[11] = top;

	t[0] = texleft;
	t[1] = textop;
	t[2] = texright;
	t[3] = textop;
	t[4] = texright;
	t[5] = texbottom;
	t[6] = texright;
	t[7] = texbottom;
	t[8] = texleft;
	t[9] = texbottom;
	t[10] = texleft;
	t[11] = textop;
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, v);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, t);
#endif
	//crash here
	//glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindFramebuffer(GL_FRAMEBUFFER, oldfb);
	glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
}


void DrawDeepColor(float r, float g, float b, float a, float *v, int nv, GLenum mode)
{
	glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	//glLineWidth(5.0f);

	Shader *s;
	//float v[6];

	s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], r, g, b, a);
	
	//make starting depth 0 in the bottom, high corner of the scrolled view area

	Vec3f ray, point;
	IsoToCart(g_scroll + Vec2i(g_width,g_height), &ray, &point);
	ray = Normalize(ray);
	float highz = TILE_RISE * 15;
	float offz = highz - point.z;
	float ratioz = offz / ray.z;
	point = point + ray * ratioz;
	float neardepth = Dot( ray, point );

	//const float speddepth = SPEDDEPTH;

	//glUniform1f(s->slot[SSLOT_BASEDEPTH], basedepth - (speddepth + neardepth));

	for(int vi=2; vi<nv*3; vi+=3)
		v[vi] -= (neardepth);

#if 0
	v[0] = x1;
	v[1] = y1;
	v[2] = d1 - (SPEDDEPTH + neardepth);
	v[3] = x2;
	v[4] = y2;
	v[5] = d2 - (SPEDDEPTH + neardepth);
#endif
	
#ifdef PLATFORM_GL14
	glVertexPointer(3, GL_FLOAT, 0, v);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
#endif
	
	glDrawArrays(mode, 0, nv);

	//glLineWidth(1.0f);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
}

void DrawSquare(float r, float g, float b, float a, float left, float top, float right, float bottom, float *crop)
{
	Shader *s;
	float v[2*6], newleft, newtop, newright, newbottom;

	if(crop[0] >= crop[2])
		return;
	if(crop[1] >= crop[3])
		return;

	s = g_sh+g_curS;

	glUniform4f(s->slot[SSLOT_COLOR], r, g, b, a);

	newleft = left;
	newtop = top;
	newright = right;
	newbottom = bottom;

	if(newleft < crop[0])
		newleft = crop[0];
	else if(newleft > crop[2])
		return;

	if(newright < crop[0])
		return;
	else if(newright > crop[2])
		newright = crop[2];

	if(newtop < crop[1])
		newtop = crop[1];
	else if(newtop > crop[3])
		return;

	if(newbottom < crop[1])
		return;
	else if(newbottom > crop[3])
		newbottom = crop[3];

	v[0] = newleft;
	v[1] = newtop;
	v[2] = newright;
	v[3] = newtop;
	v[4] = newright;
	v[5] = newbottom;
	v[6] = newright;
	v[7] = newbottom;
	v[8] = newleft;
	v[9] = newbottom;
	v[10] = newleft;
	v[11] = newtop;
	
#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//2015/10/27 cropping now works correctly for lines
void DrawLine(float r, float g, float b, float a, float x1, float y1, float x2, float y2, float *crop)
{
	float v[2*2], dx, dy, slope;
	Shader *s;

	//ax+by+c=0

	if(crop[0] >= crop[2])
		return;
	if(crop[1] >= crop[3])
		return;

	dx = x2-x1;
	dy = y2-y1;
	slope = dy/dx;

	if(x1 < crop[0])
	{
		dx = crop[0] - x1;
		dy = dx * slope;
		x1 += dx;
		y1 += dy;
	}

	//TODO fix droplist closing on left mouse button release after using slider first time
	//TODO different currencies
	
	if(x2 < crop[0])
	{
		dx = crop[0] - x2;
		dy = dx * slope;
		x2 += dx;
		y2 += dy;
	}
	
	if(y1 < crop[1])
	{
		dy = crop[1] - y1;
		dx = dy / slope;
		x1 += dx;
		y1 += dy;
	}
	
	if(y2 < crop[1])
	{
		dy = crop[1] - y2;
		dx = dy / slope;
		x2 += dx;
		y2 += dy;
	}
	
	if(x1 > crop[2])
	{
		dx = x1 - crop[2];
		dy = dx * slope;
		x1 -= dx;
		y1 -= dy;
	}
	
	if(x2 > crop[2])
	{
		dx = x2 - crop[2];
		dy = dx * slope;
		x2 -= dx;
		y2 -= dy;
	}

	if(y1 > crop[3])
	{
		dy = y1 - crop[3];
		dx = dy / slope;
		x1 -= dx;
		y1 -= dy;
	}
	
	if(y2 > crop[3])
	{
		dy = y2 - crop[3];
		dx = dy / slope;
		x2 -= dx;
		y2 -= dy;
	}

	s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], r, g, b, a);

	v[0] = x1;
	v[1] = y1;
	v[2] = x2;
	v[3] = y2;

#ifdef PLATFORM_GL14
	glVertexPointer(2, GL_FLOAT, 0, v);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*0, &v[0]);
#endif
	
	glDrawArrays(GL_LINES, 0, 2);
}
