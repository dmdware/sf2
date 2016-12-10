

#include "sesim.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/3dmath.h"
#include "../math/vec4f.h"
#include "../math/plane3f.h"
#include "seviewport.h"
#include "../gui/gui.h"
#include "../sim/sim.h"
#include "segui.h"
#include "../sim/door.h"
#include "../save/modelholder.h"
#include "../save/compilemap.h"
#include "../tool/rendersprite.h"
#include "../render/shadow.h"

Brush g_copyB;
ModelHolder g_copyM;
int g_edtool = EDTOOL_NONE;
ecbool g_leads[LEADS_DIRS];

// draw selected brushes filled bg
void DrawFilled(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_sh[g_curS];
	glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.3f, 0.9f, 0.5f);

	for(int i=0; i<g_selB.size(); i++)
	{
		Brush* b = g_selB[i];

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* side = &b->sides[j];

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->drawva.vertices);
            glVertexPointer(3, GL_FLOAT, 0, side->drawva.vertices);

			glDrawArrays(GL_TRIANGLES, 0, side->drawva.numverts);
		}
	}

#if 1
	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;
		//Model* m = &g_model[pmh->modeli];
		Model2* m = &pmh->model;

#if 0	//TODO
		int maxframes = m->ms3d.totalFrames;

		DrawVA(&pmh->frames[ g_renderframe % maxframes ], pmh->translation);
#else

		int frames = 1;

		if(m->pScene->mNumAnimations > 0)
		{
			double duration = m->pScene->mAnimations[0]->mDuration;
			double tickspersec = (float)(m->pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			double dframes = duration / tickspersec;
			if(dframes >= 1.0f)
				frames = (int)dframes;
		}
		
		//pmh->model.Render(g_renderframe % frames, pmh->translation, pmh->modeli, pmh->scale, pmh->rotationmat);
		pmh->model.Render(g_renderframe, pmh->translation, pmh->modeli, pmh->scale, pmh->rotationmat);
#endif
	}
#endif
}

// draw brush outlines
void DrawOutlines(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_sh[g_curS];

#if 1
	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);

	Matrix mvpmat;
	mvpmat.set(g_camproj.m);
	mvpmat.postmult(g_camview);
    glUniformMatrix4fv(shader->slot[SSLOT_MVP], 1, 0, mvpmat.m);
#endif

	//glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, 0.75f);

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* side = &b->sides[i];

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);

			float alpha = 1.0f - Dot(side->centroid - g_cam.view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

			glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->outline.drawoutva);
            glVertexPointer(3, GL_FLOAT, 0, side->outline.drawoutva);

			glDrawArrays(GL_LINE_STRIP, 0, side->outline.edv.size());
		}
	}

#if 1
	for(std::list<ModelHolder>::iterator mhiter = modelholder.begin(); mhiter != modelholder.end(); mhiter++)
	{
		ModelHolder* pmh = &*mhiter;

		float topverts[] =
		{
			//top
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//top far right
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//top near right
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//top near left
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
		};

		float bottomverts[] =
		{
			//bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//bottom far right
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//bottom near right
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//bottom near left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
		};

		float leftverts[] =
		{
			//left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//left top far
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//left top near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//left bottom near
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
		};

		float rightverts[] =
		{
			//right
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//right top far
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//right top near
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//right bottom near
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
		};

		float nearverts[] =
		{
			//near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//near left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//near right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//near right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
		};

		float farverts[] =
		{
			//far
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//far left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//far right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//far right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
		};

		float alpha = 1.0f - Dot((pmh->absmin+pmh->absmax)/2.0f - g_cam.view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

		glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
        glVertexPointer(3, GL_FLOAT, 0, topverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
        glVertexPointer(3, GL_FLOAT, 0, farverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
#endif
}

// draw selected brush outlines
void DrawSelOutlines(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_sh[g_curS];
	glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, 0.75f);

#if 1
	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);

	Matrix mvpmat;
	mvpmat.set(g_camproj.m);
	mvpmat.postmult(g_camview);
    glUniformMatrix4fv(shader->slot[SSLOT_MVP], 1, 0, mvpmat.m);
#endif

	for(std::vector<Brush*>::iterator bi=g_selB.begin(); bi!=g_selB.end(); bi++)
	{
		Brush* b = *bi;

		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* side = &b->sides[i];

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->outline.drawoutva);
            glVertexPointer(3, GL_FLOAT, 0, side->outline.drawoutva);

			glDrawArrays(GL_LINE_STRIP, 0, side->outline.edv.size());
		}
	}

#if 1
	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		float topverts[] =
		{
			//top
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//top far right
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//top near right
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//top near left
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
		};

		float bottomverts[] =
		{
			//bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//bottom far right
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//bottom near right
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//bottom near left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
		};

		float leftverts[] =
		{
			//left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//left top far
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//left top near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//left bottom near
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
		};

		float rightverts[] =
		{
			//right
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//right top far
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//right top near
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//right bottom near
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
		};

		float nearverts[] =
		{
			//near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//near left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//near right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//near right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
		};

		float farverts[] =
		{
			//far
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//far left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//far right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//far right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
		};

		float alpha = 1.0f - Dot((pmh->absmin+pmh->absmax)/2.0f - g_cam.view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

		glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
        glVertexPointer(3, GL_FLOAT, 0, topverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
        glVertexPointer(3, GL_FLOAT, 0, farverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
#endif
}

void DrawDrag_Door(EdMap* map, Matrix* mvp, int w, int h, ecbool persp)
{
	Shader* shader = &g_sh[g_curS];

	Brush* b = *g_selB.begin();

	EdDoor* door = b->door;

	if(!door)
		return;

	Vec3f startpoint = door->point;
	Vec3f axispoint = door->point + door->axis;

	Vec4f startscreenpos = ScreenPos(mvp, startpoint, w, h, persp);
	Vec4f axisscreenpos = ScreenPos(mvp, axispoint, w, h, persp);

	{
		float verts[] =
		{
			startscreenpos.x, startscreenpos.y, 0,
			axisscreenpos.x, axisscreenpos.y, 0
		};

		float colour2[] = DOOR_POINT_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINE_STRIP, 0, 2);

	}

	{
		float verts[] =
		{
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0
		};

		float colour[] = DOOR_POINT_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_QUADS, 0, 4);

		float colour2[] = DOOR_POINT_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}

	{
		float verts[] =
		{
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0
		};

		float colour[] = DOOR_AXIS_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_QUADS, 0, 4);

		float colour2[] = DOOR_AXIS_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
}


void DrawDrag_Clip(EdMap* map, Matrix* mvp, int w, int h, ecbool persp)
{
	return;

	Vec2i vmin;
	Vec2i vmax;

	AllScreenMinMax(&vmin, &vmax, w, h);

	Shader* shader = &g_sh[g_curS];

	float verts[] =
	{
		(float)(vmin.x), (float)vmin.y, 0,
		(float)vmax.x, (float)(vmin.y), 0,
		(float)(vmax.x), (float)vmax.y, 0,
		(float)vmin.x, (float)(vmax.y), 0,
		(float)(vmin.x), (float)vmin.y, 0
	};

	float colour[] = VERT_DRAG_FILLRGBA;
	glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

	glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	glDrawArrays(GL_QUADS, 0, 4);

	float colour2[] = VERT_DRAG_OUTLRGBA;
	glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

	glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

void DrawDrag_VertFaceBrush(EdMap* map, Matrix* mvp, int w, int h, ecbool persp)
{
	Shader* shader = &g_sh[g_curS];

	for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;

		for(int j=0; j<b->nsharedv; j++)
		{
			Vec3f sharedv = b->sharedv[j];
			Vec4f screenpos = ScreenPos(mvp, sharedv, w, h, persp);

			float verts[] =
			{
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0
			};

			float colour[] = VERT_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
            glVertexPointer(3, GL_FLOAT, 0, verts);
			glDrawArrays(GL_QUADS, 0, 4);

			float colour2[] = VERT_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
            glVertexPointer(3, GL_FLOAT, 0, verts);
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}
	}

	for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* side = &b->sides[j];
			Vec4f screenpos = ScreenPos(mvp, side->centroid, w, h, persp);

			float verts[] =
			{
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0
			};

			if(g_sel1b == b && g_dragS == j)
			{
				float colour[] = FACE_DRAG_SFILLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
			}
			else
			{
				float colour[] = FACE_DRAG_FILLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
			}

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
            glVertexPointer(3, GL_FLOAT, 0, verts);
			glDrawArrays(GL_QUADS, 0, 4);

			if(g_sel1b == b && g_dragS == j)
			{
				float colour2[] = FACE_DRAG_SOUTLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
			}
			else
			{
				float colour2[] = FACE_DRAG_OUTLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
			}

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
            glVertexPointer(3, GL_FLOAT, 0, verts);
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}
	}


	for(std::vector<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;
		Vec3f centroid = Vec3f(0,0,0);

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* s = &b->sides[j];

			centroid = centroid + s->centroid / (float)b->nsides;
		}

		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		float verts[] =
		{
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0
		};

		float coluor[] = BRUSH_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, coluor);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_QUADS, 0, 4);

		float colour2[] = BRUSH_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
}


void DrawDrag_ModelHolder(EdMap* map, Matrix* mvp, int w, int h, ecbool persp)
{
	Shader* shader = &g_sh[g_curS];

	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		// Top side
		Vec3f topcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmax.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f topscreenpos = ScreenPos(mvp, topcentroid, w, h, persp);
		float topverts[] =
		{
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Bottom side
		Vec3f bottomcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmin.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f bottomscreenpos = ScreenPos(mvp, bottomcentroid, w, h, persp);
		float bottomverts[] =
		{
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Left side
		Vec3f leftcentroid = Vec3f( pmh->absmin.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f leftscreenpos = ScreenPos(mvp, leftcentroid, w, h, persp);
		float leftverts[] =
		{
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Right side
		Vec3f rightcentroid = Vec3f( pmh->absmax.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f rightscreenpos = ScreenPos(mvp, rightcentroid, w, h, persp);
		float rightverts[] =
		{
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Near side
		Vec3f nearcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmax.z );
		Vec4f nearscreenpos = ScreenPos(mvp, nearcentroid, w, h, persp);
		float nearverts[] =
		{
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Far side
		Vec3f farcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmin.z );
		Vec4f farscreenpos = ScreenPos(mvp, farcentroid, w, h, persp);
		float farverts[] =
		{
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		if(g_sel1m == pmh && g_dragS == DRAG_TOP)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
        glVertexPointer(3, GL_FLOAT, 0, topverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_TOP)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
        glVertexPointer(3, GL_FLOAT, 0, topverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Bottom side
		if(g_sel1m == pmh && g_dragS == DRAG_BOTTOM)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_BOTTOM)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Left side
		if(g_sel1m == pmh && g_dragS == DRAG_LEFT)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_LEFT)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Right side
		if(g_sel1m == pmh && g_dragS == DRAG_RIGHT)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_RIGHT)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Near side
		if(g_sel1m == pmh && g_dragS == DRAG_NEAR)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_NEAR)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Far side
		if(g_sel1m == pmh && g_dragS == DRAG_FAR)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
        glVertexPointer(3, GL_FLOAT, 0, farverts);
		glDrawArrays(GL_QUADS, 0, 4);
		if(g_sel1m == pmh && g_dragS == DRAG_FAR)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
        glVertexPointer(3, GL_FLOAT, 0, farverts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);


	}


	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		Vec3f centroid = (pmh->absmin + pmh->absmax) / 2.0f;

		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		float verts[] =
		{
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0
		};

		float colorr[] = BRUSH_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colorr);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_QUADS, 0, 4);

		float colour2[] = BRUSH_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

		//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
}

//draw drag handles
void DrawDrag(EdMap* map, Matrix* mvp, int w, int h, ecbool persp)
{
	return;
	Shader* shader = &g_sh[g_curS];

	if(g_gui.get("door edit")->opened)
	{
		if(g_selB.size() <= 0)
			return;

		//return;

		//DrawDrag_Door(map, mvp, w, h, persp);
	}
	else if(g_selB.size() > 0)
	{
		DrawDrag_VertFaceBrush(map, mvp, w, h, persp);
	}
	else if(g_selM.size() > 0)
	{
		DrawDrag_ModelHolder(map, mvp, w, h, persp);
	}

	DrawDrag_Clip(map, mvp, w, h, persp);
}

void SelectBrush(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax)
{
	g_dragV = -1;
	g_dragS = -1;
	CloseSideView();
	//CloseView("brush edit");

	//Log("select brush ("<<line[0].x<<","<<line[0].y<<","<<line[0].z<<")->("<<line[1].x<<","<<line[1].y<<","<<line[1].z<<")");
	std::list<Brush*> selB;
	std::list<ModelHolder*> selM;

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		Vec3f trace = b->traceray(line);
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			//g_selB.clear();
			//g_selB.push_back(&*b);
			selB.push_back(&*b);
			//OpenAnotherView("brush edit");
			//return;
		}
	}

	for(std::list<ModelHolder>::iterator mh=g_modelholder.begin(); mh!=g_modelholder.end(); mh++)
	{
		Vec3f trace = mh->traceray(line);
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			//g_selB.clear();
			//g_selB.push_back(&*b);
			selM.push_back(&*mh);
			//OpenAnotherView("brush edit");
			//return;
		}
	}

	//If we already have a selected brush (globally),
	//choose the one after it in our selection array
	//(selB) as the next selected brush.
	if(g_selB.size() == 1)
	{
		ecbool found = ecfalse;
		for(std::list<Brush*>::iterator i=selB.begin(); i!=selB.end(); i++)
		{
			if(found)
			{
				g_selB.clear();
				g_selB.push_back(*i);
				CloseSideView();
				g_gui.show("brush edit");
				return;
			}

			std::vector<Brush*>::iterator j = g_selB.begin();

			if(*i == *j)
				found = ectrue;
		}
	}

	int prevnB = g_selB.size();
	g_selB.clear();

	//If we've reached the end of the selection array,
	//(selB), select through the models,
	if(g_selM.size() == 1)
	{
		ecbool found = ecfalse;
		for(std::list<ModelHolder*>::iterator i=selM.begin(); i!=selM.end(); i++)
		{
			if(found)
			{
				g_selM.clear();
				g_selM.push_back(*i);
				//OpenAnotherView("model edit");
				return;
			}

			std::vector<ModelHolder*>::iterator j = g_selM.begin();

			if(*i == *j)
				found = ectrue;
		}
	}

	int prevnM = g_selM.size();
	g_selM.clear();

	//and then restart at model at the front
	//if we previously selected a brush (and
	//reached the last one) or if there aren't
	//any brushes at all
	if(selM.size() > 0
		&& (prevnB > 0 || selB.size() == 0))
	{
		g_selM.push_back( *selM.begin() );
		return;
	}
	//or else the brush at the front.
	else if(selB.size() > 0)
	{
		g_selB.push_back( *selB.begin() );
		g_gui.show("brush edit");
		return;
	}
	//else select model, if nothing is selected or model
	//was previously selected from another place (?)
	//Edit: maybe we don't want to select anything after cycle through.
#if 0
	else if(selM.size() > 0)
	{
		g_selM.push_back( *selM.begin() );
		return;
	}
#endif
}

ecbool SelectDrag_Door(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, ecbool persp, Brush* b, EdDoor* door)
{
	Vec3f startvec = door->point;
	Vec3f axisvec = door->point + door->axis;

	Vec4f startscreenpos = ScreenPos(mvp, startvec, w, h, persp);
	Vec4f axisscreenpos = ScreenPos(mvp, axisvec, w, h, persp);

	if(x >= axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE && x <= axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE && y >= axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE && y <= axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE)
	{
		g_sel1b = b;
		g_dragV = -1;
		g_dragS = -1;
		g_dragW = ecfalse;
		g_dragD = DRAG_DOOR_AXIS;
		return ectrue;
	}

	if(x >= startscreenpos.x - DOOR_POINT_DRAG_HSIZE && x <= startscreenpos.x + DOOR_POINT_DRAG_HSIZE && y >= startscreenpos.y - DOOR_POINT_DRAG_HSIZE && y <= startscreenpos.y + DOOR_POINT_DRAG_HSIZE)
	{
		g_sel1b = b;
		g_dragV = -1;
		g_dragS = -1;
		g_dragW = ecfalse;
		g_dragD = DRAG_DOOR_POINT;
		return ectrue;
	}

	return ecfalse;
}

void SelectDrag_VertFaceBrush(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, ecbool persp)
{
	float nearest = -1;

	for(int i=0; i<g_selB.size(); i++)
	{
		Brush* b = g_selB[i];

		for(int j=0; j<b->nsharedv; j++)
		{
			Vec3f sharedv = b->sharedv[j];
			Vec4f screenpos = ScreenPos(mvp, sharedv, w, h, persp);

			if(x >= screenpos.x - VERT_DRAG_HSIZE && x <= screenpos.x + VERT_DRAG_HSIZE && y >= screenpos.y - VERT_DRAG_HSIZE && y <= screenpos.y + VERT_DRAG_HSIZE)
			{
				float thisD = MAG_VEC3F(b->sharedv[j] - eye);

				if(thisD < nearest || nearest < 0)
				{
					g_sel1b = b;
					g_dragV = j;
					g_dragS = -1;
					g_dragW = ecfalse;
					nearest = thisD;
				}
			}
		}

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* side = &b->sides[j];
			//Log("centroid "<<side->centroid.x<<","<<side->centroid.y<<","<<side->centroid.z<<std::endl;
			//
			Vec4f screenpos = ScreenPos(mvp, side->centroid, w, h, persp);

			if(x >= screenpos.x - FACE_DRAG_HSIZE && x <= screenpos.x + FACE_DRAG_HSIZE && y >= screenpos.y - FACE_DRAG_HSIZE && y <= screenpos.y + FACE_DRAG_HSIZE)
			{
				float thisD = MAG_VEC3F(side->centroid - eye);

				if(thisD <= nearest || nearest < 0 || g_dragS < 0)
				{
					g_sel1b = b;
					g_dragV = -1;
					g_dragS = j;
					g_dragW = ecfalse;
					nearest = thisD;
				}
			}
		}

		Vec3f centroid(0,0,0);

		for(int j=0; j<b->nsides; j++)
		{
			BrushSide* side = &b->sides[j];
			centroid = centroid + side->centroid;
		}

		centroid = centroid / (float)b->nsides;
		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		if(x >= screenpos.x - BRUSH_DRAG_HSIZE && x <= screenpos.x + BRUSH_DRAG_HSIZE && y >= screenpos.y - BRUSH_DRAG_HSIZE && y <= screenpos.y + BRUSH_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(centroid - eye);

			g_sel1b = b;
			g_dragV = -1;
			g_dragS = -1;
			g_dragW = ectrue;
			nearest = thisD;
		}
	}
}

void SelectDrag_ModelHolder(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, ecbool persp)
{
	float nearest = -1;
	Shader* shader = &g_sh[g_curS];

	for(std::vector<ModelHolder*>::iterator mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		// Whole model centroid
		Vec3f modelcentroid =(pmh->absmin + pmh->absmax)/2.0f;
		Vec4f modelscreenpos = ScreenPos(mvp, modelcentroid, w, h, persp);
		float centerverts[] =
		{
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0,
			modelscreenpos.x + FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0,
			modelscreenpos.x + FACE_DRAG_HSIZE, modelscreenpos.y + FACE_DRAG_HSIZE, 0,
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y + FACE_DRAG_HSIZE, 0,
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		Vec3f topcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmax.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f topscreenpos = ScreenPos(mvp, topcentroid, w, h, persp);
		float topverts[] =
		{
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Bottom side
		Vec3f bottomcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmin.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f bottomscreenpos = ScreenPos(mvp, bottomcentroid, w, h, persp);
		float bottomverts[] =
		{
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Left side
		Vec3f leftcentroid = Vec3f( pmh->absmin.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f leftscreenpos = ScreenPos(mvp, leftcentroid, w, h, persp);
		float leftverts[] =
		{
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Right side
		Vec3f rightcentroid = Vec3f( pmh->absmax.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f rightscreenpos = ScreenPos(mvp, rightcentroid, w, h, persp);
		float rightverts[] =
		{
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Near side
		Vec3f nearcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmax.z );
		Vec4f nearscreenpos = ScreenPos(mvp, nearcentroid, w, h, persp);
		float nearverts[] =
		{
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Far side
		Vec3f farcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmin.z );
		Vec4f farscreenpos = ScreenPos(mvp, farcentroid, w, h, persp);
		float farverts[] =
		{
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		if(x >= topscreenpos.x - FACE_DRAG_HSIZE && x <= topscreenpos.x + FACE_DRAG_HSIZE && y >= topscreenpos.y - FACE_DRAG_HSIZE && y <= topscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(topcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_TOP;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Bottom side
		if(x >= bottomscreenpos.x - FACE_DRAG_HSIZE && x <= bottomscreenpos.x + FACE_DRAG_HSIZE && y >= bottomscreenpos.y - FACE_DRAG_HSIZE && y <= bottomscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(bottomcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_BOTTOM;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Left side
		if(x >= leftscreenpos.x - FACE_DRAG_HSIZE && x <= leftscreenpos.x + FACE_DRAG_HSIZE && y >= leftscreenpos.y - FACE_DRAG_HSIZE && y <= leftscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(leftcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_LEFT;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Right side
		if(x >= rightscreenpos.x - FACE_DRAG_HSIZE && x <= rightscreenpos.x + FACE_DRAG_HSIZE && y >= rightscreenpos.y - FACE_DRAG_HSIZE && y <= rightscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(rightcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_RIGHT;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Near side
		if(x >= nearscreenpos.x - FACE_DRAG_HSIZE && x <= nearscreenpos.x + FACE_DRAG_HSIZE && y >= nearscreenpos.y - FACE_DRAG_HSIZE && y <= nearscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(nearcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_NEAR;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Far side
		if(x >= farscreenpos.x - FACE_DRAG_HSIZE && x <= farscreenpos.x + FACE_DRAG_HSIZE && y >= farscreenpos.y - FACE_DRAG_HSIZE && y <= farscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(farcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_FAR;
				g_dragW = ecfalse;
				nearest = thisD;
			}
		}

		// Whole model drag
		if(x >= modelscreenpos.x - FACE_DRAG_HSIZE && x <= modelscreenpos.x + FACE_DRAG_HSIZE && y >= modelscreenpos.y - FACE_DRAG_HSIZE && y <= modelscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = MAG_VEC3F(modelcentroid - eye);

			// Prefer selecting the whole model drag handle over selecting a side drag handle
			if(thisD < nearest || nearest < 0 || g_sel1m == pmh || g_sel1m == NULL)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = -1;
				g_dragW = ectrue;
				nearest = thisD;
			}
		}
	}
}

ecbool SelectDrag(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, ecbool persp)
{
	g_sel1b = NULL;
	g_sel1m = NULL;
	g_dragV = -1;
	g_dragS = -1;
	g_dragW = ecfalse;
	g_dragD = -1;

#if 0
	if(g_gui.get("door edit")->opened)
	{
		if(g_selB.size() <= 0)
			goto nodoor;

		Brush* b = *g_selB.begin();
		EdDoor* door = b->door;

		if(door == NULL)
			goto nodoor;

		if(SelectDrag_Door(map, mvp, w, h, x, y, eye, persp, b, door))
			return ectrue;
	}

	nodoor:
#endif

	CloseSideView();
	//CloseView("brush side edit");

	SelectDrag_VertFaceBrush(map, mvp, w, h, x, y, eye, persp);

	if(g_sel1b != NULL)
	{
		if(g_dragS >= 0)
		{
			CloseSideView();
			g_gui.show("brush edit");
			g_gui.show("brush side edit");
			RedoBSideGUI();
		}

		return ectrue;
	}

	SelectDrag_ModelHolder(map, mvp, w, h, x, y, eye, persp);

	if(g_sel1m != NULL)
	{
		//MessageBox(g_hWnd, "sle m", "asd", NULL);
		return ectrue;
	}

	return ecfalse;
}

//#define PRUNEB_DEBUG

ecbool PruneB(EdMap* map, Brush* b)
{
#ifdef PRUNEB_DEBUG
	Log("-------------");
	
#endif

	//remove sides that share the exact same set of vertices
	for(int i=0; i<b->nsides; i++)
	{
#ifdef PRUNEB_DEBUG
		Log("\t side"<<i<<std::endl;
#endif
		BrushSide* s1 = &b->sides[i];

		if(s1->ntris <= 0)
		{
			b->removeside(i);
			i--;
			continue;
		}

		ecbool allclose = ectrue;
		Vec3f matchv = b->sharedv[ s1->vindices[0] ];

#ifdef PRUNEB_DEBUG
		Log("s1->ntris = "<<s1->ntris<<std::endl;
#endif

		for(int v=0; v<s1->outline.edv.size(); v++)
		{
			Vec3f thisv = b->sharedv[ s1->vindices[v] ];

#ifdef PRUNEB_DEBUG
			Log("vertex "<<v<<" = "<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			
#endif

			float mag = MAG_VEC3F(matchv - thisv);

			if(mag > MERGEV_D)
			{
				allclose = ecfalse;
#ifndef PRUNEB_DEBUG
				break;
#endif
			}
		}

		if(allclose)
		{
			b->removeside(i);
			i--;
			continue;
		}

		for(int j=i+1; j<b->nsides; j++)
		{
			BrushSide* s2 = &b->sides[j];
			ecbool same = ectrue;

			for(int k=0; k<s1->outline.edv.size(); k++)
			{
				int matchindex = s1->vindices[k];
				ecbool have = ecfalse;

				for(int l=0; l<s2->outline.edv.size(); l++)
				{
					if(s2->vindices[l] == matchindex)
					{
						have = ectrue;
						break;
					}
				}

				if(!have)
				{
					same = ecfalse;
					break;
				}
			}

			if(same)
			{
				b->removeside(j);
				break;
			}

			for(int k=0; k<s2->outline.edv.size(); k++)
			{
				int matchindex = s2->vindices[k];
				ecbool have = ecfalse;

				for(int l=0; l<s1->outline.edv.size(); l++)
				{
					if(s1->vindices[l] == matchindex)
					{
						have = ectrue;
						break;
					}
				}

				if(!have)
				{
					same = ecfalse;
					break;
				}
			}

			if(same)
			{
				b->removeside(j);
				break;
			}
		}
	}

	//for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		// remove brushes with less than 3 vertices or 4 sides
		if(b->nsharedv < 3 || b->nsides < 4)
		{
			for(int i=0; i<g_selB.size(); i++)
			{
				//if(g_selB[i] == &*b)
				if(g_selB[i] == b)
				{
					g_selB.erase( g_selB.begin() + i );
					//continue;
					i--;
				}
			}

			//if(g_sel1b == &*b)
			if(g_sel1b == b)
			{
				g_sel1b = NULL;
				g_dragV = -1;
				g_dragS = -1;
			}

			//b = map->brush.erase(b);

			for(std::list<Brush>::iterator i=map->brush.begin(); i!=map->brush.end(); i++)
			{
				if(&*i == b)
				{
					map->brush.erase(i);
					//break;
					return ectrue;
				}
			}

			//Log("pruned brush");
			//
			//continue;

			return ectrue;
		}
	}

	return ecfalse;
}
