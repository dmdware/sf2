


#ifndef QUAKE3BSP_H
#define QUAKE3BSP_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/frustum.h"

#define BSP_TEXTURES	1024

#define FACE_POLYGON	1

#define TYPE_RAY		0
#define TYPE_SPHERE		1
#define TYPE_BOX		2

#define EPSILON			0.03125f

struct tVector3i
{
	int x, y, z;
};

struct tBSPHeader
{
    char strID[4];	// This should always be 'IBSP'
    int version;	// This should be 0x2e for Quake 3 files
}; 

struct tBSPLump
{
	int offset;
	int length;
};

struct tBSPVertex
{
    Vec3f vPosition;	
    Vec2f vTextureCoord;
    Vec2f vLightmapCoord;
    Vec3f vNormal;
    byte color[4];
};

struct tBSPFace
{
    int textureID;				// The index into the texture array 
    int effect;					// The index for the effects (or -1 = n/a) 
    int type;					// 1=polygon, 2=patch, 3=mesh, 4=billboard 
    int startVertIndex;			// The starting index into this face's first vertex 
    int numOfVerts;				// The number of vertices for this face 
    int startIndex;				// The starting index into the indices array for this face
    int numOfIndices;			// The number of indices for this face
    int lightmapID;				// The texture index for the lightmap 
    int lMapCorner[2];			// The face's lightmap corner in the image 
    int lMapSize[2];			// The size of the lightmap section 
    Vec3f lMapPos;			// The 3D origin of lightmap. 
    Vec3f lMapVecs[2];		// The 3D space for s and t unit vectors. 
    Vec3f vNormal;			// The face normal. 
    int size[2];				// The bezier patch dimensions. 
};

struct tBSPTexture
{
    char strName[64];			// The name of the texture w/o the extension 
    int flags;					// The surface flags (unknown) 
    int textureType;			// The type of texture (solid, water, slime, etc..) (type & 1) = 1 (solid)
};

struct tBSPLightmap
{
    byte imageBits[128][128][3];   // The RGB data in a 128x128 image
};

struct tBSPNode
{
    int plane;					// The index into the planes array 
    int front;					// The child index for the front node 
    int back;					// The child index for the back node 
    tVector3i min;				// The bounding box min position. 
    tVector3i max;				// The bounding box max position. 
}; 

// This stores a leaf (end node) in the BSP tree
struct tBSPLeaf
{
    int cluster;				// The visibility cluster 
    int area;					// The area portal 
    tVector3i min;				// The bounding box min position 
    tVector3i max;				// The bounding box max position 
    int leafface;				// The first index into the face array 
    int numOfLeafFaces;			// The number of faces for this leaf 
    int leafBrush;				// The first index for into the brushes 
    int numOfLeafBrushes;		// The number of brushes for this leaf 
}; 

// This stores a splitter plane in the BSP tree
struct tBSPPlane
{
    Vec3f vNormal;			// Plane3f normal. 
    float d;					// The plane distance from origin 
};

// This stores the cluster data for the PVS's
struct tBSPVisData
{
	int numOfClusters;			// The number of clusters
	int bytesPerCluster;		// The amount of bytes (8 bits) in the cluster's bitset
	byte *pBitsets;				// The array of bytes that holds the cluster bitsets				
};

// This stores the brush data
struct tBSPBrush 
{
	int brushSide;				// The starting brush side for the brush 
	int numOfBrushSides;		// Number of brush sides for the brush
	int textureID;				// The texture index for the brush
}; 

// This stores the brush side data, which stores indices for the normal and texture ID
struct tBSPBrushSide 
{
	int plane;					// The plane index
	int textureID;				// The texture index
}; 

// This is our lumps enumeration
enum eLumps
{
    kEntities = 0,				// Stores player/object positions, etc...
    kTextures,					// Stores texture information
    kPlanes,				    // Stores the splitting planes
    kNodes,						// Stores the BSP nodes
    kLeafs,						// Stores the leafs of the nodes
    kLeafFaces,					// Stores the leaf's indices into the faces
    kLeafBrushes,				// Stores the leaf's indices into the brushes
    kModels,					// Stores the info of world models
    kBrushes,					// Stores the brushes info (for collision)
    kBrushSides,				// Stores the brush surfaces info
    kVertices,					// Stores the level vertices
    kIndices,					// Stores the level indices
    kShaders,					// Stores the shader files (blending, anims..)
    kFaces,						// Stores the faces for the level
    kLightmaps,					// Stores the lightmaps for the level
    kLightVolumes,				// Stores extra world lighting information
    kVisData,					// Stores PVS and cluster info (visibility)
    kMaxLumps					// A constant to store the number of lumps
};

class CBitset 
{

public:
    CBitset() : m_bits(0), m_size(0) {}

	~CBitset() 
	{
		if(m_bits) 
		{
			delete m_bits;
			m_bits = NULL;
		}
	}

	void Resize(int count) 
	{ 
		m_size = count/32 + 1;

        if(m_bits) 
		{
			delete m_bits;
			m_bits = 0;
		}

		m_bits = new unsigned int[m_size];
		ClearAll();
	}

	void Set(int i) 
	{
		m_bits[i >> 5] |= (1 << (i & 31));
	}

	int On(int i) 
	{
		return m_bits[i >> 5] & (1 << (i & 31 ));
	}

	void Clear(int i) 
	{
		m_bits[i >> 5] &= ~(1 << (i & 31));
	}

	void ClearAll() 
	{
		memset(m_bits, 0, sizeof(unsigned int) * m_size);
	}

private:

	unsigned int *m_bits;
	int m_size;
};

class Q3BSP
{

public:
	Q3BSP();
	~Q3BSP();

	bool LoadBSP(const char *strFileName);
	void ReloadTextures();

	void RenderLevel(const Vec3f &vPos);
	void RenderSky();

	Vec3f traceray(Vec3f vStart, Vec3f vEnd);
	Vec3f TraceSphere(Vec3f vStart, Vec3f vEnd, float radius, float maxStep);
	Vec3f TraceBox(Vec3f vStart, Vec3f vEnd, Vec3f vMin, Vec3f vMax, float maxStep);

	bool IsOnGround()	{ return m_bGrounded;	}
	bool Collided()		{ return m_bCollided;	}
	bool Stuck()		{ return m_bStuck;		}
	bool Ceiling()		{ return m_bCeiling;	}

	void Destroy();

	int IsClusterVisible(int current, int test);

private:
	void ChangeGamma(byte *pImage, int size, float factor);
	void CreateLightmapTexture(UINT &texture, byte *pImageBits, int width, int height);
	int FindLeaf(const Vec3f &vPos);

	Vec3f TryToStep(Vec3f vStart, Vec3f vEnd, float maxStep);

	Vec3f Trace(Vec3f vStart, Vec3f vEnd);

	void CheckNode(int nodeIndex, Vec3f vStart, Vec3f vEnd);
	void CheckBrush(tBSPBrush *pBrush, Vec3f vStart, Vec3f vEnd);
	
	Vec3f UnstuckNode(int nodeIndex, Vec3f vStart, Vec3f vEnd);
	Vec3f UnstuckBrush(tBSPBrush *pBrush, Vec3f vStart);

	void FindTextureExtension(char *strFileName);

	void RenderFace(int faceIndex);
	void RenderSkyFace(int faceIndex);

	unsigned int m_posvbo;
	unsigned int m_indvbo;
	unsigned int m_vao;

	int m_numOfVerts;
	int m_numOfFaces;
	int m_numOfIndices;
	int m_numOfTextures;
	int m_numOfLightmaps;
	int m_numOfNodes;
	int m_numOfLeafs;
	int m_numOfLeafFaces;
	int m_numOfPlanes;
	int m_numOfBrushes;
	int m_numOfBrushSides;
	int m_numOfLeafBrushes;
	
	int m_traceType;
	float m_traceRatio;
	float m_traceRadius;
	bool m_bStuck;
	Vec3f m_vStart;
	Vec3f m_vEnd;

	bool m_bCollided;
	bool m_bGrounded;
	bool m_bCeiling;
	bool m_bTryStep;
	bool m_bLadder;

	Vec3f m_vTraceMins;
	Vec3f m_vTraceMaxs;
	Vec3f m_vExtents;
	Vec3f m_vCollisionNormal;

	int* m_pIndices;
	tBSPVertex* m_pVerts;
	tBSPFace* m_pFaces;
	tBSPNode* m_pNodes;
	tBSPLeaf* m_pLeafs;
	tBSPPlane* m_pPlanes;
	int* m_pLeafFaces;
	tBSPVisData m_clusters;
	tBSPTexture* m_pTextures;
	tBSPLightmap *m_pLightmaps;
	tBSPBrush* m_pBrushes;
	tBSPBrushSide* m_pBrushSides;
	int* m_pLeafBrushes;
								
	UINT m_textures[BSP_TEXTURES];
	UINT m_lightmaps[BSP_TEXTURES];
	bool m_passable[BSP_TEXTURES];
	bool m_sky[BSP_TEXTURES];
								
	CBitset m_FacesDrawn;
};

void DrawBackdrop();

//extern Q3BSP g_map;
extern Frustum g_frustum;
extern Q3BSP g_bsp;

#endif
