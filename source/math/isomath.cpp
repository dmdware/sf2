




#include "isomath.h"
#include "../sim/map.h"
#include "../platform.h"
#include "3dmath.h"
#include "../utils.h"
#include "fixmath.h"
#include "../render/heightmap.h"
#include "plane3f.h"
#include "../window.h"

//cartesian to isometric
//i.e., 3d to screen
Vec2i CartToIso(Vec3i cmpos)
{
	Vec2i screenpos(0,0);

	screenpos.x += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;

	screenpos.x -= cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;

	//for mv and fine-grained elements that don't need to tile seamlessly
	screenpos.y -= cmpos.z *  ( (int)TILE_PIXEL_RISE * 1000 / (int)TILE_RISE ) / 1000;
	//screenpos.y -= CartZToIso(cmpos.z);

	return screenpos;
}

int CartZToIso(int cmz)
{
	//return cmy / tan(DEGTORAD(30));
	//return cmy * (TILE_PIXEL_WIDTH / 8) / (TILE_SIZE/2);
	//Log("cmy "<<cmy);
	//Log("result "<<(cmy * TILE_PIXEL_RISE / TILE_RISE));
	//ceili is necessary to get rid of 1-pixel-off problem
	//edit: this still happens
	return ceili(cmz * TILE_PIXEL_RISE, (int)TILE_RISE);
	//return cmz * ( (int)TILE_PIXEL_RISE * 1000 / (int)TILE_RISE ) / 1000;
	//return cmz / (int)TILE_RISE * TILE_PIXEL_RISE;
}

Vec2i CartToIso2(Vec3i cmpos)
{
	Vec2i screenpos(0,0);

	screenpos.x += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;

	screenpos.x -= cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;

	//screenpos.y -= cmpos.y * (TILE_PIXEL_WIDTH / 4) / (TILE_SIZE/2);
	screenpos.y -= CartZToIso(cmpos.z);

	return screenpos;
}

//can only process results in intervals of TILE_RISE
//(ie, there won't be smooth 1-by-1 transitions but it will happen in small jumps)
//but it avoids the gaps in map tiles after cycling through several elevation levels
int CartZToIso2(int cmz)
{
	//return cmy / tan(DEGTORAD(30));
	//return cmy * (TILE_PIXEL_WIDTH / 8) / (TILE_SIZE/2);
	//Log("cmy "<<cmy);
	//Log("result "<<(cmy * TILE_PIXEL_RISE / TILE_RISE));
	//ceili is necessary to get rid of 1-pixel-off problem
	//edit: this still happens
	//return ceili(cmz * TILE_PIXEL_RISE, (int)TILE_RISE);
	//return cmz * ( (int)TILE_PIXEL_RISE * 1000 / (int)TILE_RISE ) / 1000;
	return cmz / (int)TILE_RISE * TILE_PIXEL_RISE;
}

void CartToDepth(Vec3i cmpos, int *depth)
{
	//Vec3i top = Vec3i(0,0,0);	//top corner of map
	//Vec3i bot = Vec3i(MAX_MAP*TILE_SIZE,MAX_MAP*TILE_SIZE,255);	//bottom corner of map
	//Vec3i bot = Vec3i(TILE_SIZE,TILE_SIZE,0);	//bottom corner of same tile

	Vec2i pixpos = CartToIso(cmpos);

	Vec3f ray;
	Vec3f point;
	IsoToCart(pixpos, &ray, &point);

	//Plane3f projplane;
	//projplane.d = - MAG_VEC3F( Vec2i(1,1) * TILE_SIZE * MAX_MAP );
	//projplane.d = 0;
	//projplane.normal = ray;

	Vec3f cmposf = Vec3f(
		(float)cmpos.x,
		(float)cmpos.y,
		(float)cmpos.z);

	//ray = Normalize(ray);

	*depth = (int) Dot( ray, cmposf );	//+ (int) MAG_VEC3F( Vec3f(1,1,1) * TILE_SIZE * MAX_MAP );

#if 0
	//origin being bottom most, highest corner of map

	cmposf = Vec3f(
		(float)MAX_MAP*TILE_SIZE,
		(float)MAX_MAP*TILE_SIZE,
		(float)TILE_RISE*10);

	int neardepth = (int) Dot( ray, cmposf );

	//Log("dpeth: "<<*depth<<" neardepth:"<<neardepth<<" final:"<<(*depth-neardepth));

	*depth -= neardepth;

#elif 0
	//origin being bottom most, highest corner of scrolled view

	IsoToCart(g_scroll + Vec2i(g_width,g_height), &ray, &point);

	ray = Normalize(ray);

	float highz = TILE_RISE * 10;
	float offz = highz - point.z;
	float ratioz = offz / ray.z;

	//edit: must be done before rendering, because rendz isn't updated when screen scrolled for fixed objects

#endif

	//float offmaxx = TILE_SIZE * MAX_MAP - 
}

//isometric to cartesian
//i.e., screen pixel coords to 3d coords
void IsoToCart(Vec2i pixpos, Vec3f *ray, Vec3f *point)
{
	//*ray = Vec3f(0,0,0);
	//*point = Vec3f(0,0,0);

	Vec3i point1 = Vec3i(-TILE_SIZE,-TILE_SIZE,-TILE_RISE);
	//An offset of (+X,+X) where X is an equal, positive amount on x- and y- axis
	//in the isometric plane will be vertically on the same line on the screen in pixels.
	Vec3i point2 = Vec3i(TILE_SIZE,TILE_SIZE,0);	//must have same screen pixel pos
	//solve for point2.z
	Vec2i pix1 = CartToIso(point1);

	//We use the last line of CartToIso, that
	//screenpos.y -= cmpos.z * TILE_PIXEL_RISE / TILE_RISE;
	//so we can
	//cmpos.z -= screenpos.y * TILE_RISE / TILE_PIXEL_RISE;
	//since (100,100) will be lower on the screen in pixels, it makes sense that we
	//must subtract to reposition it at the same point as point1 (0,0,0).
	point2.z = (int)((float)point1.z - (float)pix1.y * (float)TILE_RISE / (float)TILE_PIXEL_RISE);

	//Log("point2.z="<<point2.z<<" point1.z="<<point1.z<<" pix1.y="<<pix1.y);

	*ray = Vec3f((float)point1.x, (float)point1.y, (float)point1.z) - Vec3f((float)point2.x, (float)point2.y, (float)point2.z);
	//*ray = Vec3f((float)point2.x, (float)point2.y, (float)point2.z) - Vec3f((float)point1.x, (float)point1.y, (float)point1.z);
	*ray = Normalize( *ray );

	point->z = 0.0f;
	//point->y = - (float)pixpos.x / (float)TILE_PIXEL_WIDTH / (float)2 / (float)TILE_SIZE;
	//point->x = (float)pixpos.x / ((float)TILE_PIXEL_WIDTH / (float)2 / (float)TILE_SIZE) + point->y;
	//point->x = ( (float)pixpos.y - point->y * (float)TILE_PIXEL_WIDTH / (float)2 / (float)TILE_SIZE / (float)2 ) / ( (float)TILE_PIXEL_WIDTH / (float)2 / (float)TILE_SIZE / (float)2 );

	const float A = (float)TILE_PIXEL_WIDTH / (float)2 / (float)TILE_SIZE;
	point->x = (2.0f * (float)pixpos.y / A + (float)pixpos.x / A) / 2.0f;
	point->y = (2.0f * (float)pixpos.y / A - (float)pixpos.x / A) / 2.0f;

	/*
	http://gamedevelopment.tutsplus.com/tutorials/creating-isometric-worlds-a-primer-for-game-developers--gamedev-6511

	//Cartesian to isometric:

	isoX = cartX - cartY;
	isoY = (cartX + cartY) / 2;

	//Isometric to Cartesian:

	cartX = (2 * isoY + isoX) / 2;
	cartY = (2 * isoY - isoX) / 2;
	*/


	/*
	Proof

	1) screenpos.x = cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	2) screenpos.y = cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 - cmz * TILE_PIXEL_RISE / TILE_RISE;

	1->3) cmpos.x = ( screenpos.x + cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE ) / (TILE_PIXEL_WIDTH / 2 / TILE_SIZE);
	2->4) cmpos.x = ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.z * TILE_PIXEL_RISE / TILE_RISE ) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 );

	3+4->5) ( screenpos.x + cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE ) / (TILE_PIXEL_WIDTH / 2 / TILE_SIZE) =
	( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.z * TILE_PIXEL_RISE / TILE_RISE ) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 )

	5->6) ( screenpos.x + cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE ) / ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.z * TILE_PIXEL_RISE / TILE_RISE ) =
	(TILE_PIXEL_WIDTH / 2 / TILE_SIZE) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 );

	6->7) screenpos.x / ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.z * TILE_PIXEL_RISE / TILE_RISE )
	+ ( cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE ) / ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 + cmpos.z * TILE_PIXEL_RISE / TILE_RISE )
	= (TILE_PIXEL_WIDTH / 2 / TILE_SIZE) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 );

	Assume cmpos.z = 0
	Isolate cmpos.y

	7->8) screenpos.x / ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 )
	+ ( cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE ) / ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 )
	= (TILE_PIXEL_WIDTH / 2 / TILE_SIZE) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 );

	Set A = TILE_PIXEL_WIDTH / 2 / TILE_SIZE

	8->9) screenpos.x / ( screenpos.y - cmpos.y * A / 2 ) + ( cmpos.y * A ) / ( screenpos.y - cmpos.y * A / 2 ) = (A) / ( A / 2 );

	9->10) ( screenpos.x + ( cmpos.y * A ) ) / ( screenpos.y - cmpos.y * A / 2 ) = 2;

	10->11) ( screenpos.x + ( cmpos.y * A ) ) / ( screenpos.y * 2 - cmpos.y * A ) = 0;

	11->12) ( screenpos.x + ( cmpos.y * A ) ) * ( screenpos.y

	//too hard

	Wrong:

	11->12) screenpos.x + ( cmpos.y * A ) = 0;

	Solutions:

	12->13) cmpos.y = - screenpos.x / TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	wrong//3->14) cmpos.x = screenpos.x / (TILE_PIXEL_WIDTH / 2 / TILE_SIZE) + cmpos.y;
	4->15) cmpos.x = ( screenpos.y - cmpos.y * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 ) / ( TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2 );
	*/
}
