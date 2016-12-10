











#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"

struct ProjectileType
{
public:
	unsigned int tex;

	void Define(char* texpath);
};

enum PROJECTILE {GUNPROJ, PROJECIN_TYPES};
extern ProjectileType g_projectileType[PROJECIN_TYPES];

struct Projectile
{
public:
	ecbool on;
	Vec3f start;
	Vec3f end;
	int type;

	Projectile()
	{
		on = ecfalse;
	}

	Projectile(Vec3f s, Vec3f e, int t)
	{
		on = ectrue;
		start = s;
		end = e;
		type = t;
	}
};

#define PROJECTILES	128
extern Projectile g_projectile[PROJECTILES];

void LoadProjectiles();
void DrawProjectiles();
void NewProjectile(Vec3f start, Vec3f end, int type);
