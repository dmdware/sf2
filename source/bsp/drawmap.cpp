

#include "../math/vec3f.h"
#include "../math/plane3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../render/model2.h"
#include "../render/vertexarray.h"
#include "brushside.h"
#include "brush.h"
#include "../save/savemap.h"
#include "../render/shader.h"
#include "../debug.h"
#include "../math/camera.h"
#include "../render/shadow.h"
#include "../texture.h"
#include "../save/edmap.h"
#include "../save/saveedm.h"

#if 0
void DrawMap(Map* map)
{
#if 0
	Shader* s = g_sh+g_curS;

	//glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_camproj.m);
	//glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_camview.m);

	Matrix modelmat;
	modelmat.reset();
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
	CheckGLError(__FILE__, __LINE__);

	Matrix mvp;
	mvp.set(g_camproj.m);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);
	CheckGLError(__FILE__, __LINE__);

	Matrix modelview;
	modelview.set(g_camview.m);
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m);
	CheckGLError(__FILE__, __LINE__);

#if 1
	//modelview.set(g_camview.m);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m);
#endif

	//g_bsp.RenderLevel(g_cam.pos);
#else
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	const ecbool showsky = ecfalse;

	Shader* s = g_sh+g_curS;

	//glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_camproj.m);
	//glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_camview.m);

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
    CheckGLError(__FILE__, __LINE__);

	Matrix mvp;
	mvp.set(g_camproj.m);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);
			CheckGLError(__FILE__, __LINE__);

	Matrix modelview;
    modelview.set(g_camview.m);
    modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m);
			CheckGLError(__FILE__, __LINE__);

#if 1
	//modelview.set(g_camview.m);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
    Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m);
#endif
	
			CheckGLError(__FILE__, __LINE__);

	//for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	for(int bi=0; bi<map->nbrush; ++bi)
	{
		Brush* b = &map->brush[bi];
		Texture* t = &g_texture[b->texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* side = &b->sides[i];
			/*
			CreateTex(side->diffusem, "gui/frame.jpg", ecfalse);
			side->diffusem = g_texindex;*/
			side->usedifftex();
			CheckGLError(__FILE__, __LINE__);
			side->usespectex();
			CheckGLError(__FILE__, __LINE__);
			side->usenormtex();
			CheckGLError(__FILE__, __LINE__);
			side->useteamtex();

			CheckGLError(__FILE__, __LINE__);
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", ecfalse);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &side->drawva;
			glActiveTexture(GL_TEXTURE0);

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
#endif

#ifdef PLATFORM_GL32
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
#endif
			
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
			CheckGLError(__FILE__, __LINE__);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			CheckGLError(__FILE__, __LINE__);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			CheckGLError(__FILE__, __LINE__);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif

			CheckGLError(__FILE__, __LINE__);
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
			CheckGLError(__FILE__, __LINE__);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
#endif
}
#endif
#if 00
void DrawMapDepth(Map* map)
{
#if 0
	g_bsp.RenderLevel(g_cam.pos);
#else
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	const ecbool showsky = ecfalse;

	Shader* shader = &g_sh[g_curS];

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);

	//for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	for(int bi=0; bi<map->nbrush; ++bi)
	{
		Brush* b = &map->brush[bi];
		Texture* t = &g_texture[b->texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* side = &b->sides[i];
			/*
			CreateTex(side->diffusem, "gui/frame.jpg", ecfalse);
			side->diffusem = g_texindex;*/
			side->usedifftex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", ecfalse);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(shader->slot[SSLOT_TEXTURE0], 0);

			VertexArray* va = &side->drawva;

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#endif
			
#ifdef PLATFORM_GL32
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#endif

#ifdef PLATFORM_GL14
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif
			
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
#endif
}
#endif