










#include "frustum.h"
#include "../utils.h"
#include "../platform.h"

Frustum g_frustum;

// This is the index in our selection buffer that has the closet object ID clicked
#define FIRST_OBJECT_ID  3

// We create an enum of the sides so we don't have to call each side 0 or 1.
// This way it makes it more understandable and readable when dealing with frustum sides.
enum FrustumSide
{
	RIGHT	= 0,		// The RIGHT side of the frustum
	LEFT	= 1,		// The LEFT	 side of the frustum
	BOTTOM	= 2,		// The BOTTOM side of the frustum
	TOP		= 3,		// The TOP side of the frustum
	BACK	= 4,		// The BACK	side of the frustum
	FRONT	= 5			// The FRONT side of the frustum
};

// Like above, instead of saying a number for the ABC and D of the plane, we
// want to be more descriptive.
enum PlaneData
{
	A = 0,				// The X value of the plane's normal
	B = 1,				// The Y value of the plane's normal
	C = 2,				// The Z value of the plane's normal
	D = 3				// The distance the plane is from the origin
};

void NormalizePlane(float frustum[6][4], int side)
{
	float magnitude = (float)sqrtf( frustum[side][A] * frustum[side][A] +
									frustum[side][B] * frustum[side][B] +
									frustum[side][C] * frustum[side][C] );

	frustum[side][A] /= magnitude;
	frustum[side][B] /= magnitude;
	frustum[side][C] /= magnitude;
	frustum[side][D] /= magnitude;
}

void Frustum::construct(const Plane3f left, const Plane3f right, const Plane3f top, const Plane3f bottom, const Plane3f front, const Plane3f back)
{
	Frustum[LEFT][A] = left.normal.x;
	Frustum[LEFT][B] = left.normal.y;
	Frustum[LEFT][C] = left.normal.z;
	Frustum[LEFT][D] = left.d;

	Frustum[RIGHT][A] = right.normal.x;
	Frustum[RIGHT][B] = right.normal.y;
	Frustum[RIGHT][C] = right.normal.z;
	Frustum[RIGHT][D] = right.d;

	Frustum[TOP][A] = top.normal.x;
	Frustum[TOP][B] = top.normal.y;
	Frustum[TOP][C] = top.normal.z;
	Frustum[TOP][D] = top.d;

	Frustum[BOTTOM][A] = bottom.normal.x;
	Frustum[BOTTOM][B] = bottom.normal.y;
	Frustum[BOTTOM][C] = bottom.normal.z;
	Frustum[BOTTOM][D] = bottom.d;

	Frustum[FRONT][A] = front.normal.x;
	Frustum[FRONT][B] = front.normal.y;
	Frustum[FRONT][C] = front.normal.z;
	Frustum[FRONT][D] = front.d;

	Frustum[BACK][A] = back.normal.x;
	Frustum[BACK][B] = back.normal.y;
	Frustum[BACK][C] = back.normal.z;
	Frustum[BACK][D] = back.d;
}

void Frustum::construct(const float* proj, const float* modl)
{
	//float   proj[16];
	//float   modl[16];
	float   clip[16];	// clipping planes

	//glGetFloatv( GL_PROJECTION_MATRIX, proj );
	//glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	// Now we actually want to get the sides of the frustum.  To do this we take
	// the clipping planes we received above and extract the sides from them.

	// This will extract the RIGHT side of the frustum
	Frustum[RIGHT][A] = clip[ 3] - clip[ 0];
	Frustum[RIGHT][B] = clip[ 7] - clip[ 4];
	Frustum[RIGHT][C] = clip[11] - clip[ 8];
	Frustum[RIGHT][D] = clip[15] - clip[12];

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.

	// Normalize the RIGHT side
	NormalizePlane(Frustum, RIGHT);

	// This will extract the LEFT side of the frustum
	Frustum[LEFT][A] = clip[ 3] + clip[ 0];
	Frustum[LEFT][B] = clip[ 7] + clip[ 4];
	Frustum[LEFT][C] = clip[11] + clip[ 8];
	Frustum[LEFT][D] = clip[15] + clip[12];

	// Normalize the LEFT side
	NormalizePlane(Frustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	Frustum[BOTTOM][A] = clip[ 3] + clip[ 1];
	Frustum[BOTTOM][B] = clip[ 7] + clip[ 5];
	Frustum[BOTTOM][C] = clip[11] + clip[ 9];
	Frustum[BOTTOM][D] = clip[15] + clip[13];

	// Normalize the BOTTOM side
	NormalizePlane(Frustum, BOTTOM);

	// This will extract the TOP side of the frustum
	Frustum[TOP][A] = clip[ 3] - clip[ 1];
	Frustum[TOP][B] = clip[ 7] - clip[ 5];
	Frustum[TOP][C] = clip[11] - clip[ 9];
	Frustum[TOP][D] = clip[15] - clip[13];

	// Normalize the TOP side
	NormalizePlane(Frustum, TOP);

	// This will extract the BACK side of the frustum
	Frustum[BACK][A] = clip[ 3] - clip[ 2];
	Frustum[BACK][B] = clip[ 7] - clip[ 6];
	Frustum[BACK][C] = clip[11] - clip[10];
	Frustum[BACK][D] = clip[15] - clip[14];

	// Normalize the BACK side
	NormalizePlane(Frustum, BACK);

	// This will extract the FRONT side of the frustum
	Frustum[FRONT][A] = clip[ 3] + clip[ 2];
	Frustum[FRONT][B] = clip[ 7] + clip[ 6];
	Frustum[FRONT][C] = clip[11] + clip[10];
	Frustum[FRONT][D] = clip[15] + clip[14];

	// Normalize the FRONT side
	NormalizePlane(Frustum, FRONT);
}


ecbool Frustum::pointin( float x, float y, float z )
{
	for(int i = 0; i < 6; i++ )
	{
		// Calculate the plane equation and check if the point is behind a side of the frustum
		if(Frustum[i][A] * x + Frustum[i][B] * y + Frustum[i][C] * z + Frustum[i][D] <= 0)
		{
			// The point was behind a side, so it ISN'T in the frustum
			return ecfalse;
		}
	}

	return ectrue;
}

ecbool Frustum::spherein( float x, float y, float z, float radius )
{
	for(int i = 0; i < 6; i++ )
	{
		// If the center of the sphere is farther away from the plane than the radius
		if( Frustum[i][A] * x + Frustum[i][B] * y + Frustum[i][C] * z + Frustum[i][D] <= -radius )
		{
			// The distance was greater than the radius so the sphere is outside of the frustum
			return ecfalse;
		}
	}

	return ectrue;
}


// This determines if a cube is in or around our frustum by it's center and 1/2 it's length
ecbool Frustum::cubein( float x, float y, float z, float size )
{
	// Basically, what is going on is, that we are given the center of the cube,
	// and half the length.  Think of it like a radius.  Then we checking each point
	// in the cube and seeing if it is inside the frustum.  If a point is found in front
	// of a side, then we skip to the next side.  If we get to a plane that does NOT have
	// a point in front of it, then it will return ecfalse.

	// *Note* - This will sometimes say that a cube is inside the frustum when it isn't.
	// This happens when all the corners of the bounding box are not behind any one plane.
	// This is rare and shouldn't effect the overall rendering speed.

	for(int i = 0; i < 6; i++ )
	{
		if(Frustum[i][A] * (x - size) + Frustum[i][B] * (y - size) + Frustum[i][C] * (z - size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x + size) + Frustum[i][B] * (y - size) + Frustum[i][C] * (z - size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x - size) + Frustum[i][B] * (y + size) + Frustum[i][C] * (z - size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x + size) + Frustum[i][B] * (y + size) + Frustum[i][C] * (z - size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x - size) + Frustum[i][B] * (y - size) + Frustum[i][C] * (z + size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x + size) + Frustum[i][B] * (y - size) + Frustum[i][C] * (z + size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x - size) + Frustum[i][B] * (y + size) + Frustum[i][C] * (z + size) + Frustum[i][D] > 0)
			continue;
		if(Frustum[i][A] * (x + size) + Frustum[i][B] * (y + size) + Frustum[i][C] * (z + size) + Frustum[i][D] > 0)
			continue;

		// If we get here, it isn't in the frustum
		return ecfalse;
	}

	return ectrue;
}


// This determines if a BOX is in or around our frustum by it's std::min and std::max points
ecbool Frustum::boxin( float x, float y, float z, float x2, float y2, float z2)
{
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 6; i++ )
	{
		if(Frustum[i][A] * x  + Frustum[i][B] * y  + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y  + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y2 + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y2 + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y  + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y  + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y2 + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y2 + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return ecfalse;
	}

	// Return a ectrue for the box being inside of the frustum
	return ectrue;
}

// Doesn't check front and back plane
ecbool Frustum::boxin2( float x, float y, float z, float x2, float y2, float z2)
{
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 4; i++ )
	{
		if(Frustum[i][A] * x  + Frustum[i][B] * y  + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y  + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y2 + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y2 + Frustum[i][C] * z  + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y  + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y  + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x  + Frustum[i][B] * y2 + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;
		if(Frustum[i][A] * x2 + Frustum[i][B] * y2 + Frustum[i][C] * z2 + Frustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return ecfalse;
	}

	// Return a ectrue for the box being inside of the frustum
	return ectrue;
}

