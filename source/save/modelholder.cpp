

#include "modelholder.h"
#include "../render/vertexarray.h"
#include "../math/brush.h"
#include "../math/quaternion.h"
#include "../math/vec4f.h"
#include "compilemap.h"
#include "../platform.h"
#include "../utils.h"
#include "../debug.h"

std::list<ModelHolder> g_modelholder;

ModelHolder::ModelHolder()
{
	//nframes = 0;
	//frames = NULL;
	modeli = -1;
	//model.fullpath = "";
}

void VAsMinMax(VertexArray** frames, int nframes, Vec3f* pvmin, Vec3f* pvmax)
{
	Vec3f vmin(0,0,0);
	Vec3f vmax(0,0,0);

	for(int i=0; i<nframes; i++)
	{
		VertexArray* frame = &(*frames)[i];

		for(int vertidx = 0; vertidx < frame->numverts; vertidx++)
		{
			Vec3f v = frame->vertices[vertidx];

			if(v.x < vmin.x)
				vmin.x = v.x;
			if(v.y < vmin.y)
				vmin.y = v.y;
			if(v.z < vmin.z)
				vmin.z = v.z;
			if(v.x > vmax.x)
				vmax.x = v.x;
			if(v.y > vmax.y)
				vmax.y = v.y;
			if(v.z > vmax.z)
				vmax.z = v.z;
		}
	}

	*pvmin = vmin;
	*pvmax = vmax;
}

ModelHolder::ModelHolder(int model, Vec3f pos)
{
	//nframes = 0;
	//frames = NULL;

	this->modeli = model;
	this->model.fullpath = g_model2[this->modeli].fullpath;
	translation = pos;
	rotdegrees = Vec3f(0,0,0);
	scale = Vec3f(1,1,1);

	retransform();
}

ModelHolder::~ModelHolder()
{
	destroy();
}


ModelHolder::ModelHolder(const ModelHolder& original)
{
	//nframes = 0;
	//frames = NULL;
	*this = original;
}

ModelHolder& ModelHolder::operator=(const ModelHolder &original)
{
#if 0
	int model;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix transform;
	VertexArray* frames;
	int nframes;
#endif

	destroy();
	//model = original.model;
	rotdegrees = original.rotdegrees;
	translation = original.translation;
	scale = original.scale;
	absmin = original.absmin;
	absmax = original.absmax;
	rotationmat = original.rotationmat;
	modeli = original.modeli;
	model.pScene = original.model.pScene;
	model.fullpath = original.model.fullpath;
	retransform();
	//CopyVAs(&frames, &nframes, &original.frames, original.nframes);
	//model = original.model;

	return *this;
}

void ModelHolder::retransform()
{
	destroy();

	Model2* m = &g_model2[modeli];

	model.Clear();
	model.GlobalInverseTransform = m->GlobalInverseTransform;
	model.pScene = m->pScene;
	model.fullpath = m->fullpath;
	model.Textures = m->Textures;
	model.Positions = m->Positions;
	model.TexCoords = m->TexCoords;
	model.Normals = m->Normals;
	model.Indices = m->Indices;
	model.BoneInfo = m->BoneInfo;
	model.BoneMapping = m->BoneMapping;
	model.Entries = m->Entries;
	model.Bones = m->Bones;
	model.NumBones = m->NumBones;
	//model.InitFromScene(m->pScene, m->fullpath);

#if 0
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
#endif

	m = &model;

	//Quaternion rotquat;
	Vec3f rotrads;
	rotrads.x = DEGTORAD(rotdegrees.x);
	rotrads.y = DEGTORAD(rotdegrees.y);
	rotrads.z = DEGTORAD(rotdegrees.z);
	//rotquat.fromAngles((float*)&rotrads);
	rotationmat.reset();
	rotationmat.rotrad((float*)&rotrads);

#if 0	//TODO
	for(int frameidx = 0; frameidx < nframes; frameidx++)
	{
		VertexArray* pframe = &frames[frameidx];
		Vec3f normal;

		for(int vertidx = 0; vertidx < pframe->numverts; vertidx++)
		{
			pframe->vertices[vertidx].transform(rotationmat);
			pframe->vertices[vertidx] = pframe->vertices[vertidx] * scale;
		}
	}

	regennormals();

	VAsMinMax(&frames, nframes, &absmin, &absmax);
	absmin = absmin + translation;
	absmax = absmax + translation;
#else

	absmin = Vec3f(0,0,0);
	absmax = Vec3f(0,0,0);

	//gets abs min max and apply transform

	ecbool minset[3] = {ecfalse,ecfalse,ecfalse};
	ecbool maxset[3] = {ecfalse,ecfalse,ecfalse};

	for(int i=0; i<model.Positions.size(); i++)
	{
		model.Positions[i].transform(rotationmat);
		model.Positions[i] = model.Positions[i] * scale;

		model.Normals[i].transform(rotationmat);	//hopefully works correctly

		if(!minset[0] || absmin.x > model.Positions[i].x)
		{
			minset[0] = ectrue;
			absmin.x = model.Positions[i].x;
		}

		if(!minset[1] || absmin.y > model.Positions[i].y)
		{
			minset[1] = ectrue;
			absmin.y = model.Positions[i].y;
		}

		if(!minset[2] || absmin.z > model.Positions[i].z)
		{
			minset[2] = ectrue;
			absmin.z = model.Positions[i].z;
		}

		if(!maxset[0] || absmax.x < model.Positions[i].x)
		{
			maxset[0] = ectrue;
			absmax.x = model.Positions[i].x;
		}

		if(!maxset[1] || absmax.y < model.Positions[i].y)
		{
			maxset[1] = ectrue;
			absmax.y = model.Positions[i].y;
		}

		if(!maxset[2] || absmax.z < model.Positions[i].z)
		{
			maxset[2] = ectrue;
			absmax.z = model.Positions[i].z;
		}
	}

	absmin = absmin + translation;
	absmax = absmax + translation;

	//regennormals();
#endif
}

void ModelHolder::regennormals()
{
	//TODO
#if 0
	Model* m = &g_model[model];
	MS3DModel* ms3d = &m->ms3d;

	std::vector<Vec3f>* normalweights;

	normalweights = new std::vector<Vec3f>[ms3d->numVertices];

	for(int f = 0; f < nframes; f++)
	{
		for(int index = 0; index < ms3d->numVertices; index++)
		{
			normalweights[index].clear();
		}

		Vec3f* vertices = frames[f].vertices;
		//Vec2f* texcoords = frames[f].texcoords;
		Vec3f* normals = frames[f].normals;

		int vert = 0;

		for(int i = 0; i < ms3d->numMeshes; i++)
		{
			for(int j = 0; j < ms3d->pMeshes[i].numTriangles; j++)
			{
				int triangleIndex = ms3d->pMeshes[i].pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->pTriangles[triangleIndex];

				Vec3f normal;
				Vec3f tri[3];
				tri[0] = vertices[vert+0];
				tri[1] = vertices[vert+1];
				tri[2] = vertices[vert+2];
				//normal = Normal2(tri);
				normal = Normal(tri);	//Reverse order
				//normals[i] = normal;
				//normals[i+1] = normal;
				//normals[i+2] = normal;

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->vertexIndices[k];
					normalweights[index].push_back(normal);

					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}

		vert = 0;

		for(int i = 0; i < ms3d->numMeshes; i++)
		{
			for(int j = 0; j < ms3d->pMeshes[i].numTriangles; j++)
			{
				int triangleIndex = ms3d->pMeshes[i].pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->pTriangles[triangleIndex];

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->vertexIndices[k];

					Vec3f weighsum(0, 0, 0);

					for(int l=0; l<normalweights[index].size(); l++)
					{
						weighsum = weighsum + normalweights[index][l] / (float)normalweights[index].size();
					}

					normals[vert] = weighsum;

					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}
	}

	delete [] normalweights;
#endif
}

void ModelHolder::destroy()
{
#if 0
	nframes = 0;

	if(frames)
	{
		delete [] frames;
		frames = NULL;
	}
#endif

	model.destroy();
}

Vec3f ModelHolder::traceray(Vec3f line[])
{
	Vec3f planenorms[6];
	float planedists[6];
	MakeHull(planenorms, planedists, Vec3f(0,0,0), absmin, absmax);

#if 0
	for(int i=0; i<6; i++)
	{
		Log("mh pl ("<<planenorms[i].x<<","<<planenorms[i].y<<","<<planenorms[i].z<<"),"<<planedists[i]<<std::endl;
	}
#endif

	if(LineInterHull(line, planenorms, planedists, 6))
	{
		return (absmin+absmax)/2.0f;
	}

	return line[1];
}

void FreeModelHolders()
{
	g_modelholder.clear();
}

void DrawModelHolders()
{
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model2* m = &g_model2[h->modeli];

#if 0 //TODO
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usedifftex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usespectex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->usenormtex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		m->useteamtex();
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//DrawVA(&m->va[rand()%10], h->translation);
		DrawVA(&h->frames[ g_renderframe % m->ms3d.totalFrames ], h->translation);
#ifdef DEBUG
		CHECKGLERROR();
#endif
#endif
		int frames = 1;

		if(m->pScene->mNumAnimations > 0)
		{
			double duration = m->pScene->mAnimations[0]->mDuration;
			double tickspersec = (float)(m->pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			double dframes = duration / tickspersec;
			if(dframes >= 1.0f)
				frames = (int)dframes;
		}
		
		//h->model.Render(g_renderframe % frames, h->translation, h->modeli, h->scale, h->rotationmat);
		h->model.Render(g_renderframe, h->translation, h->modeli, h->scale, h->rotationmat);
	}
}

void DrawModelHoldersDepth()
{
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model2* m = &g_model2[h->modeli];

#if 0	//TODO
		m->usedifftex();
		//DrawVA(&m->va[rand()%10], h->translation);
		DrawVADepth(&h->frames[ g_renderframe % m->ms3d.totalFrames ], h->translation);
#else

		int frames = 1;

		if(m->pScene->mNumAnimations > 0)
		{
			double duration = m->pScene->mAnimations[0]->mDuration;
			double tickspersec = (float)(m->pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			double dframes = duration / tickspersec;
			if(dframes >= 1.0f)
				frames = (int)dframes;
		}
		
		//h->model.RenderDepth(g_renderframe % frames, h->translation, h->modeli, h->scale, h->rotationmat);
		h->model.RenderDepth(g_renderframe, h->translation, h->modeli, h->scale, h->rotationmat);
#endif
	}
}
