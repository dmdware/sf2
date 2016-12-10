


#include "q3bsp.h"
#include "../math/frustum.h"
#include "../texture.h"
#include "../render/shader.h"
#include "../gui/gui.h"
//#include "../render/ms3d.h"
#include "../debug.h"
#include "../sim/entity.h"

Q3BSP g_map;

Q3BSP::Q3BSP()
{
	m_numOfVerts		= 0;	
	m_numOfFaces		= 0;	
	m_numOfIndices		= 0;	
	m_numOfTextures		= 0;
	m_numOfLightmaps	= 0;
	m_numOfNodes		= 0;
	m_numOfLeafs		= 0;
	m_numOfLeafFaces	= 0;
	m_numOfPlanes		= 0;
	m_numOfBrushes		= 0;
	m_numOfBrushSides	= 0;
	m_numOfLeafBrushes	= 0;

	//m_traceRatio		= 0;
	m_traceType			= 0;
	m_traceRadius		= 0;

	bool m_bCollided	= false;

	bool m_bGrounded	= false;
	bool m_bTryStep		= false;

	m_vTraceMins = Vec3f(0, 0, 0);
	m_vTraceMaxs = Vec3f(0, 0, 0);
	m_vExtents   = Vec3f(0, 0, 0);
	
	m_vCollisionNormal = Vec3f(0, 0, 0);
	
	m_pVerts		 = NULL;	
	m_pFaces		 = NULL;	
	m_pIndices		 = NULL;	
	m_pNodes		 = NULL;
	m_pLeafs		 = NULL;
	m_pPlanes		 = NULL;
	m_pLeafFaces	 = NULL;

	memset(&m_clusters, 0, sizeof(tBSPVisData));
	
	m_pBrushes       = NULL;
	m_pBrushSides	 = NULL;
	m_pTextures      = NULL;
	m_pLightmaps	 = NULL;
	m_pLeafBrushes	 = NULL;
}

void Q3BSP::ChangeGamma(byte *pImage, int size, float factor)
{
	for(int i = 0; i < size / 3; i++, pImage += 3) 
	{
		float scale = 1.0f, temp = 0.0f;
		float r = 0, g = 0, b = 0;

		r = (float)pImage[0];
		g = (float)pImage[1];
		b = (float)pImage[2];

		r = r * factor / 255.0f;
		g = g * factor / 255.0f;
		b = b * factor / 255.0f;
		
		if(r > 1.0f && (temp = (1.0f/r)) < scale) scale=temp;
		if(g > 1.0f && (temp = (1.0f/g)) < scale) scale=temp;
		if(b > 1.0f && (temp = (1.0f/b)) < scale) scale=temp;

		scale*=255.0f;		
		r*=scale;	g*=scale;	b*=scale;

		pImage[0] = (byte)r;
		pImage[1] = (byte)g;
		pImage[2] = (byte)b;
	}
}

void Q3BSP::CreateLightmapTexture(UINT &texture, byte *pImageBits, int width, int height)
{
	glGenTextures(1, &texture);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, texture);

	ChangeGamma(pImageBits, width*height*3, 10);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, pImageBits);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Q3BSP::FindTextureExtension(char *strFileName)
{
	char strJPGPath[DMD_MAX_PATH] = {0};
	char strTGAPath[DMD_MAX_PATH]    = {0}; 
	FILE *fp = NULL;

	GetCurrentDirectory(DMD_MAX_PATH, strJPGPath);

	strcat(strJPGPath, "\\");
	strcat(strJPGPath, strFileName);
	strcpy(strTGAPath, strJPGPath);
	
	strcat(strJPGPath, ".jpg");
	strcat(strTGAPath, ".tga");

	if((fp = fopen(strJPGPath, "rb")) != NULL)
	{
		strcat(strFileName, ".jpg");
		return;
	}

	if((fp = fopen(strTGAPath, "rb")) != NULL)
	{
		strcat(strFileName, ".tga");
		return;
	}
}

void Q3BSP::ReloadTextures()
{
	int i;

	for(i = 0; i < m_numOfTextures; i++)
	{
		CreateTex(m_textures[i], m_pTextures[i].strName, false, true);
	}

	for(i = 0; i < m_numOfLightmaps ; i++)
	{
		CreateLightmapTexture(m_lightmaps[i], (unsigned char *)m_pLightmaps[i].imageBits, 128, 128);
	}
}

bool Q3BSP::LoadBSP(const char *name)
{
	FILE *fp = NULL;
	int i = 0;

	//char backdrop[128];
	//sprintf(backdrop, "models\\%s\\backdrop.ms3d", name);
	//g_backdrop = LoadModel(backdrop, Vec3f(1,1,1), Vec3f(0,0,0));
	
	char bspPath[128];
	char fullpath[DMD_MAX_PATH+1];
	sprintf(bspPath, "maps/%s.bsp", name);
	FullPath(bspPath, fullpath);

	if((fp = fopen(fullpath, "rb")) == NULL)
	{
		ErrMess("Error", "Could not find BSP file!");
		return false;
	}

	tBSPHeader header = {0};
	tBSPLump lumps[kMaxLumps] = {0};

	fread(&header, 1, sizeof(tBSPHeader), fp);
	fread(&lumps, kMaxLumps, sizeof(tBSPLump), fp);

	m_numOfVerts = lumps[kVertices].length / sizeof(tBSPVertex);
	m_pVerts     = new tBSPVertex [m_numOfVerts];

	m_numOfFaces = lumps[kFaces].length / sizeof(tBSPFace);
	m_pFaces     = new tBSPFace [m_numOfFaces];

	m_numOfIndices = lumps[kIndices].length / sizeof(int);
	m_pIndices     = new int [m_numOfIndices];

	m_numOfTextures = lumps[kTextures].length / sizeof(tBSPTexture);
	m_pTextures = new tBSPTexture [m_numOfTextures];
 
	m_numOfLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
	m_pLightmaps = new tBSPLightmap [m_numOfLightmaps];

	fseek(fp, lumps[kVertices].offset, SEEK_SET);

	for(i = 0; i < m_numOfVerts; i++)
	{
		fread(&m_pVerts[i], 1, sizeof(tBSPVertex), fp);
		
		float temp = m_pVerts[i].vPosition.y;
		m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
		m_pVerts[i].vPosition.z = -temp;
	}	

	fseek(fp, lumps[kIndices].offset, SEEK_SET);

	fread(m_pIndices, m_numOfIndices, sizeof(int), fp);

	fseek(fp, lumps[kFaces].offset, SEEK_SET);

	fread(m_pFaces, m_numOfFaces, sizeof(tBSPFace), fp);

	fseek(fp, lumps[kTextures].offset, SEEK_SET);
	
	fread(m_pTextures, m_numOfTextures, sizeof(tBSPTexture), fp);

	for(i = 0; i < m_numOfTextures; i++)
	{
		if(strstr(m_pTextures[i].strName, "~"))
			m_passable[i] = true;
		if(strstr(m_pTextures[i].strName, "^"))
			m_sky[i] = true;

		StripPath(m_pTextures[i].strName);
		char filename[64];
		strcpy(filename, m_pTextures[i].strName);
		//sprintf(m_pTextures[i].strName, "textures%s\\%s", name, filename);
		sprintf(m_pTextures[i].strName, "textures/%s", filename);

		FindTextureExtension(m_pTextures[i].strName);
	}

	fseek(fp, lumps[kLightmaps].offset, SEEK_SET);

	for(i = 0; i < m_numOfLightmaps ; i++)
	{
		fread(&m_pLightmaps[i], 1, sizeof(tBSPLightmap), fp);
	}

	m_numOfNodes = lumps[kNodes].length / sizeof(tBSPNode);
	m_pNodes     = new tBSPNode [m_numOfNodes];

	fseek(fp, lumps[kNodes].offset, SEEK_SET);
	fread(m_pNodes, m_numOfNodes, sizeof(tBSPNode), fp);

	m_numOfLeafs = lumps[kLeafs].length / sizeof(tBSPLeaf);
	m_pLeafs     = new tBSPLeaf [m_numOfLeafs];

	fseek(fp, lumps[kLeafs].offset, SEEK_SET);
	fread(m_pLeafs, m_numOfLeafs, sizeof(tBSPLeaf), fp);

	for(i = 0; i < m_numOfLeafs; i++)
	{
		int temp = m_pLeafs[i].min.y;
		m_pLeafs[i].min.y = m_pLeafs[i].min.z;
		m_pLeafs[i].min.z = -temp;

		temp = m_pLeafs[i].max.y;
		m_pLeafs[i].max.y = m_pLeafs[i].max.z;
		m_pLeafs[i].max.z = -temp;
	}

	m_numOfLeafFaces = lumps[kLeafFaces].length / sizeof(int);
	m_pLeafFaces     = new int [m_numOfLeafFaces];

	fseek(fp, lumps[kLeafFaces].offset, SEEK_SET);
	fread(m_pLeafFaces, m_numOfLeafFaces, sizeof(int), fp);

	m_numOfPlanes = lumps[kPlanes].length / sizeof(tBSPPlane);
	m_pPlanes     = new tBSPPlane [m_numOfPlanes];

	fseek(fp, lumps[kPlanes].offset, SEEK_SET);
	fread(m_pPlanes, m_numOfPlanes, sizeof(tBSPPlane), fp);

	for(i = 0; i < m_numOfPlanes; i++)
	{
		float temp = m_pPlanes[i].vNormal.y;
		m_pPlanes[i].vNormal.y = m_pPlanes[i].vNormal.z;
		m_pPlanes[i].vNormal.z = -temp;
	}

	fseek(fp, lumps[kVisData].offset, SEEK_SET);

	if(lumps[kVisData].length) 
	{
		fread(&(m_clusters.numOfClusters),	 1, sizeof(int), fp);
		fread(&(m_clusters.bytesPerCluster), 1, sizeof(int), fp);

		int size = m_clusters.numOfClusters * m_clusters.bytesPerCluster;
		m_clusters.pBitsets = new byte [size];

		fread(m_clusters.pBitsets, 1, sizeof(byte) * size, fp);
	}
	else
		m_clusters.pBitsets = NULL;

	m_numOfBrushes = lumps[kBrushes].length / sizeof(int);
	m_pBrushes     = new tBSPBrush [m_numOfBrushes];
	
	fseek(fp, lumps[kBrushes].offset, SEEK_SET);
	fread(m_pBrushes, m_numOfBrushes, sizeof(tBSPBrush), fp);

	m_numOfBrushSides = lumps[kBrushSides].length / sizeof(int);
	m_pBrushSides     = new tBSPBrushSide [m_numOfBrushSides];

	fseek(fp, lumps[kBrushSides].offset, SEEK_SET);
	fread(m_pBrushSides, m_numOfBrushSides, sizeof(tBSPBrushSide), fp);

	m_numOfLeafBrushes = lumps[kLeafBrushes].length / sizeof(int);
	m_pLeafBrushes     = new int [m_numOfLeafBrushes];

	fseek(fp, lumps[kLeafBrushes].offset, SEEK_SET);
	fread(m_pLeafBrushes, m_numOfLeafBrushes, sizeof(int), fp);

	//ClearScenery();
	fseek(fp, lumps[kEntities].offset, SEEK_SET);
	char* entities = new char[ lumps[kEntities].length ];
	fread(entities, lumps[kEntities].length, sizeof(char), fp);
	ReadEntities(entities);
	delete entities;

	fclose(fp);

	m_FacesDrawn.Resize(m_numOfFaces);

	ReloadTextures();

	return true;
}

int Q3BSP::FindLeaf(const Vec3f &vPos)
{
	int i = 0;
	float distance = 0.0f;
	
	// Continue looping until we find a negative index
	while(i >= 0)
	{
		// Get the current node, then find the slitter plane from that
		// node's plane index.  Notice that we use a constant reference
		// to store the plane and node so we get some optimization.
		const tBSPNode&  node = m_pNodes[i];
		const tBSPPlane& plane = m_pPlanes[node.plane];

		// Use the Plane3f Equation (Ax + by + Cz + D = 0) to find if the
		// camera is in front of or behind the current splitter plane.
        distance =	plane.vNormal.x * vPos.x + 
					plane.vNormal.y * vPos.y + 
					plane.vNormal.z * vPos.z - plane.d;

		// If the camera is in front of the plane
        if(distance >= 0)	
		{
			// Assign the current node to the node in front of itself
            i = node.front;
        }
		// Else if the camera is behind the plane
        else		
		{
			// Assign the current node to the node behind itself
            i = node.back;
        }
    }

	// Return the leaf index (same thing as saying:  return -(i + 1)).
    return ~i;  // Binary operation
}

inline int Q3BSP::IsClusterVisible(int current, int test)
{
	// Make sure we have valid memory and that the current cluster is > 0.
	// If we don't have any memory or a negative cluster, return a visibility (1).
	if(!m_clusters.pBitsets || current < 0) return 1;

	// Use binary math to get the 8 bit visibility set for the current cluster
	byte visSet = m_clusters.pBitsets[(current*m_clusters.bytesPerCluster) + (test / 8)];
	
	// Now that we have our vector (bitset), do some bit shifting to find if
	// the "test" cluster is visible from the "current" cluster, according to the bitset.
	int result = visSet & (1 << ((test) & 7));

	// Return the result ( either 1 (visible) or 0 (not visible) )
	return ( result );
}

Vec3f Q3BSP::TryToStep(Vec3f vStart, Vec3f vEnd, float maxStep)
{
	// In this function we loop until we either found a reasonable height
	// that we can step over, or find out that we can't step over anything.
	// We check 10 times, each time increasing the step size to check for
	// a collision.  If we don't collide, then we climb over the step.

	// Go through and check different heights to step up
	for(float height = 1.0f; height <= maxStep; height++)
	{
		// Reset our variables for each loop interation
		m_bCollided = false;
		m_bTryStep = false;

		// Here we add the current height to our y position of a new start and end.
		// If these 2 new start and end positions are okay, we can step up.
		Vec3f vStepStart = Vec3f(vStart.x, vStart.y + height, vStart.z);
		Vec3f vStepEnd   = Vec3f(vEnd.x, vStart.y + height, vEnd.z);
				
		// Test to see if the new position we are trying to step collides or not
		Vec3f vStepPosition = Trace(vStepStart, vStepEnd);

		// If we didn't collide, we can step!
		if(!m_bCollided)
		{
			// Here we get the current view, then increase the y value by the current height.
			// This makes it so when we are walking up the stairs, our view follows our step
			// height and doesn't sag down as we walk up the stairs.
			//Vec3f vNewView = g_cam->View();					
			//g_cam->SetView(Vec3f(vNewView.x, vNewView.y + height, vNewView.z));

			// Return the current position since we stepped up somewhere
			return vStepPosition;		
		}
	}

	// If we can't step, then we just return the original position of the collision
	return vStart;
}

Vec3f Q3BSP::Trace(Vec3f vStart, Vec3f vEnd)
{
	// Initially we set our trace ratio to 1.0f, which means that we don't have
	// a collision or intersection point, so we can move freely.
	m_traceRatio = 1.0f;
	
	// We start out with the first node (0), setting our start and end ratio to 0 and 1.
	// We will recursively go through all of the nodes to see which brushes we should check.
	CheckNode(0, vStart, vEnd);

	// If the traceRatio is STILL 1.0f, then we never collided and just return our end position
	if(m_traceRatio == 1.0f)
	{
		return vEnd;
	}
	else	// Else COLLISION!!!!
	{
		// Set our new position to a position that is right up to the brush we collided with
		Vec3f vNewPosition = vStart + ((vEnd - vStart) * m_traceRatio);
		
		//if(m_traceRatio > 0.0f)
		{
			//if(m_traceRatio < 0.0f)
			//	vNewPosition = Trace(vStart, vNewPosition);
			//if(m_traceRatio < 0.0f)
			//	m_traceRatio -= 0.01f;

			// Get the distance from the end point to the new position we just got
			Vec3f vMove = vEnd - vNewPosition;

			// Get the distance we need to travel backwards to the new slide position.
			// This is the distance of course along the normal of the plane we collided with.
			float distance = Dot(vMove, m_vCollisionNormal);

			// Get the new end position that we will end up (the slide position).
			Vec3f vEndPosition = vEnd - m_vCollisionNormal*distance;
		
			// Since we got a new position for our sliding vector, we need to check
			// to make sure that new sliding position doesn't collide with anything else.

			//if(m_traceRatio > 0.0f)
			if(vEndPosition != vEnd)
				vNewPosition = Trace(vNewPosition, vEndPosition);
			//vNewPosition = Trace(vStart, vEndPosition);
			else
				vNewPosition = vEndPosition;
		}

		if(m_vCollisionNormal.y > 0.2f || m_bGrounded)
			m_bGrounded = true;
		else
			m_bGrounded = false;

		// Return the new position to be used by our camera (or player)
		return vNewPosition;
	}
}

Vec3f Q3BSP::traceray(Vec3f vStart, Vec3f vEnd)
{
	// We don't use this function, but we set it up to allow us to just check a
	// ray with the BSP tree brushes.  We do so by setting the trace type to TYPE_RAY.
	m_traceType = TYPE_RAY;

	// Run the normal Trace() function with our start and end 
	// position and return a new position
	return Trace(vStart, vEnd);
}

Vec3f Q3BSP::TraceSphere(Vec3f vStart, Vec3f vEnd, float radius, float maxStep)
{
	m_traceType = TYPE_SPHERE;
	m_bCollided = false;

	m_bTryStep = false;
	m_bGrounded = false;

	m_traceRadius = radius;

	Vec3f vNewPosition = Trace(vStart, vEnd);

	if(m_bCollided && m_bTryStep)
		vNewPosition = TryToStep(vNewPosition, vEnd, maxStep);
	
	return vNewPosition;
}

Vec3f Q3BSP::TraceBox(Vec3f vStart, Vec3f vEnd, Vec3f vMin, Vec3f vMax, float maxStep)
{
	m_traceType = TYPE_BOX;
	m_vTraceMaxs = vMax;
	m_vTraceMins = vMin;
	m_bCollided = false;

	m_bTryStep = false;
	m_bGrounded = false;
	m_bCeiling = false;

	// Grab the extend of our box (the largest size for each x, y, z axis)
	m_vExtents = Vec3f(-m_vTraceMins.x > m_vTraceMaxs.x ? -m_vTraceMins.x : m_vTraceMaxs.x,
						  -m_vTraceMins.y > m_vTraceMaxs.y ? -m_vTraceMins.y : m_vTraceMaxs.y,
						  -m_vTraceMins.z > m_vTraceMaxs.z ? -m_vTraceMins.z : m_vTraceMaxs.z);

	m_stuck = false;

	Vec3f vNewPosition = Trace(vStart, vEnd);

	if(m_bCollided && m_bTryStep)
		vNewPosition = TryToStep(vNewPosition, vEnd, maxStep);

	//vNewPosition = Trace(vNewPosition, vNewPosition);

	//if(m_stuck)
		//vNewPosition = UnstuckNode(0, vStart - (vEnd-vStart), vNewPosition);
		//vNewPosition = vStart;

	return vNewPosition;
}

Vec3f Q3BSP::UnstuckNode(int nodeIndex, Vec3f vStart, Vec3f vEnd)
{
	Vec3f vNewPosition = vStart;

	// Check if the next node is a leaf
	if(nodeIndex < 0)
	{
		// If this node in the BSP is a leaf, we need to negate and add 1 to offset
		// the real node index into the m_pLeafs[] array.  You could also do [~nodeIndex].
		tBSPLeaf *pLeaf = &m_pLeafs[-(nodeIndex + 1)];

		// We have a leaf, so let's go through all of the brushes for that leaf
		for(int i = 0; i < pLeaf->numOfLeafBrushes; i++)
		{
			// Get the current brush that we going to check
			tBSPBrush *pBrush = &m_pBrushes[m_pLeafBrushes[pLeaf->leafBrush + i]];

			if(m_passable[pBrush->textureID])
				continue;

			// Check if we have brush sides and the current brush is solid and collidable
			if((pBrush->numOfBrushSides > 0) && (m_pTextures[pBrush->textureID].textureType & 1))
			{
				// Now we delve into the dark depths of the real calculations for collision.
				// We can now check the movement vector against our brush planes.
				vNewPosition = UnstuckBrush(pBrush, vNewPosition);
			}
		}

		// Since we found the brushes, we can go back up and stop recursing at this level
		return vNewPosition;
	}

	// Grad the next node to work with and grab this node's plane data
	tBSPNode *pNode = &m_pNodes[nodeIndex];
	tBSPPlane *pPlane = &m_pPlanes[pNode->plane];
	
	// Here we use the plane equation to find out where our initial start position is
	// according the the node that we are checking.  We then grab the same info for the end pos.
	float startDistance = Dot(vStart, pPlane->vNormal) - pPlane->d;
	float endDistance = Dot(vEnd, pPlane->vNormal) - pPlane->d;
	float offset = 0.0f;

	// If we are doing sphere collision, include an offset for our collision tests below
	if(m_traceType == TYPE_SPHERE)
		offset = m_traceRadius;

	// Here we check to see if we are working with a BOX or not
	else if(m_traceType == TYPE_BOX)
	{	
		// Get the distance our AABB is from the current splitter plane
		offset = (float)(fabs( m_vExtents.x * pPlane->vNormal.x ) +
                         fabs( m_vExtents.y * pPlane->vNormal.y ) +
                         fabs( m_vExtents.z * pPlane->vNormal.z ) );
	}

	// Here we check to see if the start and end point are both in front of the current node.
	// If so, we want to check all of the nodes in front of this current splitter plane.
	//if(startDistance >= offset)
	if(startDistance >= offset && endDistance >= offset)
	{
		// Traverse the BSP tree on all the nodes in front of this current splitter plane
		vNewPosition = UnstuckNode(pNode->front, vStart, vNewPosition);
	}
	// If both points are behind the current splitter plane, traverse down the back nodes
	//else if(startDistance < -offset)
	else if(startDistance < -offset && endDistance < -offset)
	{
		// Traverse the BSP tree on all the nodes in back of this current splitter plane
		vNewPosition = UnstuckNode(pNode->back, vStart, vNewPosition);
	}	
	//else
	//{
	//	vNewPosition = UnstuckNode(pNode->front, vStart, vNewPosition);
	//	vNewPosition = UnstuckNode(pNode->back, vStart, vNewPosition);
	//}
	else
	{
			// If we get here, then our ray needs to be split in half to check the nodes
		// on both sides of the current splitter plane.  Thus we create 2 ratios.
		float Ratio1 = 1.0f, Ratio2 = 0.0f, middleRatio = 0.0f;
		Vec3f vMiddle;	// This stores the middle point for our split ray

		// Start of the side as the front side to check
		int side = pNode->front;

		// Here we check to see if the start point is in back of the plane (negative)
		if(startDistance < endDistance)
		{
			// Since the start position is in back, let's check the back nodes
			side = pNode->back;

			// Here we create 2 ratios that hold a distance from the start to the
			// extent closest to the start (take into account a sphere and epsilon).
			float inverseDistance = 1.0f / (startDistance - endDistance);
			Ratio1 = (startDistance - offset - EPSILON) * inverseDistance;
			Ratio2 = (startDistance + offset + EPSILON) * inverseDistance;
		}
		// Check if the starting point is greater than the end point (positive)
		else if(startDistance > endDistance)
		{
			// This means that we are going to recurse down the front nodes first.
			// We do the same thing as above and get 2 ratios for split ray.
			float inverseDistance = 1.0f / (startDistance - endDistance);
			Ratio1 = (startDistance + offset + EPSILON) * inverseDistance;
			Ratio2 = (startDistance - offset - EPSILON) * inverseDistance;
		}

		// Make sure that we have valid numbers and not some weird float problems.
		// This ensures that we have a value from 0 to 1 as a good ratio should be :)
		if (Ratio1 < 0.0f) Ratio1 = 0.0f;
        else if (Ratio1 > 1.0f) Ratio1 = 1.0f;

        if (Ratio2 < 0.0f) Ratio2 = 0.0f;
        else if (Ratio2 > 1.0f) Ratio2 = 1.0f;

		// Just like we do in the Trace() function, we find the desired middle
		// point on the ray, but instead of a point we get a middleRatio percentage.
		//middleRatio = startRatio + ((endRatio - startRatio) * Ratio1);
		vMiddle = vStart + ((vEnd - vStart) * Ratio1);

		// Now we recurse on the current side with only the first half of the ray
		CheckNode(side, vStart, vMiddle);

		// Now we need to make a middle point and ratio for the other side of the node
		//middleRatio = startRatio + ((endRatio - startRatio) * Ratio2);
		vMiddle = vStart + ((vEnd - vStart) * Ratio2);

		// Depending on which side should go last, traverse the bsp with the
		// other side of the split ray (movement vector).
		if(side == pNode->back)
			UnstuckNode(pNode->front, vMiddle, vEnd);
		else
			UnstuckNode(pNode->back, vMiddle, vEnd);
	}

	if(nodeIndex == 0 && vNewPosition != vStart)
		return UnstuckNode(0, vStart, vNewPosition);

	return vNewPosition;
}

// This traverses the BSP to find the brushes closest to our position
void Q3BSP::CheckNode(int nodeIndex, Vec3f vStart, Vec3f vEnd)
{
	// Check if the next node is a leaf
	if(nodeIndex < 0)
	{
		// If this node in the BSP is a leaf, we need to negate and add 1 to offset
		// the real node index into the m_pLeafs[] array.  You could also do [~nodeIndex].
		tBSPLeaf *pLeaf = &m_pLeafs[-(nodeIndex + 1)];

		// We have a leaf, so let's go through all of the brushes for that leaf
		for(int i = 0; i < pLeaf->numOfLeafBrushes; i++)
		{
			// Get the current brush that we going to check
			tBSPBrush *pBrush = &m_pBrushes[m_pLeafBrushes[pLeaf->leafBrush + i]];

			if(m_passable[pBrush->textureID])
				continue;

			// Check if we have brush sides and the current brush is solid and collidable
			if((pBrush->numOfBrushSides > 0) && (m_pTextures[pBrush->textureID].textureType & 1))
			{
				// Now we delve into the dark depths of the real calculations for collision.
				// We can now check the movement vector against our brush planes.
				CheckBrush(pBrush, vStart, vEnd);
			}
		}

		// Since we found the brushes, we can go back up and stop recursing at this level
		return;
	}

	// Grad the next node to work with and grab this node's plane data
	tBSPNode *pNode = &m_pNodes[nodeIndex];
	tBSPPlane *pPlane = &m_pPlanes[pNode->plane];
	
	// Here we use the plane equation to find out where our initial start position is
	// according the the node that we are checking.  We then grab the same info for the end pos.
	float startDistance = Dot(vStart, pPlane->vNormal) - pPlane->d;
	float endDistance = Dot(vEnd, pPlane->vNormal) - pPlane->d;
	float offset = 0.0f;

	// If we are doing sphere collision, include an offset for our collision tests below
	if(m_traceType == TYPE_SPHERE)
		offset = m_traceRadius;

	// Here we check to see if we are working with a BOX or not
	else if(m_traceType == TYPE_BOX)
	{	
		// Get the distance our AABB is from the current splitter plane
		offset = (float)(fabs( m_vExtents.x * pPlane->vNormal.x ) +
                         fabs( m_vExtents.y * pPlane->vNormal.y ) +
                         fabs( m_vExtents.z * pPlane->vNormal.z ) );
	}

	// Here we check to see if the start and end point are both in front of the current node.
	// If so, we want to check all of the nodes in front of this current splitter plane.
	if(startDistance >= offset && endDistance >= offset)
	{
		// Traverse the BSP tree on all the nodes in front of this current splitter plane
		CheckNode(pNode->front, vStart, vEnd);
	}
	// If both points are behind the current splitter plane, traverse down the back nodes
	else if(startDistance < -offset && endDistance < -offset)
	{
		// Traverse the BSP tree on all the nodes in back of this current splitter plane
		CheckNode(pNode->back, vStart, vEnd);
	}	
	else
	{
		// If we get here, then our ray needs to be split in half to check the nodes
		// on both sides of the current splitter plane.  Thus we create 2 ratios.
		float Ratio1 = 1.0f, Ratio2 = 0.0f, middleRatio = 0.0f;
		Vec3f vMiddle;	// This stores the middle point for our split ray

		// Start of the side as the front side to check
		int side = pNode->front;

		// Here we check to see if the start point is in back of the plane (negative)
		if(startDistance < endDistance)
		{
			// Since the start position is in back, let's check the back nodes
			side = pNode->back;

			// Here we create 2 ratios that hold a distance from the start to the
			// extent closest to the start (take into account a sphere and epsilon).
			float inverseDistance = 1.0f / (startDistance - endDistance);
			Ratio1 = (startDistance - offset - EPSILON) * inverseDistance;
			Ratio2 = (startDistance + offset + EPSILON) * inverseDistance;
		}
		// Check if the starting point is greater than the end point (positive)
		else if(startDistance > endDistance)
		{
			// This means that we are going to recurse down the front nodes first.
			// We do the same thing as above and get 2 ratios for split ray.
			float inverseDistance = 1.0f / (startDistance - endDistance);
			Ratio1 = (startDistance + offset + EPSILON) * inverseDistance;
			Ratio2 = (startDistance - offset - EPSILON) * inverseDistance;
		}

		// Make sure that we have valid numbers and not some weird float problems.
		// This ensures that we have a value from 0 to 1 as a good ratio should be :)
		if (Ratio1 < 0.0f) Ratio1 = 0.0f;
        else if (Ratio1 > 1.0f) Ratio1 = 1.0f;

        if (Ratio2 < 0.0f) Ratio2 = 0.0f;
        else if (Ratio2 > 1.0f) Ratio2 = 1.0f;

		// Just like we do in the Trace() function, we find the desired middle
		// point on the ray, but instead of a point we get a middleRatio percentage.
		//middleRatio = startRatio + ((endRatio - startRatio) * Ratio1);
		vMiddle = vStart + ((vEnd - vStart) * Ratio1);

		// Now we recurse on the current side with only the first half of the ray
		CheckNode(side, vStart, vMiddle);

		// Now we need to make a middle point and ratio for the other side of the node
		//middleRatio = startRatio + ((endRatio - startRatio) * Ratio2);
		vMiddle = vStart + ((vEnd - vStart) * Ratio2);

		// Depending on which side should go last, traverse the bsp with the
		// other side of the split ray (movement vector).
		if(side == pNode->back)
			CheckNode(pNode->front, vMiddle, vEnd);
		else
			CheckNode(pNode->back, vMiddle, vEnd);
	}
}

Vec3f Q3BSP::UnstuckBrush(tBSPBrush *pBrush, Vec3f vStart)
{
	float greatestNegD = -9999999999999.0f;
	float tempNegD;
	float offsetD;
	Vec3f vNormal(0, 0, 0);

	for(int i = 0; i < pBrush->numOfBrushSides; i++)
	{
		// Here we grab the current brush side and plane in this brush
		tBSPBrushSide *pBrushSide = &m_pBrushSides[pBrush->brushSide + i];
		tBSPPlane *pPlane = &m_pPlanes[pBrushSide->plane];

		// Let's store a variable for the offset (like for sphere collision)
		float offset = 0.0f;

		// If we are testing sphere collision we need to add the sphere radius
		if(m_traceType == TYPE_SPHERE)
			offset = m_traceRadius;

		// Test the start and end points against the current plane of the brush side.
		// Notice that we add an offset to the distance from the origin, which makes
		// our sphere collision work.
		//float startDistance = Dot(vStart, pPlane->vNormal) - (pPlane->d + offset);
		//float endDistance = Dot(vEnd, pPlane->vNormal) - (pPlane->d + offset);

		// Store the offset that we will check against the plane
		Vec3f vInOffset = Vec3f(0, 0, 0);
		Vec3f vOutOffset = Vec3f(0, 0, 0);

		// If we are using AABB collision
		if(m_traceType == TYPE_BOX)
		{   
			// Grab the closest corner (x, y, or z value) that is closest to the plane
			vInOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMaxs.x : m_vTraceMins.x;
			vInOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMaxs.y : m_vTraceMins.y;
			vInOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMaxs.z : m_vTraceMins.z;
			
			// Grab the corner (x, y, or z value) that is FARTHEST from the plane
            vOutOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMins.x : m_vTraceMaxs.x;
			vOutOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMins.y : m_vTraceMaxs.y;
			vOutOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMins.z : m_vTraceMaxs.z;

			// Use the plane equation to grab the distance our start position is from the plane.
            float startInDistance = Dot(vStart + vInOffset, pPlane->vNormal) - pPlane->d;
			tempNegD = Dot(vStart + vOutOffset, pPlane->vNormal) - pPlane->d;

			// Get the distance our end position is from this current brush plane
            //endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;

			if(startInDistance < 0.0f && tempNegD > greatestNegD)
			{
				greatestNegD = tempNegD;
				offsetD = startInDistance;
				vNormal = pPlane->vNormal;
			}
        }
	}
	
	if(vNormal != Vec3f(0, 0, 0))
		vStart = vStart - vNormal * offsetD;

	return vStart;
}

#define START_RATIO		-100.0f

// This checks our movement vector against all the planes of the brush
void Q3BSP::CheckBrush(tBSPBrush *pBrush, Vec3f vStart, Vec3f vEnd)
{
	//float startRatio = -1.0f;		// Like in BrushCollision.htm, start a ratio at -1
	float startRatio = START_RATIO;		// Like in BrushCollision.htm, start a ratio at -1
    float endRatio = 1.0f;			// Set the end ratio to 1
    bool startsOut = false;			// This tells us if we starting outside the brush
	bool getsOut = false;

	// Go through all of the brush sides and check collision against each plane
	for(int i = 0; i < pBrush->numOfBrushSides; i++)
	{
		// Here we grab the current brush side and plane in this brush
		tBSPBrushSide *pBrushSide = &m_pBrushSides[pBrush->brushSide + i];
		tBSPPlane *pPlane = &m_pPlanes[pBrushSide->plane];

		// Let's store a variable for the offset (like for sphere collision)
		float offset = 0.0f;

		// If we are testing sphere collision we need to add the sphere radius
		if(m_traceType == TYPE_SPHERE)
			offset = m_traceRadius;

		// Test the start and end points against the current plane of the brush side.
		// Notice that we add an offset to the distance from the origin, which makes
		// our sphere collision work.
		float startDistance = Dot(vStart, pPlane->vNormal) - (pPlane->d + offset);
		float endDistance = Dot(vEnd, pPlane->vNormal) - (pPlane->d + offset);

		// Store the offset that we will check against the plane
		Vec3f vOffset = Vec3f(0, 0, 0);

		// If we are using AABB collision
		if(m_traceType == TYPE_BOX)
		{
			// Grab the closest corner (x, y, or z value) that is closest to the plane
            vOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMaxs.x : m_vTraceMins.x;
			vOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMaxs.y : m_vTraceMins.y;
			vOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMaxs.z : m_vTraceMins.z;
            
			// Use the plane equation to grab the distance our start position is from the plane.
            startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;

			// Get the distance our end position is from this current brush plane
            endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;
        }

		// Make sure we start outside of the brush's volume
		if(startDistance > 0)	startsOut = true;
		
		if(endDistance > 0) getsOut = true;

		// Stop checking since both the start and end position are in front of the plane
		if(startDistance > 0 && endDistance > 0)
			return;

		// Continue on to the next brush side if both points are behind or on the plane
		if(startDistance <= 0 && endDistance <= 0)
			continue;

		// If the distance of the start point is greater than the end point, we have a collision!
		if(startDistance > endDistance)
		//if(startDistance > 0)
		{
			// This gets a ratio from our starting point to the approximate collision spot
			float Ratio1 = (startDistance - EPSILON) / (startDistance - endDistance);
			//float Ratio1 = (startDistance) / (startDistance - endDistance);

			// If this is the first time coming here, then this will always be true,
			if(Ratio1 > startRatio)
			{
				// Set the startRatio (currently the closest collision distance from start)
				startRatio = Ratio1;
				m_bCollided = true;		// Let us know we collided!

				// Store the normal of plane that we collided with for sliding calculations
				m_vCollisionNormal = pPlane->vNormal;

				/*
				for(int i=0; i<m_collisionPlanes.size(); i++)
					if(pBrushSide->plane == m_collisionPlanes[i])
					{
						m_traceRatio = 0;
						return;
					}

				m_collisionPlanes.push_back(pBrushSide->plane);
				*/

				// This checks first tests if we actually moved along the x or z-axis,
				// meaning that we went in a direction somewhere.  The next check makes
				// sure that we don't always check to step every time we collide.  If
				// the normal of the plane has a Y value of 1, that means it's just the
				// flat ground and we don't need to check if we can step over it, it's flat!
				if((vStart.x != vEnd.x || vStart.z != vEnd.z) && pPlane->vNormal.y != 1 && pPlane->vNormal.y >= 0.0f)
				{
					// We can try and step over the wall we collided with
					m_bTryStep = true;
				}

				// Here we make sure that we don't slide slowly down walls when we
				// jump and collide into them.  We only want to say that we are on
				// the ground if we actually have stopped from falling.  A wall wouldn't
				// have a high y value for the normal, it would most likely be 0.
				if(m_vCollisionNormal.y >= 0.2f)
					m_bGrounded = true;

				if(m_vCollisionNormal.y <= -0.2f)
					m_bCeiling = true;
			}
		}
		else
		//if(endDistance > 0)
		{
			// Get the ratio of the current brush side for the endRatio
			float Ratio = (startDistance + EPSILON) / (startDistance - endDistance);
			//float Ratio = (startDistance) / (startDistance - endDistance);

			// If the ratio is less than the current endRatio, assign a new endRatio.
			// This will usually always be true when starting out.
			if(Ratio < endRatio)
				endRatio = Ratio;
		}
	}

	// If we didn't start outside of the brush we don't want to count this collision - return;
	if(startsOut == false)
	{
		if(!getsOut)
		{
			m_traceRatio = 0;
			return;
		}
		//UnstuckBrush(pBrush, vStart);
		//UnstuckBrush(pBrush, m_vTrace);
		//m_stuck = true;
		//char msg[128];
		//sprintf(msg, "stuck @ %d", (int)GetTickCount());
		//Chat(msg);
		//return;
	}
	
	// If our startRatio is less than the endRatio there was a collision!!!
	if(startRatio < endRatio)
	{
		// Make sure the startRatio moved from the start and check if the collision
		// ratio we just got is less than the current ratio stored in m_traceRatio.
		// We want the closest collision to our original starting position.
		//if(startRatio > -1 && startRatio < m_traceRatio)
		if(startRatio > START_RATIO && startRatio < m_traceRatio)
		//if(startRatio < m_traceRatio)
		{
			// If the startRatio is less than 0, just set it to 0
			//if(startRatio < 0)
			//	startRatio = 0;

			// Store the new ratio in our member variable for later
			m_traceRatio = startRatio;
		}
	}
}

void Q3BSP::RenderFace(int faceIndex)
{
	tBSPFace *pFace = &m_pFaces[faceIndex];
	Shader* s = &g_shader[g_curS];

#if 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
#endif//TODO


	glVertexPointer(3, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vPosition));
	glEnableClientState(GL_VERTEX_ARRAY);

	// Set the current pass as the first texture (For multi-texturing)
	//glActiveTextureARB(GL_TEXTURE0_ARB);
	glActiveTexture(GL_TEXTURE0);

	// Give OpenGL the texture coordinates for the first texture, and enable that texture
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vTextureCoord));

	// Set our vertex array client states for allowing texture coordinates
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Turn on texture arrays for the first pass
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Turn on texture mapping and bind the face's texture map
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,  m_textures[pFace->textureID]);

	// Set the current pass as the second lightmap texture_
	glActiveTextureARB(GL_TEXTURE1_ARB);

	// Turn on texture arrays for the second lightmap pass
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Next, we need to specify the UV coordinates for our lightmaps.  This is done
	// by switching to the second texture and giving OpenGL our lightmap array.
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vLightmapCoord));

	// Turn on texture mapping and bind the face's lightmap over the texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,  m_lightmaps[pFace->lightmapID]);

	// Render our current face to the screen with vertex arrays
	glDrawElements(GL_TRIANGLES, pFace->numOfIndices, GL_UNSIGNED_INT, &(m_pIndices[pFace->startIndex]) );
}

void Q3BSP::RenderSkyFace(int faceIndex)
{
// Here we grab the face from the index passed in
	tBSPFace *pFace = &m_pFaces[faceIndex];

	// Assign our array of face vertices for our vertex arrays and enable vertex arrays
	glVertexPointer(3, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vPosition));
	glEnableClientState(GL_VERTEX_ARRAY);

	glActiveTextureARB(GL_TEXTURE0_ARB);

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vTextureCoord));

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,  m_textures[pFace->textureID]);

	glDrawElements(GL_TRIANGLES, pFace->numOfIndices, GL_UNSIGNED_INT, &(m_pIndices[pFace->startIndex]) );
}

void Q3BSP::RenderSky()
{
	int i = m_numOfFaces;

	glDisable(GL_DEPTH_TEST);

	while(i--)
	{
		tBSPFace* pFace = &m_pFaces[i];

		if(!m_sky[pFace->textureID]) continue;
		
		RenderSkyFace(i);
	}

	glEnable(GL_DEPTH_TEST);
}

void Q3BSP::RenderLevel(const Vec3f &vPos)
{
	return;

	m_FacesDrawn.ClearAll();

	int leafIndex = FindLeaf(vPos);

	int cluster = m_pLeafs[leafIndex].cluster;
	int i = m_numOfLeafs;

	while(i--)
	{
		tBSPLeaf *pLeaf = &m_pLeafs[i];

		if(!IsClusterVisible(cluster, pLeaf->cluster)) 
			continue;

		if(!g_frustum.boxin((float)pLeaf->min.x, (float)pLeaf->min.y, (float)pLeaf->min.z,
		  	 				       (float)pLeaf->max.x, (float)pLeaf->max.y, (float)pLeaf->max.z))
			continue;
		
		int faceCount = pLeaf->numOfLeafFaces;

		while(faceCount--)
		{
			int faceIndex = m_pLeafFaces[pLeaf->leafface + faceCount];

			if(m_pFaces[faceIndex].type != FACE_POLYGON) continue;

			if(m_sky[m_pFaces[faceIndex].textureID]) continue;

			if(!m_FacesDrawn.On(faceIndex)) 
			{
				m_FacesDrawn.Set(faceIndex);
				RenderFace(faceIndex);
			}
		}			
	}
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	//glDisable(GL_TEXTURE_2D);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void Q3BSP::Destroy()
{
	if(m_pVerts) 
	{
		delete [] m_pVerts;		m_pVerts = NULL;
	}

	if(m_pFaces)	
	{
		delete [] m_pFaces;		m_pFaces = NULL;
	}
	
	if(m_pIndices)	
	{
		delete [] m_pIndices;
		m_pIndices = NULL;
	}

	if(m_pNodes)	
	{
		delete [] m_pNodes;		m_pNodes = NULL;
	}

	if(m_pLeafs)	
	{
		delete [] m_pLeafs;		m_pLeafs = NULL;
	}

	if(m_pLeafFaces)	
	{
		delete [] m_pLeafFaces;	m_pLeafFaces = NULL;
	}

	if(m_pPlanes)	
	{
		delete [] m_pPlanes;	m_pPlanes = NULL;
	}

	if(m_clusters.pBitsets)	
	{
		delete [] m_clusters.pBitsets;		m_clusters.pBitsets = NULL;
	}

	if(m_pBrushes)	
	{
		delete [] m_pBrushes;		m_pBrushes = NULL;
	}
	
	if(m_pBrushSides)	
	{
		delete [] m_pBrushSides;	m_pBrushSides = NULL;
	}

	if(m_pLeafBrushes)	
	{
		delete [] m_pLeafBrushes;	m_pLeafBrushes = NULL;
	}

	if(m_pTextures)	
	{
		delete [] m_pTextures;		m_pTextures = NULL;
	}	

	if(m_pLightmaps)
	{
		delete [] m_pLightmaps;		m_pLightmaps = NULL;
	}

	glDeleteTextures(m_numOfTextures, m_textures);
	glDeleteTextures(m_numOfLightmaps, m_lightmaps);
}

Q3BSP::~Q3BSP()
{
	Destroy();
}

