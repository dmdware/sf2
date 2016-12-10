




#include "main.h"
#include "3dmath.h"
#include "quake3bsp.h"
#include "frustum.h"
#include "shader.h"
#include "gui.h"
#include "debug.h"
#include "image.h"
#include "model.h"
#include "entity.h"
#include "billboard.h"

CQuake3BSP g_map;

int g_backdrop;

void DrawBackdrop()
{
	BeginVertexArrays();
	//g_model[g_backdrop].Draw(0, g_camera->Position(), 0);
	g_model[g_backdrop].Draw(0, CVector3(0,0,0), 0, 0);
	EndVertexArrays();
}

CQuake3BSP::CQuake3BSP()
{
	m_gridSize = CVector3(64.0f, 128.0f, 64.0f);

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
	m_numOfModels		= 0;
	m_numOfLightVols	= 0;

	//m_traceRatio		= 0;
	m_traceType			= 0;
	m_traceRadius		= 0;

	m_bCollided	= false;
	m_bGrounded	= false;
	m_bTryStep		= false;

	m_vTraceMins = CVector3(0, 0, 0);
	m_vTraceMaxs = CVector3(0, 0, 0);
	m_vExtents   = CVector3(0, 0, 0);
	
	m_vCollisionNormal = CVector3(0, 0, 0);
	
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
	m_pModels		 = NULL;
	m_pLightVols	 = NULL;
	
	m_brokenFace	 = NULL;
	m_brokenBrush	 = NULL;
}

void CQuake3BSP::ChangeGamma(byte *pImage, int size, float factor)
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

void CQuake3BSP::CreateLightmapTexture(UINT &texture, byte *pImageBits, int width, int height)
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

void CQuake3BSP::ReloadTextures()
{
	int i;

	for(i = 0; i < m_numOfTextures; i++)
	{
		CreateTexture(m_textures[i], m_pTextures[i].strName);
	}

	for(i = 0; i < m_numOfLightmaps ; i++)
	{
		CreateLightmapTexture(m_lightmaps[i], (unsigned char *)m_pLightmaps[i].imageBits, 128, 128);
	}
}

bool CQuake3BSP::LoadBSP(const char *name)
{
	FILE *fp = NULL;
	int i = 0;

	char bspPath[128];
	sprintf(bspPath, "tmpQuArK/maps/%s.bsp", name);

	g_log<<bspPath<<endl;

	if((fp = fopen(bspPath, "rb")) == NULL)
	{
		g_log<<"Could not find BSP file "<<name<<endl;
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
	m_brokenFace = new bool [m_numOfFaces];

	m_numOfIndices = lumps[kIndices].length / sizeof(int);
	m_pIndices     = new int [m_numOfIndices];

	m_numOfTextures = lumps[kTextures].length / sizeof(tBSPTexture);
	m_pTextures = new tBSPTexture [m_numOfTextures];
	m_textures = new UINT [m_numOfTextures];
	m_passable = new bool [m_numOfTextures];
	m_sky = new bool [m_numOfTextures];
	m_transparent = new bool [m_numOfTextures];
	m_water = new bool [m_numOfTextures];
	m_breakable = new bool [m_numOfTextures];
	m_ladder = new bool [m_numOfTextures];
	m_grate = new bool [m_numOfTextures];
 
	m_numOfLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
	m_pLightmaps = new tBSPLightmap [m_numOfLightmaps];
	m_lightmaps = new UINT [m_numOfLightmaps];

	g_log<<"m_numOfLightmaps = "<<m_numOfLightmaps<<endl;

	m_numOfModels = lumps[kModels].length / sizeof(tBSPModel);
	m_pModels = new tBSPModel [m_numOfModels];

	m_numOfLightVols = lumps[kLightVolumes].length / sizeof(tBSPLightVol);
	m_pLightVols = new tBSPLightVol [m_numOfLightVols];

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
		m_passable[i] = false;
		m_sky[i] = false;
		m_water[i] = false;
		m_transparent[i] = false;
		m_breakable[i] = false;
		m_ladder[i] = false;
		m_grate[i] = false;
	}

	for(i = 0; i < m_numOfTextures; i++)
	{
		if(strstr(m_pTextures[i].strName, "~"))
			m_passable[i] = true;
		if(strstr(m_pTextures[i].strName, "^"))
			m_sky[i] = true;
		if(strstr(m_pTextures[i].strName, "$"))
			m_water[i] = true;
		if(strstr(m_pTextures[i].strName, "%"))
			m_transparent[i] = true;
		if(strstr(m_pTextures[i].strName, "!"))
			m_breakable[i] = true;
		if(strstr(m_pTextures[i].strName, "#"))
			m_ladder[i] = true;
		if(strstr(m_pTextures[i].strName, "`"))
			m_grate[i] = true;

		PathStripPath(m_pTextures[i].strName);
		char filename[32];
		strcpy(filename, m_pTextures[i].strName);
		sprintf(m_pTextures[i].strName, "tmpQuArK\\textures\\%s", filename);

		FindTextureExtension(m_pTextures[i].strName);
	}

	int vIndex;
	for(int faceIndex=0; faceIndex<m_numOfFaces; faceIndex++)
	{
		m_brokenFace[faceIndex] = false;

		if(m_pFaces[faceIndex].type != FACE_POLYGON) continue;
		if(!m_transparent[m_pFaces[faceIndex].textureID]) continue;

		CSortFace sortFace;
		sortFace.faceIndex = faceIndex;
		CVector3 vPos(0, 0, 0);
		
		//m_pFaces[faceIndex].
		tBSPFace* pFace = &m_pFaces[faceIndex];
		
		for(i=0; i<pFace->numOfIndices; i++)
		{
			vIndex = m_pIndices[pFace->startIndex+i] + pFace->startVertIndex;

			if(vIndex < 0 || vIndex >= m_numOfVerts)
			{
				g_log<<"vIndex out 0 <= "<<vIndex<<" < "<<m_numOfVerts<<endl;
				g_log.flush();
			}

			vPos = m_pVerts[vIndex].vPosition / (float)(i+1) + vPos * (float)i/(float)(i+1);
		}
		//m_pVerts[pFace->startVertIndex].vPosition
		//pFace->numOfIndices, GL_UNSIGNED_INT, &(m_pIndices[pFace->startIndex]
		//vPos = m_pVerts[pFace->startVertIndex].vPosition;
		sortFace.pos = vPos;
		m_sortFaces.push_back(sortFace);
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

	m_numOfBrushes = lumps[kBrushes].length / sizeof(tBSPBrush); //sizeof(int);	// Bug fix - Denis
	m_pBrushes     = new tBSPBrush [m_numOfBrushes];
	m_brokenBrush  = new bool [m_numOfBrushes];

	for(int i=0; i<m_numOfBrushes; i++)
		m_brokenBrush[i] = false;
	
	fseek(fp, lumps[kBrushes].offset, SEEK_SET);
	fread(m_pBrushes, m_numOfBrushes, sizeof(tBSPBrush), fp);

	m_numOfBrushSides = lumps[kBrushSides].length / sizeof(tBSPBrushSide);	//sizeof(int);	// Bug fix - Denis
	m_pBrushSides     = new tBSPBrushSide [m_numOfBrushSides];

	fseek(fp, lumps[kBrushSides].offset, SEEK_SET);
	fread(m_pBrushSides, m_numOfBrushSides, sizeof(tBSPBrushSide), fp);

	m_numOfLeafBrushes = lumps[kLeafBrushes].length / sizeof(int);
	m_pLeafBrushes     = new int [m_numOfLeafBrushes];

	fseek(fp, lumps[kLeafBrushes].offset, SEEK_SET);
	fread(m_pLeafBrushes, m_numOfLeafBrushes, sizeof(int), fp);

	fseek(fp, lumps[kModels].offset, SEEK_SET);
	fread(m_pModels, m_numOfModels, sizeof(tBSPModel), fp);

	for(i=0; i<m_numOfModels; i++)
	{
		float temp = m_pModels[i].mins.y;
		m_pModels[i].mins.y = m_pModels[i].mins.z;
		m_pModels[i].mins.z = -temp;

		temp = m_pModels[i].maxs.y;
		m_pModels[i].maxs.y = m_pModels[i].maxs.z;
		m_pModels[i].maxs.z = -temp;
		
		float tempmin = min(m_pModels[i].mins.z, m_pModels[i].maxs.z);
		float tempmax = max(m_pModels[i].mins.z, m_pModels[i].maxs.z);
		m_pModels[i].mins.z = tempmin;
		m_pModels[i].maxs.z = tempmax;
	}
	
	m_bbox.min = m_pModels[0].mins;
	m_bbox.max = m_pModels[0].maxs;

	g_log<<"map bbox min="<<m_bbox.min.x<<","<<m_bbox.min.y<<","<<m_bbox.min.z<<" max="<<m_bbox.max.x<<","<<m_bbox.max.y<<","<<m_bbox.max.z<<endl;
	g_log<<"m_numOfLightVols = "<<m_numOfLightVols<<endl;

	fseek(fp, lumps[kLightVolumes].offset, SEEK_SET);
	fread(m_pLightVols, m_numOfLightVols, sizeof(tBSPLightVol), fp);

	/*
	
	file->setPosition(header.lumps[LUMP_MODELS].offset);
	int num_models=header.lumps[LUMP_MODELS].length/sizeof(Q3MODEL);

	Q3MODEL *models=new Q3MODEL[num_models];
	file->readVOID(models,num_models*sizeof(Q3MODEL));

	gridsize=VECTOR3(64.0f,128.0f,64.0f);
	worldmodel->bbox=QBBox(models[0].mins,models[0].maxs);

	worldmodel->num_lightvols_x=(unsigned int)(floorf(worldmodel->bbox.max.x/worldmodel->gridsize.x)-ceilf(worldmodel->bbox.min.x/worldmodel->gridsize.x)+1);
	worldmodel->num_lightvols_y=(unsigned int)(floorf(worldmodel->bbox.max.y/worldmodel->gridsize.y)-ceilf(worldmodel->bbox.min.y/worldmodel->gridsize.y)+1);
	worldmodel->num_lightvols_z=(unsigned int)(floorf(worldmodel->bbox.max.z/worldmodel->gridsize.z)-ceilf(worldmodel->bbox.min.z/worldmodel->gridsize.z)+1);

	unsigned int realnum_lightvols=worldmodel->num_lightvols_x*worldmodel->num_lightvols_y*worldmodel->num_lightvols_z;
	if (realnum_lightvols!=worldmodel->num_lightvols)
	{
		std::cout<<"Q3BSPLoader::ReadLightVols():\n   ^3 calculated number of lightvols differs from bsp file"<<std::endl;
		return;
	}
	*/
	
	num_lightvols.x = (unsigned int) (floorf(m_bbox.max.x/m_gridSize.x) - ceilf(m_bbox.min.x/m_gridSize.x) + 1);
	num_lightvols.y = (unsigned int) (floorf(m_bbox.max.y/m_gridSize.y) - ceilf(m_bbox.min.y/m_gridSize.y) + 1);
	num_lightvols.z = (unsigned int) (floorf(m_bbox.max.z/m_gridSize.z) - ceilf(m_bbox.min.z/m_gridSize.z) + 1);

	unsigned int realnum_lightvols = num_lightvols.x * num_lightvols.y * num_lightvols.z;

	if(realnum_lightvols != m_numOfLightVols)
	{
		g_log<<"realnum_lightvols ("<<realnum_lightvols<<") != m_numOfLightVols ("<<m_numOfLightVols<<")"<<endl;
	}

	/*
	int billb = Billboard("fog%~");
	for(float x=m_bbox.min.x; x < m_bbox.max.x; x+=m_gridSize.x)
	{
		for(float y=m_bbox.min.y; y < m_bbox.max.y; y+=m_gridSize.y)
		{
			for(float z=m_bbox.min.z; z < m_bbox.max.z; x+=m_gridSize.z)
			{
				PlaceBillboard(billb, CVector3(x, y, z), 10, -1);	
			}
		}
	}*/

	fseek(fp, lumps[kEntities].offset, SEEK_SET);
	char* entities = new char[ lumps[kEntities].length ];
	fread(entities, lumps[kEntities].length, sizeof(char), fp);
	ReadEntities(entities);
	delete entities;

	fclose(fp);

	m_FacesDrawn.Resize(m_numOfFaces);

	g_log<<"Reloading textures (map)..."<<endl;
	g_log.flush();

	ReloadTextures();

	g_log<<"Done loading map."<<endl;
	g_log.flush();

	return true;
}

int CQuake3BSP::FindLeaf(const CVector3 &vPos)
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

		// Use the Plane Equation (Ax + by + Cz + D = 0) to find if the
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

int CQuake3BSP::FindCluster(const CVector3 &vPos)
{
	int leaf = FindLeaf(vPos);
	return m_pLeafs[leaf].cluster;
}

inline int CQuake3BSP::IsClusterVisible(int current, int test)
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

template <class T>
inline T trilinear(const float tx, const float ty, const float tz, T *p)
{
	const float tx2=1.0f-tx;
	const float ty2=1.0f-ty;
	const float tz2=1.0f-tz;

	T np;
	np=p[0]*(tx2*ty2*tz2)+p[1]*(tx*ty2*tz2)+p[2]*(tx2*ty2*tz)+p[3]*(tx*ty2*tz)+
	   p[4]*(tx2*tz2*ty)+p[5]*(tx*ty*tz2)+p[6]*(tx2*ty*tz)+p[7]*(tx*ty*tz);
	return np;
}

template <class T>
inline T trilinear2(const float tx, const float ty, const float tz, const T &p0, const T &p1, const T &p2, const T &p3, const T &p4, const T &p5, const T &p6, const T &p7)
{
	const float tx2=1.0f-tx;
	const float ty2=1.0f-ty;
	const float tz2=1.0f-tz;

	T np;
	np=p0*(tx2*ty2*tz2)+p1*(tx*ty2*tz2)+p2*(tx2*ty2*tz)+p3*(tx*ty2*tz)+
	   p4*(tx2*tz2*ty)+p5*(tx*ty*tz2)+p6*(tx2*ty*tz)+p7*(tx*ty*tz);
	return np;
}

CVector3 CQuake3BSP::LightVol(CVector3 vPos)
{
	CVector3 color;

	//g_log<<"get light "<<vPos.x<<","<<vPos.y<<","<<vPos.z<<endl;
	//g_log.flush();

	if(m_numOfLightVols <= 0)
	{
		//g_log<<"access0"<<endl;
		//g_log.flush();

		color.x = 1;
		color.y = 1;
		color.z = 1;
		
		//g_log<<"access00"<<endl;
		//g_log.flush();

		return color;
	}
	
	//g_log<<"got light"<<endl;
	//g_log.flush();

#ifdef DEBUG_2

	if(strstr(g_debug2str, "zombie2"))
	{
		g_log<<"----------------------"<<endl;
		g_log<<"g_debug2val = "<<g_debug2val<<endl;
		g_log<<"before vPos.y = "<<vPos.y<<endl;
	}
#endif

	vPos.x = vPos.x - m_bbox.min.x;
	vPos.y = vPos.y - m_bbox.min.y;
	vPos.z = vPos.z - m_bbox.min.z;
	
	float px = vPos.x / m_gridSize.x;
	float py = vPos.y / m_gridSize.y;
	float pz = vPos.z / m_gridSize.z;
	
	unsigned int lx = (unsigned int)px; 
	//unsigned int lx = num_lightvols.x - (unsigned int)px; 
	//unsigned int ly = (unsigned int)(py-1);
	unsigned int ly = (unsigned int)(py);
	//unsigned int ly = num_lightvols.y - (unsigned int)(py);
	unsigned int lz = (unsigned int)(num_lightvols.z - pz);
	//unsigned int lz = (unsigned int)(num_lightvols.y - pz);
	//unsigned int lz = (unsigned int)pz;

	
#ifdef DEBUG_3
	if(g_debug3)
	{
		g_log<<"lightvol "<<lx<<"/"<<num_lightvols.x<<","<<ly<<"/"<<num_lightvols.y<<","<<lz<<"/"<<num_lightvols.z<<endl;
		g_log.flush();
	}
#endif


#ifdef DEBUG_2
	//g_log<<"lightvol "<<lx<<"/"<<num_lightvols.x<<","<<ly<<"/"<<num_lightvols.y<<","<<lz<<"/"<<num_lightvols.z<<endl;
	//g_log.flush();
	//g_log<<"py = "<<py<<endl;
	//g_log<<"m_gridSize.y = "<<m_gridSize.y<<endl;
	//g_log.flush();

	if(strstr(g_debug2str, "zombie2"))
	{
		g_log<<"py = "<<py<<endl;
		g_log<<"vPos.y = "<<vPos.y<<endl;
		g_log<<"m_bbox.min.y = "<<m_bbox.min.y<<endl;
		g_log<<"m_gridSize.y = "<<m_gridSize.y<<endl;
		g_log<<"num_lightvols.y = "<<num_lightvols.y<<endl;
		g_log<<"g_debug2v "<<g_debug2v.x<<","<<g_debug2v.y<<","<<g_debug2v.z<<endl;
		g_log<<"g_debug2v2 "<<g_debug2v2.x<<","<<g_debug2v2.y<<","<<g_debug2v2.z<<endl;
		g_log<<"g_debug2v+g_debug2v2 "<<(g_debug2v2.x+g_debug2v.x)<<","<<(g_debug2v2.y+g_debug2v.y)<<","<<(g_debug2v2.z+g_debug2v.z)<<endl;
		g_log.flush();
	}
#endif
	
	//if(lx > num_lightvols.x - 2 || ly > num_lightvols.y - 2 || lz > num_lightvols.z - 2)
	if(lx > num_lightvols.x - 1 || ly > num_lightvols.y - 1 || lz > num_lightvols.z - 1)
	{
		color.x = 0;
		color.y = 0;
		color.z = 0;
		char msg[128];
#ifndef DEBUG_2
		int g_debug2val = -1;
		char g_debug2str[32];
		CVector3 g_debug2v;
		CVector3 g_debug2v2;
#endif
		g_log<<"lightvolout "<<lx<<"/"<<num_lightvols.x<<","<<ly<<"/"<<num_lightvols.y<<","<<lz<<"/"<<num_lightvols.z<<endl;
		g_log.flush();

		sprintf(msg, "out of lightgrid %d %s %f,%f,%f @ %d", g_debug2val, g_debug2str, g_debug2v.x, g_debug2v.y, g_debug2v.z, (int)GetTickCount());
		g_debug2val = -1;
		Chat(msg);
		return color;
	}
	
	px = px - floorf(px);
	py = py - floorf(py);
	pz = pz - floorf(pz);
	
	/*
		float temp = m_pVerts[i].vPosition.y;
		m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
		m_pVerts[i].vPosition.z = -temp;
		*/

	unsigned int elem1 = ly*num_lightvols.z*num_lightvols.x + lz*num_lightvols.x + lx;
	//unsigned int elem1 = lz * num_lightvols.z * num_lightvols.y + ly * num_lightvols.x + lx;
	//unsigned int elem1 = ly * num_lightvols.z * num_lightvols.x + lx * num_lightvols.z + lz;
	//unsigned int elem1 = lz * num_lightvols.y * num_lightvols.x + lx * num_lightvols.y + ly;
	//unsigned int elem1 = lx * num_lightvols.y * num_lightvols.z + lz * num_lightvols.y + ly;
	/*
	//Original
	const unsigned int elem2 = elem1 + num_lightvols.x;
	const unsigned int elem3 = elem2 + num_lightvols.x*num_lightvols.z;
	const unsigned int elem4 = elem3 - num_lightvols.x;
	*/
	const unsigned int elem2 = elem1 - num_lightvols.x;
	const unsigned int elem3 = elem2 + num_lightvols.x*num_lightvols.z;
	const unsigned int elem4 = elem3 + num_lightvols.x;

	
#ifdef DEBUG_3
	if(g_debug3)
	{
		g_log<<"elem1 "<<elem1<<"/"<<m_numOfLightVols<<endl;
		g_log<<"elem2 "<<elem2<<"/"<<m_numOfLightVols<<endl;
		g_log<<"elem3 "<<elem3<<"/"<<m_numOfLightVols<<endl;
		g_log<<"elem4 "<<elem4<<"/"<<m_numOfLightVols<<endl;
		g_log.flush();
	}
#endif
	

	CVector3 temp[8];
	temp[0] = m_pLightVols[elem1].ambient + m_pLightVols[elem1].directional;
	temp[1] = m_pLightVols[elem1+1].ambient + m_pLightVols[elem1+1].directional;
	temp[2] = m_pLightVols[elem2].ambient + m_pLightVols[elem2].directional;
	temp[3] = m_pLightVols[elem2+1].ambient + m_pLightVols[elem2+1].directional;
	temp[4] = m_pLightVols[elem4].ambient + m_pLightVols[elem4].directional;
	temp[5] = m_pLightVols[elem4+1].ambient + m_pLightVols[elem4+1].directional;
	temp[6] = m_pLightVols[elem3].ambient + m_pLightVols[elem3].directional;
	temp[7] = m_pLightVols[elem3+1].ambient + m_pLightVols[elem3+1].directional;
	color = VMin(255, trilinear(px, py, pz, temp))/255.0f;

	//color.x = min(255, (int)m_pLightVols[elem1].ambient.x + (int)m_pLightVols[elem1].directional.x);
	//color.y = min(255, (int)m_pLightVols[elem1].ambient.y + (int)m_pLightVols[elem1].directional.y);
	//color.z = min(255, (int)m_pLightVols[elem1].ambient.z + (int)m_pLightVols[elem1].directional.z);
	
	//color.x = m_pLightVols[elem1].ambient.x;
	//color.y = m_pLightVols[elem1].ambient.y;
	//color.z = m_pLightVols[elem1].ambient.z;
	
	//color.x = m_pLightVols[elem1].directional.x;
	//color.y = m_pLightVols[elem1].directional.y;
	//color.z = m_pLightVols[elem1].directional.z;

	return color;

	/*
		if (!lightvols) return false;
	pos.x=pos.x-bbox.min.x;
	pos.y=pos.y-bbox.min.y;
	pos.z=bbox.max.z-pos.z;

	float px=pos.x/gridsize.x;
	float py=pos.y/gridsize.y;
	float pz=pos.z/gridsize.z;

	unsigned int lx=(unsigned int)px; 
	unsigned int ly=(unsigned int)(py+0.5f);
	unsigned int lz=(unsigned int)pz;

	if (lx>num_lightvols_x-2||ly>num_lightvols_y-2||lz>num_lightvols_z-2) return false;

	px=px-floorf(px);
	py=py-floorf(py);
	pz=pz-floorf(pz);

	//FIXME: this check is not sufficiant and causes crashes if you leave the map upwards
	//if (elem>=num_lightvols) return false;

	// by using lat and long instead of computed normals 1) space is saved to exactly 8 bytes
	// 2) both angles can be interpolated linearly
	// the ugly thing is that lat and long are converted to float every time
	const unsigned int elem1=ly*num_lightvols_z*num_lightvols_x+lz*num_lightvols_x+lx;
	const unsigned int elem2=elem1+num_lightvols_x;
	const unsigned int elem3=elem2+num_lightvols_x*num_lightvols_z;
	const unsigned int elem4=elem3-num_lightvols_x;

	VECTOR3 temp[8];
	temp[0]=lightvols[elem1].ambient;
	temp[1]= lightvols[elem1+1].ambient;
	temp[2]= lightvols[elem2].ambient;
	temp[3]= lightvols[elem2+1].ambient;
	temp[4]= lightvols[elem4].ambient;
	temp[5]= lightvols[elem4+1].ambient;
	temp[6]= lightvols[elem3].ambient;
	temp[7]= lightvols[elem3+1].ambient;
	light.ambient=trilinear(px,py,pz,temp)*(1.0f/255.0f);

	temp[0]=lightvols[elem1].directional;
	temp[1]= lightvols[elem1+1].directional;
	temp[2]= lightvols[elem2].directional;
	temp[3]= lightvols[elem2+1].directional;
	temp[4]= lightvols[elem4].directional;
	temp[5]= lightvols[elem4+1].directional;
	temp[6]= lightvols[elem3].directional;
	temp[7]= lightvols[elem3+1].directional;
	light.diffuse=trilinear(px,py,pz,temp)*(1.0f/255.0f);

	float newlng=trilinear2(px,py,pz,(float)lightvols[elem1].latlng[0],
									(float)lightvols[elem1+1].latlng[0],
									(float)lightvols[elem2].latlng[0],
									(float)lightvols[elem2+1].latlng[0],
									(float)lightvols[elem4].latlng[0],
									(float)lightvols[elem4+1].latlng[0],
									(float)lightvols[elem3].latlng[0],
									(float)lightvols[elem3+1].latlng[0]);

	float newlat=trilinear2(px,py,pz,(float)lightvols[elem1].latlng[1],
								(float)lightvols[elem1+1].latlng[1],
								(float)lightvols[elem2].latlng[1],
								(float)lightvols[elem2+1].latlng[1],
								(float)lightvols[elem4].latlng[1],
								(float)lightvols[elem4+1].latlng[1],
								(float)lightvols[elem3].latlng[1],
								(float)lightvols[elem3+1].latlng[1]);

	//(VECTOR3&)light.pos=latlng2vec[((int)newlng)<<8|(int)newlat];
	getVecFromLatLong(newlat*PI/128.0f,newlng*PI/128.0f,(VECTOR3&)light.pos);
	light.pos[3]=0.0f;

	light.specular=NullVector4;
	light.cutoff=180.0f;

	light.used_attribs=DIRECTIONAL_LIGHT;
	*/
}

CVector3 CQuake3BSP::TryToStep(CVector3 vStart, CVector3 vEnd, float maxStep)
{
	// In this function we loop until we either found a reasonable height
	// that we can step over, or find out that we can't step over anything.
	// We check 10 times, each time increasing the step size to check for
	// a collision.  If we don't collide, then we climb over the step.

	// Go through and check different heights to step up
	for(float height = 1.0f; height <= maxStep; height++)
	//float height = 15;
	{
		// Reset our variables for each loop interation
		m_bCollided = false;
		m_bTryStep = false;

		// Here we add the current height to our y position of a new start and end.
		// If these 2 new start and end positions are okay, we can step up.
		CVector3 vStepStart = CVector3(vStart.x, vStart.y + height, vStart.z);
		CVector3 vStepEnd   = CVector3(vEnd.x, vStart.y + height, vEnd.z);

		// Test to see if the new position we are trying to step collides or not
		CVector3 vStepPosition = Trace(vStepStart, vStepEnd);

		// If we didn't collide, we can step!
		if(!m_bCollided)
		{
			// Here we get the current view, then increase the y value by the current height.
			// This makes it so when we are walking up the stairs, our view follows our step
			// height and doesn't sag down as we walk up the stairs.
			//CVector3 vNewView = g_camera->View();					
			//g_camera->SetView(CVector3(vNewView.x, vNewView.y + height, vNewView.z));

			// Return the current position since we stepped up somewhere
			return vStepPosition;		
		}
	}

	// If we can't step, then we just return the original position of the collision
	return vStart;
}

CVector3 CQuake3BSP::Trace(CVector3 vStart, CVector3 vEnd)
{
	// Initially we set our trace ratio to 1.0f, which means that we don't have
	// a collision or intersection point, so we can move freely.
	m_traceRatio = 1.0f;
	
	// We start out with the first node (0), setting our start and end ratio to 0 and 1.
	// We will recursively go through all of the nodes to see which brushes we should check.
	CheckNode(0, vStart, vEnd);

	// If the traceRatio is STILL 1.0f, then we never collided and just return our end position
	if(m_traceRatio == 1.0f)
		return vEnd;
	else	// Else COLLISION!!!!
	{
		// Set our new position to a position that is right up to the brush we collided with
		CVector3 vNewPosition = vStart + ((vEnd - vStart) * m_traceRatio);

		if(m_traceType == TYPE_RAY)
			return vNewPosition;

		// Get the distance from the end point to the new position we just got
		CVector3 vMove = vEnd - vNewPosition;

		// Get the distance we need to travel backwards to the new slide position.
		// This is the distance of course along the normal of the plane we collided with.
		float distance = Dot(vMove, m_vCollisionNormal);

		// Get the new end position that we will end up (the slide position).
		CVector3 vEndPosition = vEnd - m_vCollisionNormal*distance;
		
		// Since we got a new position for our sliding vector, we need to check
		// to make sure that new sliding position doesn't collide with anything else.
		vNewPosition = Trace(vNewPosition, vEndPosition);

		if(m_vCollisionNormal.y > 0.2f || m_bGrounded)
			m_bGrounded = true;
		else
			m_bGrounded = false;

		// Return the new position to be used by our camera (or player)
		return vNewPosition;
	}
}

void CQuake3BSP::Break(CVector3 vStart, CVector3 vEnd)
{
	// Initially we set our trace ratio to 1.0f, which means that we don't have
	// a collision or intersection point, so we can move freely.
	m_traceRatio = 1.0f;
	
	// We start out with the first node (0), setting our start and end ratio to 0 and 1.
	// We will recursively go through all of the nodes to see which brushes we should check.
	BreakNode(0, vStart, vEnd);
}

CVector3 CQuake3BSP::TraceRay(CVector3 vStart, CVector3 vEnd)
{
	// We don't use this function, but we set it up to allow us to just check a
	// ray with the BSP tree brushes.  We do so by setting the trace type to TYPE_RAY.
	m_traceType = TYPE_RAY;
	m_bCollided = false;

	m_vStart = vStart;
	m_vEnd = vEnd;

	// Run the normal Trace() function with our start and end 
	// position and return a new position
	return Trace(vStart, vEnd);
}

bool CQuake3BSP::BreakFaces(CVector3 vStart, CVector3 vEnd)
{
	m_traceType = TYPE_RAY;
	m_bBroke = false;
	
	m_vStart = vStart;
	m_vEnd = vEnd;

	Break(vStart, vEnd);

	int leafIndex = FindLeaf(vStart);

	int cluster = m_pLeafs[leafIndex].cluster;
	int i = m_numOfLeafs;
	int faceCount;
	int faceIndex;
	tBSPLeaf* pLeaf;

	while(i--)
	{
		pLeaf = &m_pLeafs[i];

		if(!IsClusterVisible(cluster, pLeaf->cluster)) 
			continue;

		if(!g_frustum.BoxInFrustum((float)pLeaf->min.x, (float)pLeaf->min.y, (float)pLeaf->min.z,
		  	 				       (float)pLeaf->max.x, (float)pLeaf->max.y, (float)pLeaf->max.z))
			continue;
		
		faceCount = pLeaf->numOfLeafFaces;

		while(faceCount--)
		{
			faceIndex = m_pLeafFaces[pLeaf->leafface + faceCount];

			if(m_pFaces[faceIndex].type != FACE_POLYGON) continue;
			
			if(!m_breakable[m_pFaces[faceIndex].textureID]) continue;
			if(m_brokenFace[faceIndex]) continue;

			BreakFace(faceIndex, vStart, vEnd);
		}			
	}

	return m_bBroke;
}

void CQuake3BSP::BreakFace(int faceIndex, CVector3 vStart, CVector3 vEnd)
{
	int vIndex;
	tBSPFace* pFace = &m_pFaces[faceIndex];
	CVector3* vPoly = new CVector3[pFace->numOfIndices];
		
	for(int i=0; i<pFace->numOfIndices; i++)
	{
		vIndex = m_pIndices[pFace->startIndex+i] + pFace->startVertIndex;
		vPoly[i] = m_pVerts[vIndex].vPosition;
	}

	CVector3 vLine[2];
	vLine[0] = vStart;
	vLine[1] = vEnd;

	if(IntersectedPolygon(vPoly, vLine, pFace->numOfIndices, NULL))
	{
		m_bBroke = true;
		m_brokenFace[faceIndex] = true;
	}

	delete [] vPoly;
}

CVector3 CQuake3BSP::TraceSphere(CVector3 vStart, CVector3 vEnd, float radius, float maxStep)
{
	m_traceType = TYPE_SPHERE;
	m_bCollided = false;

	m_vStart = vStart;
	m_vEnd = vEnd;

	m_bTryStep = false;
	m_bGrounded = false;

	m_traceRadius = radius;

	CVector3 vNewPosition = Trace(vStart, vEnd);

	if(m_bCollided && m_bTryStep)
		vNewPosition = TryToStep(vNewPosition, vEnd, maxStep);
	
	return vNewPosition;
}

CVector3 CQuake3BSP::TraceBox(CVector3 vStart, CVector3 vEnd, CVector3 vMin, CVector3 vMax, float maxStep)
{
	m_traceType = TYPE_BOX;
	m_vTraceMaxs = vMax;
	m_vTraceMins = vMin;
	m_bCollided = false;

	m_vStart = vStart;
	m_vEnd = vEnd;

	m_bTryStep = false;
	m_bGrounded = false;
	m_bLadder = false;

	// Grab the extend of our box (the largest size for each x, y, z axis)
	m_vExtents = CVector3(-m_vTraceMins.x > m_vTraceMaxs.x ? -m_vTraceMins.x : m_vTraceMaxs.x,
						  -m_vTraceMins.y > m_vTraceMaxs.y ? -m_vTraceMins.y : m_vTraceMaxs.y,
						  -m_vTraceMins.z > m_vTraceMaxs.z ? -m_vTraceMins.z : m_vTraceMaxs.z);

	CVector3 vNewPosition = Trace(vStart, vEnd);

	if(m_bCollided && m_bTryStep)
		vNewPosition = TryToStep(vNewPosition, vEnd, maxStep);
	
	m_bStuck = false;

	vNewPosition = Trace(vNewPosition, vNewPosition);

	if(m_bStuck)
		return vStart;

	return vNewPosition;
}

CVector3 CQuake3BSP::UnstuckNode(int nodeIndex, CVector3 vStart)
{
	CVector3 vNewPosition = vStart;

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
	//float endDistance = Dot(vEnd, pPlane->vNormal) - pPlane->d;
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
	if(startDistance >= offset)
	{
		// Traverse the BSP tree on all the nodes in front of this current splitter plane
		vNewPosition = UnstuckNode(pNode->front, vNewPosition);
	}
	// If both points are behind the current splitter plane, traverse down the back nodes
	else if(startDistance < -offset)
	{
		// Traverse the BSP tree on all the nodes in back of this current splitter plane
		vNewPosition = UnstuckNode(pNode->back, vNewPosition);
	}	
	else
	{
		vNewPosition = UnstuckNode(pNode->front, vNewPosition);
		vNewPosition = UnstuckNode(pNode->back, vNewPosition);
	}

	if(nodeIndex == 0 && vNewPosition != vStart)
		return UnstuckNode(0, vNewPosition);

	return vNewPosition;
}

// This traverses the BSP to find the brushes closest to our position
void CQuake3BSP::CheckNode(int nodeIndex, CVector3 vStart, CVector3 vEnd)
{
	int brushIndex;

	// Check if the next node is a leaf
	if(nodeIndex < 0)
	{
		// If this node in the BSP is a leaf, we need to negate and add 1 to offset
		// the real node index into the m_pLeafs[] array.  You could also do [~nodeIndex].
		tBSPLeaf *pLeaf = &m_pLeafs[-(nodeIndex + 1)];

		// We have a leaf, so let's go through all of the brushes for that leaf
		for(int i = 0; i < pLeaf->numOfLeafBrushes; i++)
		{
			brushIndex = m_pLeafBrushes[pLeaf->leafBrush + i];

			// Get the current brush that we going to check
			tBSPBrush *pBrush = &m_pBrushes[brushIndex];

			if(m_passable[pBrush->textureID])
				continue;

			if(m_traceType == TYPE_RAY && m_grate[pBrush->textureID])
				continue;

			if(m_brokenBrush[brushIndex])
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
		float Ratio1 = 1.0f, Ratio2 = 0.0f;	//, middleRatio = 0.0f;
		CVector3 vMiddle;	// This stores the middle point for our split ray

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

void CQuake3BSP::BreakNode(int nodeIndex, CVector3 vStart, CVector3 vEnd)
{
	int brushIndex;

	// Check if the next node is a leaf
	if(nodeIndex < 0)
	{
		// If this node in the BSP is a leaf, we need to negate and add 1 to offset
		// the real node index into the m_pLeafs[] array.  You could also do [~nodeIndex].
		tBSPLeaf *pLeaf = &m_pLeafs[-(nodeIndex + 1)];

		// We have a leaf, so let's go through all of the brushes for that leaf
		for(int i = 0; i < pLeaf->numOfLeafBrushes; i++)
		{
			brushIndex = m_pLeafBrushes[pLeaf->leafBrush + i];

			// Get the current brush that we going to check
			tBSPBrush *pBrush = &m_pBrushes[brushIndex];

			if(!m_breakable[pBrush->textureID])
				continue;

			if(m_brokenBrush[brushIndex])
				continue;

			// Check if we have brush sides and the current brush is solid and collidable
			if((pBrush->numOfBrushSides > 0) && (m_pTextures[pBrush->textureID].textureType & 1))
			{
				// Now we delve into the dark depths of the real calculations for collision.
				// We can now check the movement vector against our brush planes.
				BreakBrush(brushIndex, pBrush, vStart, vEnd);
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
		BreakNode(pNode->front, vStart, vEnd);
	}
	// If both points are behind the current splitter plane, traverse down the back nodes
	else if(startDistance < -offset && endDistance < -offset)
	{
		// Traverse the BSP tree on all the nodes in back of this current splitter plane
		BreakNode(pNode->back, vStart, vEnd);
	}	
	else
	{
		// If we get here, then our ray needs to be split in half to check the nodes
		// on both sides of the current splitter plane.  Thus we create 2 ratios.
		float Ratio1 = 1.0f, Ratio2 = 0.0f;	//, middleRatio = 0.0f;
		CVector3 vMiddle;	// This stores the middle point for our split ray

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
		BreakNode(side, vStart, vMiddle);

		// Now we need to make a middle point and ratio for the other side of the node
		//middleRatio = startRatio + ((endRatio - startRatio) * Ratio2);
		vMiddle = vStart + ((vEnd - vStart) * Ratio2);

		// Depending on which side should go last, traverse the bsp with the
		// other side of the split ray (movement vector).
		if(side == pNode->back)
			BreakNode(pNode->front, vMiddle, vEnd);
		else
			BreakNode(pNode->back, vMiddle, vEnd);
	}
}

CVector3 CQuake3BSP::UnstuckBrush(tBSPBrush *pBrush, CVector3 vStart)
{
	float greatestNegD = -9999999999999.0f;
	float tempNegD;
	float offsetD;
	CVector3 vNormal(0, 0, 0);

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
		//float endDistance = Dot(vEnd, pPlane->vNormal) - (pPlane->d + offset);

		// Store the offset that we will check against the plane
		CVector3 vOffset = CVector3(0, 0, 0);

		// If we are using AABB collision
		if(m_traceType == TYPE_BOX)
		{   
			// Grab the closest corner (x, y, or z value) that is closest to the plane
			vOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMaxs.x : m_vTraceMins.x;
			vOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMaxs.y : m_vTraceMins.y;
			vOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMaxs.z : m_vTraceMins.z;

			// Use the plane equation to grab the distance our start position is from the plane.
            startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;
			tempNegD = Dot(vStart, pPlane->vNormal) - pPlane->d;

			// Get the distance our end position is from this current brush plane
            //endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;

			if(startDistance < 0.0f && tempNegD > greatestNegD)
			{
				greatestNegD = tempNegD;
				offsetD = startDistance;
				vNormal = pPlane->vNormal;
			}
        }
	}
	
	if(vNormal != CVector3(0, 0, 0))
		vStart = vStart - vNormal * offsetD;

	return vStart;
}

// This checks our movement vector against all the planes of the brush
void CQuake3BSP::CheckBrush(tBSPBrush *pBrush, CVector3 vStart, CVector3 vEnd)
{
	float startRatio = -1.0f;		// Like in BrushCollision.htm, start a ratio at -1
    float endRatio = 1.0f;			// Set the end ratio to 1
    bool startsOut = false;			// This tells us if we starting outside the brush
	//CVector3 vCollisionNormal;
	//tBSPPlane *pColPlane;

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
		//float startDistance = Dot(vStart, pPlane->vNormal) - (pPlane->d + offset);
		//float endDistance = Dot(vEnd, pPlane->vNormal) - (pPlane->d + offset);
		float startDistance = Dot(m_vStart, pPlane->vNormal) - (pPlane->d + offset);
		float endDistance = Dot(m_vEnd, pPlane->vNormal) - (pPlane->d + offset);

		// Store the offset that we will check against the plane
		CVector3 vOffset = CVector3(0, 0, 0);

		// If we are using AABB collision
		if(m_traceType == TYPE_BOX)
		{
			// Grab the closest corner (x, y, or z value) that is closest to the plane
            vOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMaxs.x : m_vTraceMins.x;
			vOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMaxs.y : m_vTraceMins.y;
			vOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMaxs.z : m_vTraceMins.z;
            
			// Use the plane equation to grab the distance our start position is from the plane.
            //startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;
            startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;

			// Get the distance our end position is from this current brush plane
            //endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;
            endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;
        }

		// Make sure we start outside of the brush's volume
		if(startDistance > 0)	startsOut = true;

		// Stop checking since both the start and end position are in front of the plane
		if(startDistance > 0 && endDistance > 0)
			return;

		// Continue on to the next brush side if both points are behind or on the plane
		if(startDistance <= 0 && endDistance <= 0)
			continue;

		// If the distance of the start point is greater than the end point, we have a collision!
		//if(startDistance > endDistance)
		if(startDistance > 0)
		{
			// This gets a ratio from our starting point to the approximate collision spot
			float Ratio1 = (startDistance - EPSILON) / (startDistance - endDistance);
			//float Ratio1 = startDistance / (startDistance - endDistance);

			// If this is the first time coming here, then this will always be true,
			if(Ratio1 > startRatio)
			{
				// Set the startRatio (currently the closest collision distance from start)
				startRatio = Ratio1;
				//m_bCollided = true;		// Let us know we collided!	// BUG FIX - Denis

				// Store the normal of plane that we collided with for sliding calculations
				///vCollisionNormal = pPlane->vNormal;
				//pColPlane = pPlane;

				m_vCollisionNormal = pPlane->vNormal;

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
			}
		}
		//else
		if(endDistance > 0)
		{
			// Get the ratio of the current brush side for the endRatio
			float Ratio = (startDistance + EPSILON) / (startDistance - endDistance);

			// If the ratio is less than the current endRatio, assign a new endRatio.
			// This will usually always be true when starting out.
			if(Ratio < endRatio)
				endRatio = Ratio;
		}
	}

	m_bCollided = true;	// BUG FIX - Denis

	if(m_ladder[pBrush->textureID])
		m_bLadder = true;

	/*
	m_vCollisionNormal = vCollisionNormal;

	// This checks first tests if we actually moved along the x or z-axis,
	// meaning that we went in a direction somewhere.  The next check makes
	// sure that we don't always check to step every time we collide.  If
	// the normal of the plane has a Y value of 1, that means it's just the
	// flat ground and we don't need to check if we can step over it, it's flat!
	if((vStart.x != vEnd.x || vStart.z != vEnd.z) && pColPlane->vNormal.y != 1 && pColPlane->vNormal.y >= 0.0f)
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
		*/

	// If we didn't start outside of the brush we don't want to count this collision - return;
	if(startsOut == false)
	{
		m_bStuck = true;
		//UnstuckBrush(pBrush, m_vTrace);
		return;
	}
	
	// If our startRatio is less than the endRatio there was a collision!!!
	if(startRatio < endRatio)
	{
		// Make sure the startRatio moved from the start and check if the collision
		// ratio we just got is less than the current ratio stored in m_traceRatio.
		// We want the closest collision to our original starting position.
		if(startRatio > -1 && startRatio < m_traceRatio)
		//if(startRatio > -1 && startRatio < relativeRatio)
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

void CQuake3BSP::BreakBrush(int brushIndex, tBSPBrush *pBrush, CVector3 vStart, CVector3 vEnd)
{
	float startRatio = -1.0f;		// Like in BrushCollision.htm, start a ratio at -1
    float endRatio = 1.0f;			// Set the end ratio to 1
    bool startsOut = false;			// This tells us if we starting outside the brush
	//CVector3 vCollisionNormal;
	//tBSPPlane *pColPlane;

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
		//float startDistance = Dot(vStart, pPlane->vNormal) - (pPlane->d + offset);
		//float endDistance = Dot(vEnd, pPlane->vNormal) - (pPlane->d + offset);
		float startDistance = Dot(m_vStart, pPlane->vNormal) - (pPlane->d + offset);
		float endDistance = Dot(m_vEnd, pPlane->vNormal) - (pPlane->d + offset);

		// Store the offset that we will check against the plane
		CVector3 vOffset = CVector3(0, 0, 0);

		// If we are using AABB collision
		if(m_traceType == TYPE_BOX)
		{
			// Grab the closest corner (x, y, or z value) that is closest to the plane
            vOffset.x = (pPlane->vNormal.x < 0)	? m_vTraceMaxs.x : m_vTraceMins.x;
			vOffset.y = (pPlane->vNormal.y < 0)	? m_vTraceMaxs.y : m_vTraceMins.y;
			vOffset.z = (pPlane->vNormal.z < 0)	? m_vTraceMaxs.z : m_vTraceMins.z;
            
			// Use the plane equation to grab the distance our start position is from the plane.
            //startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;
            startDistance = Dot(vStart + vOffset, pPlane->vNormal) - pPlane->d;

			// Get the distance our end position is from this current brush plane
            //endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;
            endDistance   = Dot(vEnd + vOffset, pPlane->vNormal) - pPlane->d;
        }

		// Make sure we start outside of the brush's volume
		if(startDistance > 0)	startsOut = true;

		// Stop checking since both the start and end position are in front of the plane
		if(startDistance > 0 && endDistance > 0)
			return;

		// Continue on to the next brush side if both points are behind or on the plane
		if(startDistance <= 0 && endDistance <= 0)
			continue;

		// If the distance of the start point is greater than the end point, we have a collision!
		if(startDistance > endDistance)
		{
			// This gets a ratio from our starting point to the approximate collision spot
			float Ratio1 = (startDistance - EPSILON) / (startDistance - endDistance);
			//float Ratio1 = startDistance / (startDistance - endDistance);

			// If this is the first time coming here, then this will always be true,
			if(Ratio1 > startRatio)
			{
				// Set the startRatio (currently the closest collision distance from start)
				startRatio = Ratio1;
			}
		}
		else
		{
			// Get the ratio of the current brush side for the endRatio
			float Ratio = (startDistance + EPSILON) / (startDistance - endDistance);

			// If the ratio is less than the current endRatio, assign a new endRatio.
			// This will usually always be true when starting out.
			if(Ratio < endRatio)
				endRatio = Ratio;
		}
	}
	
	m_bBroke = true;
	m_brokenBrush[brushIndex] = true;

	/*
	m_vCollisionNormal = vCollisionNormal;

	// This checks first tests if we actually moved along the x or z-axis,
	// meaning that we went in a direction somewhere.  The next check makes
	// sure that we don't always check to step every time we collide.  If
	// the normal of the plane has a Y value of 1, that means it's just the
	// flat ground and we don't need to check if we can step over it, it's flat!
	if((vStart.x != vEnd.x || vStart.z != vEnd.z) && pColPlane->vNormal.y != 1 && pColPlane->vNormal.y >= 0.0f)
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
		*/
	
	// If our startRatio is less than the endRatio there was a collision!!!
	if(startRatio < endRatio)
	{
		// Make sure the startRatio moved from the start and check if the collision
		// ratio we just got is less than the current ratio stored in m_traceRatio.
		// We want the closest collision to our original starting position.
		if(startRatio > -1 && startRatio < m_traceRatio)
		//if(startRatio > -1 && startRatio < relativeRatio)
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

void CQuake3BSP::RenderFace(int faceIndex)
{
	tBSPFace *pFace = &m_pFaces[faceIndex];

	glVertexPointer(3, GL_FLOAT, sizeof(tBSPVertex), &(m_pVerts[pFace->startVertIndex].vPosition));
	glEnableClientState(GL_VERTEX_ARRAY);

	// Set the current pass as the first texture (For multi-texturing)
	glActiveTextureARB(GL_TEXTURE0_ARB);

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
	//glDrawElements(GL_TRIANGLES, pFace->numOfIndices, GL_INT, &(m_pIndices[pFace->startIndex]) );
}

void CQuake3BSP::RenderSkyFace(int faceIndex)
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

void CQuake3BSP::RenderSky(CVector3 pos)
{
	int i = m_numOfFaces;

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glDisable(GL_DEPTH_TEST);

	while(i--)
	{
		tBSPFace* pFace = &m_pFaces[i];

		if(!m_sky[pFace->textureID]) continue;
		
		RenderSkyFace(i);
	}

	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void CQuake3BSP::RenderLevel(const CVector3 &vPos)
{
	glFrontFace(GL_CW);

	tBSPLeaf* pLeaf;
	int faceCount;
	int faceIndex;

	m_FacesDrawn.ClearAll();

	int leafIndex = FindLeaf(vPos);

	int cluster = m_pLeafs[leafIndex].cluster;
	int i = m_numOfLeafs;

	while(i--)
	{
		pLeaf = &m_pLeafs[i];

		if(!IsClusterVisible(cluster, pLeaf->cluster)) 
			continue;

		if(!g_frustum.BoxInFrustum((float)pLeaf->min.x, (float)pLeaf->min.y, (float)pLeaf->min.z,
		  	 				       (float)pLeaf->max.x, (float)pLeaf->max.y, (float)pLeaf->max.z))
			continue;
		
		faceCount = pLeaf->numOfLeafFaces;

		while(faceCount--)
		{
			faceIndex = m_pLeafFaces[pLeaf->leafface + faceCount];

			if(m_pFaces[faceIndex].type != FACE_POLYGON) continue;
			
			if(m_sky[m_pFaces[faceIndex].textureID]) continue;
			if(m_transparent[m_pFaces[faceIndex].textureID]) continue;
			if(m_brokenFace[faceIndex]) continue;

			if(m_FacesDrawn.On(faceIndex)) 
				continue;

			m_FacesDrawn.Set(faceIndex);

			if(m_numOfLightmaps <= 0)
				RenderSkyFace(faceIndex);
			else
				RenderFace(faceIndex);
		}			
	}
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glFrontFace(GL_CCW);
}

void CQuake3BSP::SortFaces(const CVector3 &vPos)
{
	tBSPLeaf* pLeaf;
	int faceCount;
	int faceIndex;

	int leafIndex = FindLeaf(vPos);

	int cluster = m_pLeafs[leafIndex].cluster;
	int i = m_numOfLeafs;

	while(i--)
	{
		pLeaf = &m_pLeafs[i];

		if(!IsClusterVisible(cluster, pLeaf->cluster)) 
			continue;

		if(!g_frustum.BoxInFrustum((float)pLeaf->min.x, (float)pLeaf->min.y, (float)pLeaf->min.z,
		  	 				       (float)pLeaf->max.x, (float)pLeaf->max.y, (float)pLeaf->max.z))
			continue;
		
		faceCount = pLeaf->numOfLeafFaces;

		while(faceCount--)
		{
			faceIndex = m_pLeafFaces[pLeaf->leafface + faceCount];

			if(m_pFaces[faceIndex].type != FACE_POLYGON) continue;
			
			if(!m_transparent[m_pFaces[faceIndex].textureID]) continue;

			//if(m_FacesDrawn.On(faceIndex)) 
			//	continue;

			m_FacesDrawn.Set(faceIndex);
		}			
	}
    
	for(int i=0; i<m_sortFaces.size(); i++)
	{
		faceIndex = m_sortFaces[i].faceIndex;

		if(!m_FacesDrawn.On(faceIndex))
			continue;
        
		m_sortFaces[i].dist = Magnitude2(m_sortFaces[i].pos - vPos);
	}

	CSortFace temp;
	int leftoff = 0;
	bool backtracking = false;
    
	for(int i=1; i<m_sortFaces.size(); i++)
	{   
		faceIndex = m_sortFaces[i].faceIndex;

		//if(!m_FacesDrawn.On(faceIndex))
		//	continue;

		if(i > 0)
		{
			if(m_sortFaces[i].dist > m_sortFaces[i-1].dist)
			{
				if(!backtracking)
				{
					leftoff = i;
					backtracking = true;
				}
				temp = m_sortFaces[i];
				m_sortFaces[i] = m_sortFaces[i-1];
				m_sortFaces[i-1] = temp;
				i-=2;
			}
			else
			{
				if(backtracking)
				{
					backtracking = false;
					i = leftoff;
				}
			}
		}
		else
			backtracking = false;
	}
}

void CQuake3BSP::RenderLevel2(const CVector3 &vPos)
{
	glFrontFace(GL_CW);
	int faceIndex;
	
	//for(int i=m_sortFaces.size()-1; i>=0; i--)
	for(int i=0; i<m_sortFaces.size(); i++)
	{
		faceIndex = m_sortFaces[i].faceIndex;

		if(m_brokenFace[faceIndex]) continue;

		if(!m_FacesDrawn.On(faceIndex))
			continue;
		
		if(m_numOfLightmaps <= 0)
			RenderSkyFace(faceIndex);
		else
			RenderFace(faceIndex);
	}
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glFrontFace(GL_CCW);
}

void CQuake3BSP::Destroy(bool delTex)
{
	if(m_pVerts) 
	{
		delete [] m_pVerts;		m_pVerts = NULL;
	}

	if(m_pFaces)	
	{
		delete [] m_pFaces;		m_pFaces = NULL;
	}

	m_sortFaces.clear();

	if(m_brokenFace)
	{
		delete [] m_brokenFace;	m_brokenFace = NULL;
	}

	if(m_brokenBrush)
	{
		delete [] m_brokenBrush;	m_brokenBrush = NULL;
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
	
	if(delTex)
	{
		//glDeleteTextures(m_numOfTextures, m_textures);

		for(int i=0; i<m_numOfTextures; i++)
		{
			FreeTexture(m_pTextures[i].strName);
		}
	}

	if(m_pTextures)	
	{
		delete [] m_pTextures;		m_pTextures = NULL;
	}	

	if(m_pLightmaps)
	{
		delete [] m_pLightmaps;		m_pLightmaps = NULL;
	}

	if(m_pModels)
	{
		delete [] m_pModels;		m_pModels = NULL;
	}

	if(m_pLightVols)
	{
		delete [] m_pLightVols;		m_pLightVols = NULL;
	}

	m_sortFaces.clear();
	
	glDeleteTextures(m_numOfLightmaps, m_lightmaps);

	if(m_textures)
	{
		delete [] m_textures;		m_textures = NULL;
	}
	
	if(m_lightmaps)
	{
		delete [] m_lightmaps;		m_lightmaps = NULL;
	}
	
	if(m_passable)
	{
		delete [] m_passable;		m_passable = NULL;
	}
	
	if(m_sky)
	{
		delete [] m_sky;		m_sky = NULL;
	}
	
	if(m_transparent)
	{
		delete [] m_transparent;		m_transparent = NULL;
	}
	
	if(m_water)
	{
		delete [] m_water;		m_water = NULL;
	}
	
	if(m_breakable)
	{
		delete [] m_breakable;		m_breakable = NULL;
	}
	
	if(m_ladder)
	{
		delete [] m_ladder;		m_ladder = NULL;
	}
	
	if(m_grate)
	{
		delete [] m_grate;		m_grate = NULL;
	}
}

CQuake3BSP::~CQuake3BSP()
{
	Destroy();
}


