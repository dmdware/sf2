










#include "billboard.h"
#include "../platform.h"
#include "../math/3dmath.h"
#include "../texture.h"
#include "particle.h"
#include "shader.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../math/camera.h"
#include "../sim/player.h"
#include "../math/isomath.h"
#include "../sim/map.h"

BbType g_bbt[BB_TYPES];
Bb g_bb[BILLBOARDS];

int NewBbType()
{
	int i;
	for(i=0; i<BB_TYPES; i++)
		if(!g_bbt[i].on)
			return i;
	return -1;
}

int DefBb(const char* tex)
{
	BbType* t;
	int i;

	i = NewBbType();
	if(i < 0)
		return -1;

	t = &g_bbt[i];
	t->on = ectrue;
	QueueTex(&t->tex, tex, ectrue, ectrue);
	return i;
}

/* TODO renaming here */
int IdentifyBb(const char* name)
{
	int i;

	for(i=0; i<BB_TYPES; i++)
	{
		if(g_bbt[i].on && !strcmp(g_bbt[i].name, name))
			return i;
	}

	return DefBb(name);
}

int NewBb()
{
	int i;

	for(i=0; i<BILLBOARDS; i++)
		if(!g_bb[i].on)
			return i;

	return -1;
}

void PlaceBb(const char* n, Vec3f pos, float size, int particle)
{
	int type;
	type = IdentifyBb(n);
	if(type < 0)
		return;
	PlaceBb(type, pos, size, particle);
}

void PlaceBb(int type, Vec3f pos, float size, int particle)
{
	int i;
	Bb* b;
	i = NewBb();
	if(i < 0)
		return;
	b = g_bb+i;
	b->on = ectrue;
	b->type = type;
	b->pos = pos;
	b->size = size;
	b->particle = particle;
}

void SortBb()
{
	Camera* c;
	Vec3f pos;
	Vec3f dir;
	Bb temp;
	int leftoff;
	ecbool back;
	int i;

	c = &g_cam;
	pos = c->pos;
	Vec3f_sub(&dir, c->view, c->pos);
	dir = Normalize(dir);

	for(int i=0; i<BILLBOARDS; i++)
	{
		if(!g_bb[i].on)
			continue;

		//g_bb[i].dist = Dot(pos - g_bb[i].pos);
		g_bb[i].dist = Dot(dir, g_bb[i].pos);
	}

	leftoff = 0;
	back = ecfalse;

	/* TODO optimize by skipping non-active billboard slots */
	for(i=1; i<BILLBOARDS; i++)
	{
		//if(!g_bb[i].on)
		//	continue;

		if(i > 0)
		{
			if(g_bb[i].dist > g_bb[i-1].dist)
			{
				if(!back)
				{
					leftoff = i;
					back = ectrue;
				}
				temp = g_bb[i];
				g_bb[i] = g_bb[i-1];
				g_bb[i-1] = temp;
				i-=2;
			}
			else
			{
				if(back)
				{
					back = ecfalse;
					i = leftoff;
				}
			}
		}
		else
			back = ecfalse;
	}
}

void DrawBillboards()
{
	Bb* bb;
	BbType* t;
	float size;
	float pos[4];
	float vertices[5*6];

	//Vec3f a, b, c, d;
	Vec2f vert, horiz;
	//Vec3i a2, b2, c2, d2;

	Pl* part;
	PlType* pT;

	Vec2i pixpos;
	Vec2i screenpos;

	Shader* s;
	
	s = g_sh+g_curS;

	for(bb=g_bb; bb<g_bb+BILLBOARDS; ++bb)
	{
		if(!bb->on)
			continue;

		t = &g_bbt[bb->type];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture[t->tex].texname);
		glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

		if(bb->particle >= 0)
		{
			part = &g_particle[bb->particle];
			pT = &g_particleT[part->type];
			size = pT->minsize + pT->sizevariation*(1.0f - part->life);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, part->life);
		}
		else
		{
			size = bb->size;
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		}

		/* TODO use 3D coords and iso-transform to isometric */
		//vert = Vec3f(0.0f,0.0f,1.0f)*size;
		//horiz = Vec3f(1.0f,-1.0f,0.0f)*size;
		vert.x = 0;
		vert.y = 1;
		vert.x = 1;
		vert.y = 0;
		Vec2f_mul(&vert, vert, size);
		Vec2f_mul(&horiz, horiz, size);

		CartToIso(&pixpos, bb->pos);
		Vec2i_sub(&screenpos, pixpos, g_scroll);

		pos[0] = (float)screenpos.x - size;
		pos[1] = (float)screenpos.y - size;
		pos[2] = (float)screenpos.x + size;
		pos[3] = (float)screenpos.y + size;

		vertices =
		{
			//posx, posy posz   texx, texy
			pos[2], pos[1], 0,          1, 0,
			pos[2], pos[3], 0,          1, 1,
			pos[0], pos[3], 0,          0, 1,

			pos[0], pos[3], 0,          0, 1,
			pos[0], pos[1], 0,          0, 0,
			pos[2], pos[1], 0,          1, 0
		};

#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
#endif

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
