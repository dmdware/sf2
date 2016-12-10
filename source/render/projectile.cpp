











#include "projectile.h"
#include "../texture.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../math/camera.h"
#include "../sim/player.h"

ProjectileType g_projectileType[PROJECIN_TYPES];
Projectile g_projectile[PROJECTILES];

void ProjectileType::Define(char* texpath)
{
	//CreateTex(tex, texpath);
	QueueTex(&tex, texpath, ectrue, ectrue);
}

void LoadProjectiles()
{
	g_projectileType[GUNPROJ].Define("particles/gunproj.png");
}

int NewProjectile()
{
	for(int i=0; i<PROJECTILES; i++)
		if(!g_projectile[i].on)
			return i;

	return -1;
}

void NewProjectile(Vec3f start, Vec3f end, int type)
{
	//g_projectile.push_back(Projectile(start, end, type));
	int i = NewProjectile();
	if(i < 0)
		return;

	g_projectile[i] = Projectile(start, end, type);
}

void DrawProjectiles()
{
	Py* py = &g_py[g_localP];
	Camera* cam = &g_cam;

	Projectile* proj;
	ProjectileType* t;
	Vec3f start;
	Vec3f end;

	float dist;
	float sizeparallel = 8, sizeperpindicular = 1;
	float ratio;
	Vec3f delta;
	Vec3f center;
	Vec3f parallel;
	Vec3f perpindicular;
	Vec3f view = Normalize(cam->view - cam->pos);
	Vec3f a, b, c, d;

	Shader* s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

	for(int i=0; i<PROJECTILES; i++)
	{
		proj = &g_projectile[i];
		if(!proj->on)
			continue;

		start = proj->start;
		end = proj->end;
		delta = end - start;
		t = &g_projectileType[proj->type];
		dist = MAG_VEC3F(delta);
		ratio = (float)(rand()%1000)/1000.0f;
		center = start + delta * ratio;
		parallel = Normalize(delta) * sizeparallel/2.0f;
		perpindicular = Normalize(Cross(parallel, view)) * sizeperpindicular/2.0f;
		a = center - parallel + perpindicular;
		b = center - parallel - perpindicular;
		c = center + parallel - perpindicular;
		d = center + parallel + perpindicular;

		glBindTexture(GL_TEXTURE_2D, t->tex);
		/*
		glBindTexture(GL_TEXTURE_2D, t->tex);
		glBegin(GL_QUADS);

		0, 0);		glVertex3f(a.x, a.y, a.z);
		0, 1);		glVertex3f(b.x, b.y, b.z);
		1, 1);		glVertex3f(c.x, c.y, c.z);
		1, 0);		glVertex3f(d.x, d.y, d.z);

		glEnd();*/

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			a.x, a.y, a.z,          0, 0,
			b.x, b.y, b.z,          0, 1,
			c.x, c.y, c.z,          1, 1,

			c.x, c.y, c.z,          1, 1,
			d.x, d.y, d.z,          1, 0,
			a.x, a.y, a.z,          0, 0
		};

#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
#endif

		glDrawArrays(GL_TRIANGLES, 0, 6);

		proj->on = ecfalse;
	}
}
