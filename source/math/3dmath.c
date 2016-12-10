










#include "3dmath.h"
#include "../utils.h"
#include "physics.h"
#include "../window.h"
#include "../gui/gui.h"
#include "vec3f.h"
#include "camera.h"
#include "quaternion.h"
#include "vec4f.h"
#include "matrix.h"
#include "vec3i.h"
#include "plane3f.h"
#include "../window.h"
#include "../sim/player.h"
#include "fixmath.h"


int Clipi(int n, int lower, int upper)
{
	return imax(lower, imin(n, upper));
}


Vec2f Normalize(Vec2f v)
{
	float magnitude = MAG_VEC2F(v);
	v.x /= magnitude;
	v.y /= magnitude;
	return v;
}

Vec3f Cross(Vec3f a, Vec3f b)
{
	Vec3f n;
	n.x = ((a.y * b.z) - (a.z * b.y));
	n.y = ((a.z * b.x) - (a.x * b.z));
	n.z = ((a.x * b.y) - (a.y * b.x));
	return n;
}


Vec3f Vector(Vec3f a, Vec3f b)
{
	Vec3f v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	v.z = a.z - b.z;
	return v;
}

// Clockwise
Vec3f Normal(Vec3f tri[])
{
	Vec3f a, b, n;
	a = Vector(tri[2], tri[0]);
	b = Vector(tri[1], tri[0]);
	n = Cross(a, b);
	n = Normalize(n);
	return n;
}

// Counter-clockwise
Vec3f Normal2(Vec3f tri[])
{
	Vec3f a, b, n;
	a = Vector(tri[2], tri[0]);
	b = Vector(tri[1], tri[0]);
	n = Cross(b, a);
	n = Normalize(n);
	return n;
}

ecbool InterPlane(Vec3f poly[], Vec3f line[], Vec3f *n, float *origdist)
{
	float distance1 = 0, distance2 = 0;
	*n = Normal(poly);
	*origdist = PlaneDistance(*n, poly[0]);
	distance1 = ((n->x * line[0].x)  +					// Ax +
		(n->y * line[0].y)  +					// Bx +
		(n->z * line[0].z)) + *origdist;	// Cz + D
	distance2 = ((n->x * line[1].x)  +					// Ax +
		(n->y * line[1].y)  +					// Bx +
		(n->z * line[1].z)) + *origdist;	// Cz + D
	if(distance1 * distance2 >= 0)
		return ecfalse;
	return ectrue;
}

ecbool Intersection(Vec3f l0, Vec3f l, Plane3f p, Vec3f* inter)
{
	Vec3f p0;
	float num, denom;
	denom = Dot(l, p.normal);
	if(fabs(denom) <= EPSILON/2.0f)
		//if(denom == 0.0f)
		return ecfalse;
	p0 = PointOnPlane(p);
	num = Dot(p0 - l0, p.normal);
	*inter = l0 + l * (num/denom);
	return ectrue;
}

float Dot(Vec3f n)
{
	return (n.x * n.x) + (n.y * n.y) + (n.z * n.z);
}

float Dot(Vec3f a, Vec3f b)
{
	return ( (a.x * b.x) + (a.y * b.y) + (a.z * b.z) );
}

int Dot(Vec3i a, Vec3i b)
{
	return ( (a.x * b.x) + (a.y * b.y) + (a.z * b.z) );
}

int Dot(Vec2i a, Vec2i b)
{
	return ( (a.x * b.x) + (a.y * b.y) );
}

double AngleBetweenVectors(Vec3f a, Vec3f b)
{
	/* todo use fast sqrtf custom */
	float dot, vecmag, angle;
	dot = Dot(a, b);
	vecmag = MAG_VEC3F(a) * MAG_VEC3F(b) ;
	angle = acos( dotProduct / vectorsMagnitude );
	/* if(_isnan(angle))
	return 0; */
	return angle;
}

ecbool WithinYaw(Camera* c, Vec3f p, float angle)
{
	Vec3f d;
	float yaw, yaw2, yaw3;
	d = p - c->pos;
	yaw = GetYaw(d.x, d.z);
	yaw2 = yaw - DEGTORAD(360.0f);
	yaw3 = yaw + DEGTORAD(360.0f);
	if(fabs(c->yaw() - yaw) <= angle || fabs(c->yaw() - yaw2) <= angle || fabs(c->yaw() - yaw3) <= angle)
		return ectrue;
	return ecfalse;
}

float DYaw(Camera* c, Vec3f p)
{
	Vec3f d;
	float yaw, yaw2, yaw3;
	float dyaw, dyaw2, dyaw3;
	float mindyaw;
	d = p - c->pos;
	yaw = GetYaw(d.x, d.z);
	yaw2 = yaw - DEGTORAD(360.0f);
	yaw3 = yaw + DEGTORAD(360.0f);
	dyaw = yaw - c->yaw();
	dyaw2 = yaw2 - c->yaw();
	dyaw3 = yaw3 - c->yaw();
	mindyaw = dyaw;
	if(fabs(dyaw2) < fabs(mindyaw))
		mindyaw = dyaw2;
	if(fabs(dyaw3) < fabs(mindyaw))
		mindyaw = dyaw3;
	return mindyaw;
}

Vec3f IntersectionPoint(Vec3f n, Vec3f l[], float distance)
{
	Vec3f p;
	Vec3f ldir;
	float num, denom, dist;
	num = 0.0; denom = 0.0; dist = 0.0;

	ldir = Vector(l[1], l[0]);
	ldir = Normalize(ldir);
	num = - (n.x * l[0].x +	
		n.y * l[0].y +
		n.z * l[0].z + distance);
	denom = Dot(n, ldir);

	/* if(_isnan(denom))
	return l[0]; */

	dist = num / denom;

	p.x = (float)(l[0].x + (ldir.x * dist));
	p.y = (float)(l[0].y + (ldir.y * dist));
	p.z = (float)(l[0].z + (ldir.z * dist));

	return p;
}

Vec3f Rotate(Vec3f v, float rad, float x, float y, float z)
{
	Vec3f newv;
	float costheta, sintheta;

	costheta = (float)cos(rad);
	sintheta = (float)sin(rad);

	newv.x  = (costheta + (1 - costheta) * x * x)		* v.x;
	newv.x += ((1 - costheta) * x * y - z * sintheta)	* v.y;
	newv.x += ((1 - costheta) * x * z + y * sintheta)	* v.z;

	newv.y  = ((1 - costheta) * x * y + z * sintheta)	* v.x;
	newv.y += (costheta + (1 - costheta) * y * y)		* v.y;
	newv.y += ((1 - costheta) * y * z - x * sintheta)	* v.z;

	newv.z  = ((1 - costheta) * x * z - y * sintheta)	* v.x;
	newv.z += ((1 - costheta) * y * z + x * sintheta)	* v.y;
	newv.z += (costheta + (1 - costheta) * z * z)		* v.z;

	return newv;
}

Vec3f RotateAround(Vec3f v, Vec3f around, float rad, float x, float y, float z)
{
	v = v - around;
	v = Rotate(v, rad, x, y, z);
	v = around + v;
	return v;
}

float GetYaw(float dx, float dz)
{
	/* todo correct for use rad */
	return atan2(dx, dz);
}

void LookAt(float* m,
			float eyex, float eyey, float eyez,
			float centerx, float centery, float centerz,
			float upx, float upy, float upz)
{
	float x[3], y[3], z[3];
	float mag;
	float m2[16];

	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag)            /* mpichler, 19950515 */
	{
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	* non-perpendicular unit-length vectors; so normalize x, y here
	*/

	mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag)
	{
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag)
	{
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

#define M(row,col)  m[col*4+row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M

	Mat_tran(m2, -eyex, -eyey, -eyez);
	Mat_postmult(m, m2);
}

void OrthoProj(float* m,
			   float l, float r, float t, float b, float n, float f)
{
#define M(row,col)  m[col*4+row]
	M(0, 0) = 2 / (r - l);
	M(0, 1) = 0;
	M(0, 2) = 0;
	M(0, 3) = 0;

	M(1, 0) = 0;
	M(1, 1) = 2 / (t - b);
	M(1, 2) = 0;
	M(1, 3) = 0;

	M(2, 0) = 0;
	M(2, 1) = 0;
	//M(2, 2) = -1 / (f - n);
	M(2, 2) = -2 / (f - n);
	M(2, 3) = 0;

	M(3, 0) = -(r + l) / (r - l);
	M(3, 1) = -(t + b) / (t - b);
	//M(3, 2) = -n / (f - n);
	M(3, 2) = -(f + n) / (f - n);
	M(3, 3) = 1;
#undef M
}

/*rad fov
#define PI_OVER_360		(M_PI/360.0f)
*/
void PerspProj(float* m,
			   float fov, float aspect, float znear, float zfar)
{
	float xymax, ymin, xmin;
	float width, height;
	float depth, q,qn;
	float w, h;

	xymax = znear * tan(fov/2.0f);
	ymin = -xymax;
	xmin = -xymax;
	width = xymax - xmin;
	height = xymax - ymin;
	depth = zfar - znear;
	q = -(zfar + znear) / depth;
	qn = -2 * (zfar * znear) / depth;
	w = 2 * znear / width;
	w = w / aspect;
	h = 2 * znear / height;

	m[0]  = w;
	m[1]  = 0;
	m[2]  = 0;
	m[3]  = 0;

	m[4]  = 0;
	m[5]  = h;
	m[6]  = 0;
	m[7]  = 0;

	m[8]  = 0;
	m[9]  = 0;
	m[10] = q;
	m[11] = -1;

	m[12] = 0;
	m[13] = 0;
	m[14] = qn;
	m[15] = 0;
}

Vec4f ScreenPos(float* mvp, Vec3f vec, float width, float height, ecbool persp)
{
	Vec4f screenpos;
	screenpos.x = vec.x;
	screenpos.y = vec.y;
	screenpos.z = vec.z;
	screenpos.w = 1;

	Vec4f_transform(&screenpos, mvp);

	if(persp)
	{
		/* does this need to be commented out for correct orthographic pixel-to-pixel match? */
		Vec4f_divf(&screenpos, screenpos, screenpos.w);
	}

	screenpos.x = (screenpos.x * 0.5f + 0.5f) * width;
	screenpos.y = (-screenpos.y * 0.5f + 0.5f) * height;

	return screenpos;
}

Vec3f OnNear(float x, float y, float width, float height,
			 Vec3f posvec, Vec3f sidevec, Vec3f upvec)
{
	float halfw, halfh;
	float ratiox, ratioy;
	float aspect;
	float hnear, wnear;
	Vec3f result;

	halfw = width / 2.0f;
	halfh = height / 2.0f;
	ratiox = (x - halfw) / halfw;
	ratioy = -(y - halfh) / halfh;
	aspect = fabsf(width / height);
	wnear = PROJ_RIGHT * aspect / g_zoom;
	hnear = PROJ_RIGHT / g_zoom;

	Vec3f_mulf(&viewdir, viewdir, mind);
	Vec3f_mulf(&sidevec, sidevec, ratiox * wnear);
	Vec3f_mulf(&upvec, upvec, ratioy * hnear);

	Vec3f_add(&result, viewdir, sidevec);
	Vec3f_add(&result, result, upvec);

	return result;
}

//rad fov
Vec3f OnNearPersp(float x, float y, float width, float height, 
				  Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, 
				  float fov, float mind)
{
	float halfw, halfh;
	float ratiox, ratioy;
	float aspect;
	float hnear, wnear;
	Vec3f result;

	viewdir = Normalize(viewdir);

	halfw = width / 2.0f;
	halfh = height / 2.0f;

	ratiox = (x - halfw) / halfw;
	ratioy = -(y - halfh) / halfh;

	aspect = fabsf(width / height);
	hnear = 2 * tan( fov / 2) * mind;
	wnear = hnear * aspect;

	Vec3f_mulf(&viewdir, viewdir, mind);
	Vec3f_mulf(&sidevec, sidevec, ratiox * wnear);
	Vec3f_mulf(&upvec, upvec, ratioy * hnear);

	Vec3f_add(&result, viewdir, sidevec);
	Vec3f_add(&result, result, upvec);
	/* look much todo dissassembly func's vs vec[]'s vs 2-param vs ...inline... vs VEC3_MACRO() */

	return result;
}

Vec3f ScreenPerspRay(float x, float y, float width, float height, 
					 Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, 
					 float fov)
{
	float halfw, halfh;
	float ratiox, ratioy;
	float aspect;
	float hnear, wnear;

	halfw = width / 2.0f;
	halfh = height / 2.0f;
	ratiox = (x - halfw) / halfw;
	ratioy = -(y - halfh) / halfh;
	aspect = fabsf(width / height);
	hnear = tan( DEGTORAD(fov) / 2.0f );
	wnear = hnear * aspect;

	Vec3f_mulf(&viewdir, viewdir, mind);
	Vec3f_mulf(&sidevec, sidevec, ratiox * wnear);
	Vec3f_mulf(&upvec, upvec, ratioy * hnear);

	Vec3f_add(&result, viewdir, sidevec);
	Vec3f_add(&result, result, upvec);

	return Normalize(result);
}

float Snap(float base, float value)
{
	int count = (int)(value / base);

	return (float)count * base;
}

float SnapNearest(float base, float value)
{
	int count = (int)((value + base/2.0f) / base);

	return (float)count * base;
}
