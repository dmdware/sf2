











#include "../platform.h"
#include "heightmap.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../gui/gui.h"
#include "../gui/icon.h"
#include "../math/vec4f.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../utils.h"
#include "../math/camera.h"
#include "../window.h"
#include "../math/polygon.h"
#include "../math/physics.h"
#include "../math/plane3f.h"
#include "../path/collidertile.h"
#include "foliage.h"
#include "../save/savemap.h"
#include "../texture.h"
#include "water.h"
#include "../sim/player.h"
#include "../debug.h"
#include "fogofwar.h"
#include "../sim/conduit.h"
#include "../sim/border.h"
#include "../math/hmapmath.h"
#include "../algo/probcloud.h"

Vec2i g_mapview[2];
Heightmap g_hmap;

/*
Number of tiles, not heightpoints/corners.
Number of height points/corners is +1.
*/
Vec2uc g_mapsz(0,0);

void AllocGrid(int wx, int wy)
{
	Log("allocating struct arrays %d,%d\r\n", wx, wy);

	if( !(g_cdtype[CD_ROAD].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_cdtype[CD_ROAD].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_cdtype[CD_POWL].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_cdtype[CD_POWL].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_cdtype[CD_CRPIPE].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_cdtype[CD_CRPIPE].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OUTOFMEM();
	if( !(g_vistile = new VisTile[ (wx*wy) ]) ) OUTOFMEM();
	if( !(g_border = new signed char[ (wx*wy) ]) ) OUTOFMEM();
	if( !(g_labdem = new int[ (wx*wy) ]) ) OUTOFMEM();
	if( !(g_labsup = new int[ (wx*wy) ]) ) OUTOFMEM();

	memset(g_labdem, 0, sizeof(int));
	memset(g_labsup, 0, sizeof(int));

	memset(g_border, -1, sizeof(signed char) * wx * wy);
}

/*
allocate room for a number of tiles, not height points.
*/
void Heightmap::alloc(int wx, int wy)
{
	destroy();

	// Vertices aren't shared between tiles or triangles.
	int numverts = wx * wy * 3 * 2;

	g_mapsz.x = wx;
	g_mapsz.y = wy;

	width.x = wx;
	width.y = wy;

	hpt = new unsigned char [ (wx+1) * (wy+1) ];
	origv = new Vec3f [ numverts ];
	norm = new Vec3f [ numverts ];
	texco = new Vec2f [ numverts ];
	tricfg = new ecbool [ wx * wy ];
	tridiv = new Plane3f [ wx * wy ];
	surftile = new Tile [ wx * wy ];

	if(!hpt) OUTOFMEM();
	if(!origv) OUTOFMEM();
	if(!norm) OUTOFMEM();
	if(!texco) OUTOFMEM();
	if(!tricfg) OUTOFMEM();
	if(!tridiv) OUTOFMEM();
	if(!surftile) OUTOFMEM();

	//Log("setting heights to 0");
	//

	// Set to initial height.
	for(int x=0; x<=wx; x++)
		for(int y=0; y<=wy; y++)
			//hpt[ y*(wx+1) + x ] = rand()%1000;
			hpt[ y*(wx+1) + x ] = 0;
    
    lowereven();
	remesh();
	//retexture();
}

void FreeGrid()
{
	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];
		//nice
		CdTile*& actual = ct->cdtiles[(int)ecfalse];
		CdTile*& planned = ct->cdtiles[(int)ectrue];

		if(planned)
		{
			delete [] planned;
			planned = NULL;
		}

		if(actual)
		{
			delete [] actual;
			actual = NULL;
		}
	}

	if(g_vistile)
	{
		delete [] g_vistile;
		g_vistile = NULL;
	}

	if(g_border)
	{
		delete [] g_border;
		g_border = NULL;
	}

	delete [] g_labdem;
	delete [] g_labsup;

	g_labdem = NULL;
	g_labsup = NULL;
}

void Heightmap::destroy()
{
	//if(g_mapsz.x <= 0 || g_mapsz.y <= 0)
	//	return;

	//Log("deleting [] g_open");
	
	/*
		delete [] g_open;

		Log("deleting [] g_road");
		
		*/

    if(hpt)
    {
        delete [] hpt;
        hpt = NULL;
    }
    
    if(origv)
    {
        delete [] origv;
        origv = NULL;
    }
    
    if(norm)
    {
        delete [] norm;
        norm = NULL;
    }
    
    if(texco)
    {
        delete [] texco;
        texco = NULL;
    }
    
    if(surftile)
    {
        delete [] surftile;
        surftile = NULL;
    }

	//2015/10/25 fixed mem leaks:

	if(tridiv)
	{
        delete [] tridiv;
        tridiv = NULL;
	}

	if(tricfg)
	{
        delete [] tricfg;
        tricfg = NULL;
	}

	//TODO don't check for null before deleting/freeing
	//TODO clean up path/collider/cd grid alloc and dealloc calls with hmap alloc/dealloc

	g_mapsz.x = 0;
	g_mapsz.y = 0;

	FreePathGrid();
}

void Heightmap::adjheight(int x, int y, signed char change)
{
	hpt[ (y)*(g_mapsz.x+1) + x ] += change;
}

void Heightmap::setheight(int x, int y, unsigned char height)
{
	hpt[ (y)*(g_mapsz.x+1) + x ] = height;
}


float Heightmap::accheight2(float x, float y)
{
	//return accheight((int)x,(int)y) * TILE_RISE;
	return accheight(x,y);

	float tx = x / (float)TILE_SIZE;
	float ty = y / (float)TILE_SIZE;

	if(tx < 0)
		tx = 0;

	if(ty < 0)
		ty = 0;

	//if(tx >= 1)
	//	tx = 0.999;

	//if(ty >= 1)
	//	ty = 0.999;

	if(tx >= width.x)
		tx = width.x*0.999;

	if(ty >= width.y)
		ty = width.y*0.999;

	//return Bilerp(this, x, y);
	//TODO check if works

	int tileindex = (int)(ty*width.x) + (int)(tx);
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(tx*TILE_SIZE, 0, ty*TILE_SIZE);

	Vec3f tri[3];

	Vec3f trinorm = norm[ tileindex6v ];

	if(PointBehindPlane(point, tridiv[tileindex]))
	{
		trinorm = norm[ tileindex6v + 0];

		tri[0] = origv[ tileindex6v + 0 ];
		tri[1] = origv[ tileindex6v + 1 ];
		tri[2] = origv[ tileindex6v + 2 ];
	}
	else
	{
		trinorm = norm[ tileindex6v + 3];

		tri[0] = origv[ tileindex6v + 3 ];
		tri[1] = origv[ tileindex6v + 4 ];
		tri[2] = origv[ tileindex6v + 5 ];
	}

	Plane3f plane;

//	Vec3f trinorm = Normal(tri);
	//Vec3f trinorm = norm[ tileindex6v ];

	float temp = tri[0].y;
	tri[0].y = tri[0].z;
	tri[0].z = temp;

	temp = trinorm.y;
	trinorm.y = trinorm.z;
	trinorm.z = temp;

	MakePlane(&plane.normal, &plane.d, tri[0], trinorm);

	float z = - ( x*plane.normal.x + y*plane.normal.z + plane.d ) / plane.normal.y;

	z = imin(TILE_RISE, z);

	return z;
}

float Heightmap::accheight(float x, float y)
{
	//return accheight((int)x,(int)y);

	//return Bilerp(this, x, y);

	float tx = x / (float)TILE_SIZE;
	float ty = y / (float)TILE_SIZE;

	if(tx < 0)
		tx = 0;

	if(ty < 0)
		ty = 0;

	//if(tx >= 1)
	//	tx = 0.999;

	//if(ty >= 1)
	//	ty = 0.999;

	if(tx >= width.x)
		tx = width.x*0.999;

	if(ty >= width.y)
		ty = width.y*0.999;

	return Bilerp(this, tx*TILE_SIZE, ty*TILE_SIZE);
	//return Bilerp(this, x, y);
	//TODO check if works

	int tileindex = (int)(ty*width.x) + (int)(tx);
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(tx*TILE_SIZE, ty*TILE_SIZE, 0);

	Vec3f tri[3];

	Vec3f trinorm = norm[ tileindex6v ];

	if(!PointBehindPlane(point, tridiv[tileindex]))
	{
		trinorm = norm[ tileindex6v + 0];

		tri[0] = origv[ tileindex6v + 0 ];
		tri[1] = origv[ tileindex6v + 1 ];
		tri[2] = origv[ tileindex6v + 2 ];
	}
	else
	{
		trinorm = norm[ tileindex6v + 3];

		tri[0] = origv[ tileindex6v + 3 ];
		tri[1] = origv[ tileindex6v + 4 ];
		tri[2] = origv[ tileindex6v + 5 ];
	}

	Plane3f plane;

//	Vec3f trinorm = Normal(tri);
	//Vec3f trinorm = norm[ tileindex6v ];

	MakePlane(&plane.normal, &plane.d, tri[0], trinorm);

	//float z = - ( (point.x-(int)tx*TILE_SIZE)*plane.normal.x + (point.y-(int)ty*TILE_SIZE)*plane.normal.y + plane.d ) / plane.normal.z;

	float z = - ( (point.x)*plane.normal.x + (point.y)*plane.normal.y + plane.d ) / plane.normal.z;

	//z = imin(TILE_RISE, z);

	return z;
}

#if 1
float Heightmap::accheight(int x, int y)
{
	int tx = x / TILE_SIZE;
	int ty = y / TILE_SIZE;

	if(tx < 0)
		tx = 0;

	if(ty < 0)
		ty = 0;

	if(tx >= width.x)
		tx = width.x-1;

	if(ty >= width.y)
		ty = width.y-1;

	return Bilerp(this, x, y);
	//TODO check if works

	int tileindex = ty*g_mapsz.x + tx;
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(x, 0, y);

	Vec3f tri[3];

	if(PointBehindPlane(point, tridiv[tileindex]))
	{
		tri[0] = origv[ tileindex6v + 0 ];
		tri[1] = origv[ tileindex6v + 1 ];
		tri[2] = origv[ tileindex6v + 2 ];
	}
	else
	{
		tri[0] = origv[ tileindex6v + 3 ];
		tri[1] = origv[ tileindex6v + 4 ];
		tri[2] = origv[ tileindex6v + 5 ];
	}

	Plane3f plane;

	Vec3f trinorm = Normal(tri);

	MakePlane(&plane.normal, &plane.d, tri[0], trinorm);

	float z = - ( x*plane.normal.x + y*plane.normal.y + plane.d ) / plane.normal.z;

	return z;
}
#else
float Heightmap::accheight(int x, int y)
{
	int tx = x / TILE_SIZE;
	int ty = y / TILE_SIZE;

#if 0
	if(tx >= g_hmap.widthx)
		tx = g_hmap.widthx-1;

	if(ty >= g_hmap.widthy)
		ty = g_hmap.widthy-1;
#endif

	if(tx < 0)
		tx = 0;

	if(ty < 0)
		ty = 0;

	if(tx >= g_mapsz.x)
		tx = g_mapsz.x-1;

	if(ty >= g_mapsz.y)
		ty = g_mapsz.y-1;

	int tileindex = ty*g_mapsz.x + tx;
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(x, 0, y);

	Vec3f tri[3];

	if(PointBehindPlane(point, tridiv[tileindex]))
	{
		tri[0] = origv[ tileindex6v + 0 ];
		tri[1] = origv[ tileindex6v + 1 ];
		tri[2] = origv[ tileindex6v + 2 ];
	}
	else
	{
		tri[0] = origv[ tileindex6v + 3 ];
		tri[1] = origv[ tileindex6v + 4 ];
		tri[2] = origv[ tileindex6v + 5 ];
	}

	Plane3f plane;

	Vec3f trinorm = Normal(tri);

#if 0
	g_applog<<"trinorm = "<<trinorm.x<<","<<trinorm.y<<","<<trinorm.z<<std::endl;
#endif

	MakePlane(&plane.normal, &plane.d, tri[0], trinorm);

#if 0
	g_applog<<"tri0 = "<<tri[0].x<<","<<tri[0].y<<","<<tri[0].z<<std::endl;
	g_applog<<"tri1 = "<<tri[1].x<<","<<tri[1].y<<","<<tri[1].z<<std::endl;
	g_applog<<"tri2 = "<<tri[2].x<<","<<tri[2].y<<","<<tri[2].z<<std::endl;
#endif

	float z = - ( x*plane.normal.x + y*plane.normal.z + plane.d ) / plane.normal.y;

#if 0
	g_applog<<"- ( "<<x<<"*"<<plane.normal.x<<" + "<<z<<"*"<<plane.normal.z<<" + "<<plane.d<<" ) / "<<plane.normal.y<<" = "<<std::endl;

	g_applog<<"y = "<<y<<std::endl;
#endif

	//if(GetCd(CD_ROAD, tx, ty, ecfalse)->on)
	//	z += TILE_SIZE/30;

	return z;
}
#endif

Vec3f Heightmap::getnormal(int x, int y)
{
	return norm[ (y * g_mapsz.x + x) * 6 ];
}

void Heightmap::lowereven()
{
    
#if 1
    ecbool changed = ecfalse;
    
    do
    {
        changed = ecfalse;
        
        for(int x=0; x<g_mapsz.x; x++)
            for(int y=0; y<g_mapsz.y; y++)
            {
                unsigned short h0 = getheight(x, y);
                unsigned short h1 = getheight(x+1, y);
                unsigned short h2 = getheight(x+1, y+1);
                unsigned short h3 = getheight(x, y+1);
                
                unsigned int minh = imin(h0, imin(h1, imin(h2, h3)));
                
                if(h0 > minh+1)
                {
                    changed = ectrue;
                    adjheight(x, y, -1);
                }
                
                if(h1 > minh+1)
                {
                    changed = ectrue;
                    adjheight(x+1, y, -1);
                }
                
                if(h2 > minh+1)
                {
                    changed = ectrue;
                    adjheight(x+1, y+1, -1);
                }
                
                if(h3 > minh+1)
                {
                    changed = ectrue;
                    adjheight(x, y+1, -1);
                }
            }
        
    }while(changed);
#endif
}

void Heightmap::highereven()
{
#if 1
    ecbool changed = ecfalse;
    
    do
    {
        changed = ecfalse;
        
        for(int x=0; x<g_mapsz.x; x++)
            for(int y=0; y<g_mapsz.y; y++)
            {
                unsigned short h0 = getheight(x, y);
                unsigned short h1 = getheight(x+1, y);
                unsigned short h2 = getheight(x+1, y+1);
                unsigned short h3 = getheight(x, y+1);
                
                unsigned int maxh = imax(h0, imax(h1, imax(h2, h3)));
                
                if(h0 < maxh-1)
                {
                    changed = ectrue;
                    adjheight(x, y, +1);
                }
                
                if(h1 < maxh-1)
                {
                    changed = ectrue;
                    adjheight(x+1, y, +1);
                }
                
                if(h2 < maxh-1)
                {
                    changed = ectrue;
                    adjheight(x+1, y+1, +1);
                }
                
                if(h3 < maxh-1)
                {
                    changed = ectrue;
                    adjheight(x, y+1, +1);
                }
            }
        
    }while(changed);
#endif
}

// TODO simplify, break up into simpler functions, refactor remesh() code.
void PackNorms(std::vector<Vec3f**> *addedvertnormals, int from6v, int to6v)
{
    
}

/*
Regenerate the mesh vertices (origv) and normals (norm) from the height points.
Texture coordinates (texco) will also be generated.
*/
void Heightmap::remesh()
{
	for(int x=0; x<this->width.x; x++)
		for(int y=0; y<this->width.y; y++)
		{
			unsigned char h0 = getheight(x, y);
			unsigned char h1 = getheight(x+1, y);
			unsigned char h2 = getheight(x+1, y+1);
			unsigned char h3 = getheight(x, y+1);

			//Log("height after "<<(unsigned int)h0);

			//Log("("<<(int)h0<<","<<(int)h1<<","<<(int)h2<<","<<(int)h3<<")");

			unsigned char minh = imin(h0, imin(h1, imin(h2, h3)));

			ecbool u0 = h0 > minh;
			ecbool u1 = h1 > minh;
			ecbool u2 = h2 > minh;
			ecbool u3 = h3 > minh;

			if(!u0 && !u1 && !u2 && !u3)	SurfTile(x,y,this).incltype = IN_0000;
			if(!u0 && !u1 && !u2 && u3)		SurfTile(x,y,this).incltype = IN_0001;
			if(!u0 && !u1 && u2 && !u3)		SurfTile(x,y,this).incltype = IN_0010;
			if(!u0 && !u1 && u2 && u3)		SurfTile(x,y,this).incltype = IN_0011;
			if(!u0 && u1 && !u2 && !u3)		SurfTile(x,y,this).incltype = IN_0100;
			if(!u0 && u1 && !u2 && u3)		SurfTile(x,y,this).incltype = IN_0101;
			if(!u0 && u1 && u2 && !u3)		SurfTile(x,y,this).incltype = IN_0110;
			if(!u0 && u1 && u2 && u3)		SurfTile(x,y,this).incltype = IN_0111;
			if(u0 && !u1 && !u2 && !u3)		SurfTile(x,y,this).incltype = IN_1000;
			if(u0 && !u1 && !u2 && u3)		SurfTile(x,y,this).incltype = IN_1001;
			if(u0 && !u1 && u2 && !u3)		SurfTile(x,y,this).incltype = IN_1010;
			if(u0 && !u1 && u2 && u3)		SurfTile(x,y,this).incltype = IN_1011;
			if(u0 && u1 && !u2 && !u3)		SurfTile(x,y,this).incltype = IN_1100;
			if(u0 && u1 && !u2 && u3)		SurfTile(x,y,this).incltype = IN_1101;
			if(u0 && u1 && u2 && !u3)		SurfTile(x,y,this).incltype = IN_1110;

			SurfTile(x,y,this).elev = minh;
		}
  
		float tilescale = 1;

	/*
	These are the vertices of the tile.

	(0,0)      (1,0)
	c or 2     d or 3
	------------
	|          |
	|          |
	|          |
	|__________|
	b or 1     a or 0
	(0,1)      (1,1)
	*/

	Vec3f a, b, c, d;	// tile corner vertices
	Vec3f norm0, norm1;	// the two tile triangle normals
	Vec3f tri0[3];	//  first tile triangle
	Vec3f tri1[3];	// second tile triangle
	float heights[4];
	float aD, bD, cD, dD;

	// The numbers here indicate whether the vertex in
	// question is involved, counting from the top,left (0,0)
	// corner going clockwise.
	float diag1010;	// height difference along diagonal a-to-c
	float diag0101;	// height difference along diagonal b-to-d

	/*
	We will blend the normals of all the vertices with 4 neighbouring
	tiles, so that the terrain doesn't look blocky.

	Each tile has 6 vertices for two triangles.

	We need pointers for this because we construct the normals as we go along,
	and only blend them in the end.
	*/
	std::vector<Vec3f**> *addedvertnormals = new std::vector<Vec3f**>[ this->width.x * this->width.y * 6 ];

	if(!addedvertnormals) OUTOFMEM();

	/*
	Because triangles will alternate, we need to keep an ordered std::list
	for the tile corner vertex normals for each tile, for
	figuring out the corner vertex normals of neighbouring tiles.
	*/
	struct TileNormals
	{
		/*
		Depending on the triangle configuration,
		there might be two normals at a corner,
		one for either triangle.
		*/
		Vec3f *normal_a_rightmost;
		Vec3f *normal_a_bottommost;
		Vec3f *normal_b_leftmost;
		Vec3f *normal_b_bottommost;
		Vec3f *normal_c_leftmost;
		Vec3f *normal_c_topmost;
		Vec3f *normal_d_topmost;
		Vec3f *normal_d_rightmost;
	};


	/*
	(0,0)      (1,0)
	c or 2     d or 3
	------------
	|          |
	|          |
	|          |
	|__________|
	b or 1     a or 0
	(0,1)      (1,1)
	*/

	TileNormals *tilenormals = new TileNormals[ this->width.x * this->width.y ];

	if(!tilenormals) OUTOFMEM();

	for(int x=0; x<this->width.x; x++)
		for(int y=0; y<this->width.y; y++)
		{
			heights[0] = getheight(x+1, y+1) * TILE_RISE;
			heights[1] = getheight(x, y+1) * TILE_RISE;
			heights[2] = getheight(x, y) * TILE_RISE;
			heights[3] = getheight(x+1, y) * TILE_RISE;

			/*
			These are the vertices of the tile.

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|          |
			|          |
			|          |
			|__________|
			b or 1     a or 0
			(0,1)      (1,1)

			We need to decide which way the diagonal (shared edge of the
			triangles) will be, because it doesn't look nice where the
			land meets the water if we don't alternate them according
			to which diagonal has the greater height difference. And also,
			it doesn't look good in general anywhere where there's
			mountains or ridges.

			What we're going to do here is give smoothness to changing
			slopes, instead of hard corners.

			Imagine if there's a really strong difference in height
			along the diagonal c-to-a.

			If we break the tile into triangles like this,

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			| \        |
			|    \     |
			|      \   |
			|________\_|
			b or 1     a or 0
			(0,1)      (1,1)

			Then there will be a sharp corner at vertex a.

			However, if we cut the tile into triangles like this,


			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|        / |
			|      /   |
			|    /     |
			|_/________|
			b or 1     a or 0
			(0,1)      (1,1)

			then this will be a bevelled corner.

			Combined with other tiles this will look like:

					  |
					  |
					  |
					 /
				   /
			_____/

			Otherwise, if we didn't check which way
			to cut the tile into triangles, it would look
			like a sharp corner:

					  |
					  |
					  |
			__________|

			*/

			a = Vec3f( (x+1)*TILE_SIZE*tilescale, heights[0], (y+1)*TILE_SIZE*tilescale );
			b = Vec3f( (x)*TILE_SIZE*tilescale, heights[1], (y+1)*TILE_SIZE*tilescale );
			c = Vec3f( (x)*TILE_SIZE*tilescale, heights[2], (y)*TILE_SIZE*tilescale );
			d = Vec3f( (x+1)*TILE_SIZE*tilescale, heights[3], (y)*TILE_SIZE*tilescale );

			/*
			Get the difference between each tile vertex
			and the average of the two neighbouring
			(diagonal) vertices.
			*/

			aD = fabsf( heights[0] - (heights[1]+heights[3])/2.0f );
			bD = fabsf( heights[1] - (heights[2]+heights[0])/2.0f );
			cD = fabsf( heights[2] - (heights[3]+heights[1])/2.0f );
			dD = fabsf( heights[3] - (heights[0]+heights[2])/2.0f );

			/*
			For either of the two possible diagonals,
			get the maximum difference, to see
			which diagonal is steeper.
			*/

			diag1010 = fmax( aD, cD );
			diag0101 = fmax( bD, dD );

			/*
			If diagonal a-to-c has a greater
			height difference,

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|          |
			|          |
			|          |
			|__________|
			b or 1     a or 0
			(0,1)      (1,1)

			triangle 1 will be made of vertices
			a, b, and d, and triangle 2 will be
			made of vertices b, c, d.
			*/

			int tileindex6v = (y * this->width.x + x) * 3 * 2;
			int tileindex = (y * this->width.x + x);
			
			//if(diag1010 > diag0101)
			//if(ectrue)	//we only want this tri config for sprite editor
			if(ecfalse)	//use 1 (next)
			{
				tricfg[ tileindex ] = 0;

				Vec3f divnorm = Cross(Normalize(b-d), Vec3f(0,1,0));
				//divnorm = Cross(Normalize(b-d), Vec3f(0,1,0));
				MakePlane(&tridiv[ tileindex ].normal, &tridiv[ tileindex ].d, Vec3f(((float)x+0.5f)*TILE_SIZE, 0, ((float)y+0.5f)*TILE_SIZE), divnorm);

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|        / |
				|      /   |
				|    /     |
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)
				*/

				origv[ tileindex6v + 0 ] = a;
				origv[ tileindex6v + 1 ] = b;
				origv[ tileindex6v + 2 ] = d;
				origv[ tileindex6v + 3 ] = b;
				origv[ tileindex6v + 4 ] = c;
				origv[ tileindex6v + 5 ] = d;


				// Need triangles to figure out
				// the tile normals.
				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = d;
				tri1[0] = b;
				tri1[1] = c;
				tri1[2] = d;

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d

				Now we have to remember that the triangle indices don't correspond to the vertex numbers;
				there's 6 triangle vertices (2 repeated) and only 4 unique tile corner vertices.
				*/

				tilenormals[ tileindex ].normal_a_rightmost = &norm[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_a_bottommost = &norm[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_b_leftmost = &norm[ tileindex6v + 3 ];
				tilenormals[ tileindex ].normal_b_bottommost = &norm[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_c_leftmost = &norm[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_c_topmost = &norm[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_d_topmost = &norm[ tileindex6v + 5 ];
				tilenormals[ tileindex ].normal_d_rightmost = &norm[ tileindex6v + 2 ];

				// Add the normals for this tile itself
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_bottommost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_d_topmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_b_bottommost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_d_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );

				//If there's a tile in the x-1 direction, add its normal to corners c(2) and b(1).
				// c(2) is the vertex index 4 of the two triangle vertices.
				// b(1) is the vertex index 1 and 3 of the two triangle vertices.
				if(x > 0)
				{
					int nearbytileindex = (y * this->width.x + (x-1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 4 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_topmost );

					// vertex 1 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_bottommost );

					// vertex 3 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}


				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the x+1 direction, add its normal to corners d(3) and a(0).
				// d(3) is the vertex index 2 and 5 of the two triangles vertices.
				// a(0) is the vertex index 0 of the two triangles.
				if(x < this->width.x-1)
				{
					int nearbytileindex = (y * this->width.x + (x+1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_c_topmost );

					// vertex 5 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_c_topmost );

					// vertex 0 of the triangles (corner a(0)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the y-1 direction, add its normal to corners c(2) and d(3).
				// c(2) is the vertex index 4 of the two triangle vertices.
				// d(3) is the vertex index 2 and 5 of the two triangle vertices.
				if(y > 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 4 of the triangles (corner c(2)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_leftmost );

					// vertex 2 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_rightmost );

					// vertex 5 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_rightmost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the y+1 direction, add its normal to corners b(1) and a(0).
				// b(1) is the vertex index 1 and 3 of the two triangles vertices.
				// a(0) is the vertex index 0 of the two triangles.
				if(y < this->width.y-1)
				{
					int nearbytileindex = ((y+1) * this->width.x + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_leftmost );

					// vertex 3 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_leftmost );

					// vertex 0 of the triangles (corner a(0)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}

				// We've added the normals of the horizontally and vertically neighbouring tiles.
				// Now we'll add the normals of diagonal tiles.

				//If there's a tile in the x-1,y-1 direction, add its normal to corner c(2).
				// c(2) is the vertex index 4 of the two triangles.
				if(x-1 >= 0 && y-1 >= 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 4 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the x+1,y-1 direction, add its normal to corner d(3).
				// d(3) is the vertex index 2 and 5 of the two triangles.
				if(x+1 < this->width.x && y-1 >= 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2,5 of the triangles (corner d(3)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the x+1,y+1 direction, add its normal to corner a(0).
				// a(0) is the vertex index 0 of the two triangles.
				if(x+1 < this->width.x && y+1 < this->width.y)
				{
					int nearbytileindex = ((y+1) * this->width.x + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 0 of the triangles (corner a(0)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_topmost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the x-1,y+1 direction, add its normal to corner b(1).
				// b(1) is the vertex index 1,3 of the two triangles.
				if(x-1 >= 0 && y+1 < this->width.y)
				{
					int nearbytileindex = ((y+1) * this->width.x + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_rightmost );

					// vertex 3 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}
			}

			/*
			Otherwise, if diagonal d-to-b has a
			greater height difference,

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|  \       |
			|    \     |
			|      \   |
			|________\_|
			b or 1     a or 0
			(0,1)      (1,1)

			triangle 1 will be made of vertices
			a, b, and c, and triangle 2 will be
			made of vertices d, a, c.
			*/

			else
			{
				tricfg[ tileindex ] = 1;

				Vec3f divnorm = Cross(Normalize(c-a), Vec3f(0,1,0));
				MakePlane(&tridiv[ tileindex ].normal, &tridiv[ tileindex ].d, Vec3f(((float)x+0.5f)*TILE_SIZE, 0, ((float)y+0.5f)*TILE_SIZE), divnorm);

				origv[ (y * this->width.x + x) * 3 * 2 + 0 ] = a;
				origv[ (y * this->width.x + x) * 3 * 2 + 1 ] = b;
				origv[ (y * this->width.x + x) * 3 * 2 + 2 ] = c;
				origv[ (y * this->width.x + x) * 3 * 2 + 3 ] = d;
				origv[ (y * this->width.x + x) * 3 * 2 + 4 ] = a;
				origv[ (y * this->width.x + x) * 3 * 2 + 5 ] = c;

				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = c;
				tri1[0] = d;
				tri1[1] = a;
				tri1[2] = c;

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				tilenormals[ tileindex ].normal_a_rightmost = &norm[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_a_bottommost = &norm[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_b_leftmost = &norm[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_b_bottommost = &norm[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_c_leftmost = &norm[ tileindex6v + 2 ];
				tilenormals[ tileindex ].normal_c_topmost = &norm[ tileindex6v + 5 ];
				tilenormals[ tileindex ].normal_d_topmost = &norm[ tileindex6v + 3 ];
				tilenormals[ tileindex ].normal_d_rightmost = &norm[ tileindex6v + 3 ];

				// Add the normals for this tile itself
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_bottommost );
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_c_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_a_bottommost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_c_leftmost );

				//If there's a tile in the x-1 direction, add its normal to corners c(2) and b(1).
				// c(2) is the vertex index 2 and 5 of the two triangle vertices.
				// b(1) is the vertex index 1 of the two triangle vertices.
				if(x > 0)
				{
					int nearbytileindex = (y * this->width.x + (x-1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_d_topmost );

					// vertex 5 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_d_topmost );

					// vertex 1 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the x+1 direction, add its normal to corners d(3) and a(0).
				// d(3) is the vertex index 3 of the two triangles vertices.
				// a(0) is the vertex index 0 and 4 of the two triangles.
				if(x < this->width.x-1)
				{
					int nearbytileindex = (y * this->width.x + (x+1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 3 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_topmost );

					// vertex 0 of the triangles (corner a(0)) is corner b(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_bottommost );

					// vertex 4 of the triangles (corner a(0)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the y-1 direction, add its normal to corners c(2) and d(3).
				// c(2) is the vertex index 2 and 5 of the two triangle vertices.
				// d(3) is the vertex index 3 of the two triangle vertices.
				if(y > 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2 of the triangles (corner c(2)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_leftmost );

					// vertex 5 of the triangles (corner c(2)) is corner b(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_leftmost );

					// vertex 3 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_rightmost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the y+1 direction, add its normal to corners b(1) and a(0).
				// b(1) is the vertex index 1 of the two triangles vertices.
				// a(0) is the vertex index 0 and 4 of the two triangles.
				if(y < this->width.y-1)
				{
					int nearbytileindex = ((y+1) * this->width.x + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_leftmost );

					// vertex 0 of the triangles (corner a(1)) is corner d(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_rightmost );

					// vertex 4 of the triangles (corner a(0)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}

				// We've added the normals of the horizontally and vertically neighbouring tiles.
				// Now we'll add the normals of diagonal tiles.

				//If there's a tile in the x-1,y-1 direction, add its normal to corner c(2).
				// c(2) is the vertex index 2,5 of the two triangles.
				if(x-1 >= 0 && y-1 >= 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_bottommost );

					// vertex 5 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the x+1,y-1 direction, add its normal to corner d(3).
				// d(3) is the vertex index 3 of the two triangles.
				if(x+1 < this->width.x && y-1 >= 0)
				{
					int nearbytileindex = ((y-1) * this->width.x + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 3 of the triangles (corner d(3)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the x+1,y+1 direction, add its normal to corner a(0).
				// a(0) is the vertex index 0,4 of the two triangles.
				if(x+1 < this->width.x && y+1 < this->width.y)
				{
					int nearbytileindex = ((y+1) * this->width.x + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 0,4 of the triangles (corner a(0)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_c_topmost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the x-1,y+1 direction, add its normal to corner b(1).
				// b(1) is the vertex index 1 of the two triangles.
				if(x-1 >= 0 && y+1 < this->width.y)
				{
					int nearbytileindex = ((y+1) * this->width.x + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}
			}

			// Generate the texture coordinates based on world position of vertices.
			for(int i=0; i<6; i++)
			{
				texco[ (y * this->width.x + x) * 3 * 2 + i ].x = origv[ (y * this->width.x + x) * 3 * 2 + i ].x / TILE_SIZE;
				texco[ (y * this->width.x + x) * 3 * 2 + i ].y = origv[ (y * this->width.x + x) * 3 * 2 + i ].z / TILE_SIZE;
			}

			// Triangle normals.
			norm0 = Normal(tri0);
			norm1 = Normal(tri1);

			// These are temporary normals before we blend them between tiles.
			norm[ (y * this->width.x + x) * 3 * 2 + 0 ] = norm0;
			norm[ (y * this->width.x + x) * 3 * 2 + 1 ] = norm0;
			norm[ (y * this->width.x + x) * 3 * 2 + 2 ] = norm0;
			norm[ (y * this->width.x + x) * 3 * 2 + 3 ] = norm1;
			norm[ (y * this->width.x + x) * 3 * 2 + 4 ] = norm1;
			norm[ (y * this->width.x + x) * 3 * 2 + 5 ] = norm1;
		}

	Vec3f* tempnormals = new Vec3f[ this->width.x * this->width.y * 3 * 2 ];

	if(!tempnormals) OUTOFMEM();

#if 0
	// Average the added up normals and store them in tempnormals.
	for(int x=0; x<this->width.x; x++)
		for(int y=0; y<this->width.y; y++)
			for(int trivert = 0; trivert < 6; trivert++)
			{
				int tileindex6v = (y * this->width.x + x) * 3 * 2 + trivert;
				int tileindex = (y * this->width.x + x);
				std::vector<Vec3f**> vertexnormals = addedvertnormals[ tileindex6v ];

				// Before we add up the added normals, we make sure we don't repeat
				// any normals, since for diagonals we add the two normals for
				// the two possible triangles. Now that we have the triangle
				// configuration, we know which corners have one triangle and
				// which have two.
				// Remove repeating added normals.
#if 1
				for(std::list<Widget*>::iterator normiter = vertexnormals.begin(); normiter != vertexnormals.end(); normiter++)
				{
					std::list<Widget*>::iterator normiter2 = normiter+1;
					while(normiter2 != vertexnormals.end())
					{
						// If both pointers are pointing to the same normal std::vector, remove the second copy.
						if(**normiter2 == **normiter)
						{
							normiter2 = vertexnormals.erase( normiter2 );
							continue;
						}

						normiter2++;
					}
				}
#endif

				Vec3f finalnormal(0,0,0);

				// Average all the added normals for each tile vertex

				for(int i=0; i<vertexnormals.size(); i++)
					finalnormal = finalnormal + **(vertexnormals[i]);

				if(vertexnormals.size() <= 0)
					continue;

				tempnormals[ tileindex6v ] = Normalize( finalnormal / (float)vertexnormals.size() );
			}

	// Transfer the normals from tempnormals to norms.
	for(int x=0; x<this->width.x; x++)
		for(int y=0; y<this->width.y; y++)
			for(int trivert = 0; trivert < 6; trivert ++)
			{
				int tileindex6v = (y * this->width.x + x) * 3 * 2 + trivert;
				norm[ tileindex6v ] = tempnormals[ tileindex6v ];
				origv[ tileindex6v ] = origv[ tileindex6v ];
			}
#else
#if 1
	for(int x=0; x<this->width.x; x++)
		for(int y=0; y<this->width.y; y++)
		{
			for(int trivert = 0; trivert < 6; trivert ++)
			{
				int tileindex6v = (y * this->width.x + x) * 3 * 2 + trivert;
				//origv[ tileindex6v ] = origv[ tileindex6v ];
				float temp = origv[ tileindex6v ].y;
				origv[ tileindex6v ].y = origv[ tileindex6v ].z;
				origv[ tileindex6v ].z = temp;

				temp = norm[ tileindex6v ].y;
				norm[ tileindex6v ].y = norm[ tileindex6v ].z;
				norm[ tileindex6v ].z = temp;
			}
			
			int tileindex = (y * this->width.x + x);

			float temp = tridiv[ tileindex ].normal.y;
			tridiv[ tileindex ].normal.y = tridiv[ tileindex ].normal.z;
			tridiv[ tileindex ].normal.z = temp;
		}
#endif
#endif

	delete [] tempnormals;
	delete [] addedvertnormals;
	delete [] tilenormals;

}

void Heightmap::draw()
{
	if(g_mapsz.x <= 0 || g_mapsz.y <= 0)
		return;
	//return;
	Shader* s = g_sh+g_curS;

}
