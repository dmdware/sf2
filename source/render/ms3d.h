
#ifndef MS3D_H
#define MS3D_H

#pragma warning( disable : 4160 )

#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/matrix.h"

// byte-align structures
#pragma pack(push, 1)

typedef unsigned char byte;
typedef unsigned short word;

// File header
struct MS3DHeader
{
	char ID[10];
	int version;
};

// Vertex information
struct MS3DVertex
{
	byte flags;
	float vertex[3];
	char boneID;
	byte refCount;
};

// Triangle information
struct MS3DTriangle
{
	word flags;
	word vertexIndices[3];
	float vertexNormals[3][3];
	float s[3], t[3];
	byte smoothingGroup;
	byte groupIndex;
};

// Material information
struct MS3DMaterial
{
    char name[32];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emissive[4];
    float shininess;	// 0.0f - 128.0f
    float transparency;	// 0.0f - 1.0f
    byte mode;	// 0, 1, 2 is unused now
    char diffusem[128];
    char alphabldg[128];
};

//	Joint information
struct MS3DJoint
{
	byte flags;
	char name[32];
	char parentName[32];
	float rotation[3];
	float translation[3];
	word numRotationKeyframes;
	word numTranslationKeyframes;
};

// Keyframe data
struct MS3DKeyframe
{
	float time;
	float parameter[3];
};

// Default alignment
#pragma pack(pop)

struct VertexArray;

struct MS3DModel
{
	public:
		char relative[SFH_MAX_PATH+1];
		int frame;

		//	Mesh
		struct Mesh
		{
			int materialIndex;
			int numTriangles;
			int *pTriangleIndices;
		};

		//	Material properties
		struct Material
		{
			float ambient[4], diffuse[4], specular[4], emissive[4];
			float shininess;
			unsigned int diffusem;
			char *pTextureFilename;
		};

		//	Triangle structure
		struct Triangle
		{
			float vertexNormals[3][3];
			float s[3], t[3];
			int vertexIndices[3];
		};

		//	Vertex structure
		struct Vertex
		{
			char boneID;	// for skeletal animation
			float location[3];
		};

		//	Animation keyframe information
		struct Keyframe
		{
			int jointIndex;
			float time;	// in milliseconds
			float parameter[3];
		};

		//	Skeleton bone joint
		struct Joint
		{
			float localRotation[3];
			float localTranslation[3];
			Matrix absolute, relative;

			int numRotationKeyframes, numTranslationKeyframes;
			Keyframe *pTranslationKeyframes;
			Keyframe *pRotationKeyframes;

			int currentTranslationKeyframe, currentRotationKeyframe;
			Matrix final;

			int parent;
		};

	public:
		MS3DModel();
		~MS3DModel();

		ecbool load(const char *relative, unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, ecbool dontqueue);
		void destroy();

		void loadtex(unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, ecbool dontqueue);
		void genva(VertexArray** vertexArrays, Vec3f scale, Vec3f translate, const char* filepath);

	//protected:
		/*
			Set the values of a particular keyframe for a particular joint.
				jointIndex		The joint to setup the keyframe for
				keyframeIndex	The maximum number of keyframes
				time			The time in milliseconds of the keyframe
				parameter		The rotation/translation values for the keyframe
				isRotation		Whether it is a rotation or a translation keyframe
		*/
		void setjointkf( int jointIndex, int keyframeIndex, float time, float *parameter, ecbool isRotation );

		//	Setup joint matrices
		void setupjoints();

		//	Advance animation by a frame
		void advanceanim();

		//	Restart animation
		void restart();

		//	Meshes used
		int numMeshes;
		Mesh *pMeshes;

		//	Materials used
		int numMaterials;
		Material *pMaterials;

		//	Triangles used
		int numTriangles;
		Triangle *pTriangles;

		//	Vertices Used
		int numVertices;
		Vertex *pVertices;

		int numJoints;
		Joint *pJoints;

		//	Total animation time
		double totalTime;
		int totalFrames;
};

#endif
