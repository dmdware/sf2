

#ifndef MODEL2_H
#define MODEL2_H

#include "../platform.h"
#include "ms3d.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "vertexarray.h"

//struct VertexArray;
struct MS3DModel;
struct Shader;

struct Material
{
public:
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;
	unsigned int ownerm;
	std::string file;
};

struct Model2
{
public:
	ecbool on;
#if 0
	MS3DModel ms3d;
	VertexArray* va;
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;
	unsigned int ownerm;
#endif

#if 1
	std::string fullpath;
	std::string relative;
#endif

#define NUM_BONES_PER_VERTEX 4

    struct BoneInfo
    {
        Matrix BoneOffset;
        Matrix FinalTransformation;

        BoneInfo()
        {
            BoneOffset.reset();
            FinalTransformation.reset();
        }
    };

    struct VertexBoneData
    {
        uint IDs[NUM_BONES_PER_VERTEX];
        float Weights[NUM_BONES_PER_VERTEX];

        VertexBoneData()
        {
            Reset();
        };

        void Reset()
        {
            ZERO_MEM(IDs);
            ZERO_MEM(Weights);
        }

        void AddBoneData(uint BoneID, float Weight);
    };

#define INVALID_MATERIAL 0xFFFFFFFF

enum VB_TYPES {
    INDEX_BUFFER,
    POS_VB,
    NORMAL_VB,
    TEXCOORD_VB,
    BONE_VB,
    NUM_VBs
};

    GLuint VAO;
    GLuint Buffers[NUM_VBs];

    struct MeshEntry
    {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

		unsigned int NumUniqueVerts;
        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> Entries;
    std::vector<Material> Textures;

    std::map<std::string,uint> BoneMapping; // maps a bone name to its index
    uint NumBones;
    std::vector<BoneInfo> BoneInfo;
    Matrix GlobalInverseTransform;

	const aiScene* pScene;
    Assimp::Importer Importer;

	std::vector<Vec3f> Positions;
	std::vector<Vec3f> Normals;
	std::vector<Vec2f> TexCoords;
	std::vector<VertexBoneData> Bones;
	std::vector<uint> Indices;

	Model2();
	~Model2();
	Model2(const Model2& original);
	Model2& operator=(const Model2 &original);

    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const Matrix& ParentTransform);
    ecbool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(uint MeshIndex,
                  const aiMesh* paiMesh,
                  std::vector<Vec3f>& Positions,
                  std::vector<Vec3f>& Normals,
                  std::vector<Vec2f>& TexCoords,
                  std::vector<VertexBoneData>& Bones,
                  std::vector<unsigned int>& Indices,
					  const aiScene* paiScene);
	void LoadMeshMat(const aiMesh* paiMesh,
					  const aiScene* pScene,
					  uint MeshIndex);
    void LoadBones(uint MeshIndex, const aiMesh* paiMesh, std::vector<VertexBoneData>& Bones);
    ecbool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();
    void Render(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat);
    void RenderDepth(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat);
    uint NumBones() const
    {
        return NumBones;
    }
    void BoneTransform(float TimeInSeconds, std::vector<Matrix>& Transforms);

	ecbool load(const char* relative, Vec3f scale, Vec3f translate, ecbool dontqueue);
#if 0
	void usedifftex();
	void usespectex();
	void usenormtex();
	void useteamtex();
#endif
	void draw(int frame, Vec3f pos, float yaw);
	void drawdepth(int frame, Vec3f pos, float yaw);
	void destroy()
	{
		Clear();
		on = ecfalse;
	}
};

#define MODELS2	512
extern Model2 g_model2[MODELS2];

int NewModel2();
int FindModel2(const char* relative);
void QueueModel2(int* id, const char* relative, Vec3f scale, Vec3f translate);
int LoadModel2(const char* relative, Vec3f scale, Vec3f translate, ecbool dontqueue);
void FreeModels2();

#endif
