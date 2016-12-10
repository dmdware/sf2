#include "../platform.h"
#include "../math/3dmath.h"
#include "model2.h"
#include "../texture.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"
#include "shadow.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "../sim/simtile.h"
#include "../tool/rendersprite.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../sim/simtile.h"

Model2 g_model2[MODELS2];

int NewModel2()
{
	for(int i=0; i<MODELS2; i++)
		if(!g_model2[i].on)
			return i;

	return -1;
}

int FindModel2(const char* relative)
{
	char full[SFH_MAX_PATH+1];
	FullPath(relative, full);
	char corrected[SFH_MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);

	for(int i=0; i<MODELS2; i++)
	{
		Model2* m = &g_model2[i];

		if(!m->on)
			continue;

		if(stricmp(m->fullpath.c_str(), corrected) == 0)
			return i;
	}

	return -1;
}

int LoadModel2(const char* relative, Vec3f scale, Vec3f translate, ecbool dontqueue)
{
	int i = FindModel2(relative);

	if(i >= 0)
		return i;

	i = NewModel2();

	if(i < 0)
		return i;

	if(g_model2[i].load(relative, scale, translate, dontqueue))
		return i;

	return -1;
}

#define GLCheckError() (glGetError() == GL_NO_ERROR)

void Model2::VertexBoneData::AddBoneData(uint BoneID, float Weight)
{
	for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(IDs) ; i++) {
		if (Weights[i] == 0.0) {
			IDs[i]     = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

Model2::Model2()
{
	on = ecfalse;
	VAO = 0;
	ZERO_MEM(Buffers);
	NumBones = 0;
	pScene = NULL;
}

Model2::~Model2()
{
	Clear();
}


void Model2::Clear()
{
	on = ecfalse;
	fullpath = "";
	relative = "";

	Positions.clear();
	Normals.clear();
	TexCoords.clear();
	Bones.clear();
	Indices.clear();

	//for (uint i = 0 ; i < Textures.size() ; i++) {
	//	SAFE_DELETE(Textures[i]);
	//}
#if 0
	for(int i=0; i<Textures.size(); ++i)
	{
		if(Textures[i].diffusem)
			FreeTexture(Textures[i].diffusem);
		if(Textures[i].normalm)
			FreeTexture(Textures[i].normalm);
		if(Textures[i].ownerm)
			FreeTexture(Textures[i].ownerm);
		if(Textures[i].specularm)
			FreeTexture(Textures[i].specularm);
	}
#endif
	Textures.clear();

	if (Buffers[0] != 0) {
		glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(Buffers), Buffers);
	}

	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	//pScene->Clear();
	//((aiScene*)(pScene))->~aiScene();
	//pScene = NULL;
	//Importer.FreeScene();
}

ecbool Model2::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
	//Clear();

	Entries.resize(pScene->mNumMeshes);
	Textures.resize(pScene->mNumMaterials);

	Log("Num embedded textures: %d", (int)pScene->mNumTextures);

	uint NumVertices = 0;
	uint NumIndices = 0;

	// Count the number of vertices and indices
	for (uint i = 0 ; i < Entries.size() ; i++) {
		Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
		Entries[i].BaseVertex    = NumVertices;
		Entries[i].BaseIndex     = NumIndices;
		Entries[i].NumUniqueVerts = pScene->mMeshes[i]->mNumVertices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices  += Entries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Bones.resize(NumVertices);
	//Bones.reserve(NumVertices);
	Indices.reserve(NumIndices);

	// Initialize the meshes in the scene one by one
	for (uint i = 0 ; i < Entries.size() ; i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices, pScene);
	}

#if 0
	if (!InitMaterials(pScene, Filename)) {
		return ecfalse;
	}
#endif

#if 0
	// Generate and populate the buffers with vertex attributes and the indices
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
#endif

	return ectrue;
	//return GLCheckError();
}

ecbool Model2::load(const char* relative, Vec3f scale, Vec3f translate, ecbool dontqueue)
{

	// Release the previously loaded mesh (if it exists)
	Clear();

	ecbool result = ecfalse;

	char full[SFH_MAX_PATH+1];
	FullPath(relative, full);
	char corrected[SFH_MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);
	fullpath = corrected;
	relative = relative;
	//scene = aiImportFile(corrected, aiProcessPreset_TargetRealtime_MaxQuality);

	//std::string rel = MakeRelative(fullpath.c_str());

	//InfoMess("fullpath", fullpath.c_str());
	//InfoMess("rel", rel.c_str());

	// Create the VAO
	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	// Create the buffers for the vertices attributes
	//glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(Buffers), Buffers);

	ecbool Ret = ecfalse;

	//Assimp::Importer imp;

#if 1
	pScene = Importer.ReadFile(corrected,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs );
#endif

#if 0
	aiPropertyStore* props = aiCreatePropertyStore();

	pScene = (aiScene*)aiImportFileExWithProperties(corrected,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs,
		NULL,
		props);

	aiReleasePropertyStore(props);
#endif

	if (pScene) {
		GlobalInverseTransform = pScene->mRootNode->mTransformation;
		GlobalInverseTransform.inverse();
		std::string dir = StripFile(fullpath);
		//InfoMess("dir", dir.c_str());
		Ret = InitFromScene(pScene, corrected);
		on = ectrue;
	}
	else {
		char msg[1280];
		sprintf(msg, "Error parsing '%s': '%s'\n", corrected, Importer.GetErrorString());
		ErrMess("ASSIMP Error", msg);
		glBindVertexArray(0);
		return ecfalse;
	}


#if 0
	Log("properties: ");

	for(int mi=0; mi<pScene->mNumMaterials; mi++)
		for(int pi=0; pi<pScene->mMaterials[mi]->mNumProperties; pi++)
		{
			//pScene->mMaterials[mi]->mProperties[pi]->
			pScene->mMaterials[mi]->mProperties[pi]->mData[pScene->mMaterials[mi]->mProperties[pi]->mDataLength-1]=0;

			Log("\t prop"<<pi<<" (l"<<pScene->mMaterials[mi]->mProperties[pi]->mDataLength<<"): "<<
				pScene->mMaterials[mi]->mProperties[pi]->mKey.data<<" = "<<
				"\""<<pScene->mMaterials[mi]->mProperties[pi]->mData<<"\"");
		}
#endif
		

		// Make sure the VAO is not changed from the outside
		glBindVertexArray(0);

		on = Ret;

		return Ret;

#if 0
		// Extract the directory part from the file name
		std::string::size_type slashindex = filename.find_last_of("/");
		std::string dir;

		if (SlashIndex == std::string::npos)
		{
			dir = ".";
		}
		else if (SlashIndex == 0)
		{
			dir = "/";
		}
		else
		{
			dir = filename.substr(0, slashindex);
		}
#endif

#if 0
		if(result)
			//if(scene)
		{
			on = ectrue;
			ms3d.genva(&va, scale, translate, relative);
			char full[SFH_MAX_PATH+1];
			FullPath(relative, full);
			char corrected[SFH_MAX_PATH+1];
			strcpy(corrected, full);
			CorrectSlashes(corrected);
			fullpath = corrected;
		}

		/*
		if(result)
		{
		//CreateTex(spectex, specfile);
		//QueueTex(&spectex, specfile, ectrue);
		CorrectNormals();
		}*/

		return result;
#endif
}



void Model2::InitMesh(uint MeshIndex,
					  const aiMesh* paiMesh,
					  std::vector<Vec3f>& Positions,
					  std::vector<Vec3f>& Normals,
					  std::vector<Vec2f>& TexCoords,
					  std::vector<VertexBoneData>& Bones,
					  std::vector<uint>& Indices,
					  const aiScene* paiScene)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
		const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(Vec3f(pPos->x, pPos->y, pPos->z));
		Normals.push_back(Vec3f(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(Vec2f(pTexCoord->x, pTexCoord->y));
	}

	LoadBones(MeshIndex, paiMesh, Bones);

	// Populate the index buffer
	for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		//assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}

	LoadMeshMat(paiMesh, paiScene, MeshIndex);
}

void Model2::LoadMeshMat(const aiMesh* paiMesh,
						 const aiScene* pScene,
						 uint MeshIndex)
{

	ecbool dontqueue = ectrue;
	//std::string dir = StripFile(fullpath);
	//dir = MakeRelative(dir.c_str());
	
	std::string dir = StripFile(relative);
	//dir = MakeRelative(dir.c_str());

	//InfoMess(dir.c_str(), dir.c_str());

#if 0
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		Material* pmat = &Textures[i];

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;
#if 0
		char cbasefull[SFH_MAX_PATH+1];
		FullPath(relative, cbasefull);
		basefull = StripFile(std::string(cbasefull));
#endif

		Log("Material #"<<i<<std::endl;


		for (unsigned int pi = 0; pi < pMaterial->mNumProperties;++pi) {
			aiMaterialProperty* prop = pMaterial->mProperties[pi];

			g_applog <<"\tMaterial property: \""<<prop->mKey.data<<"\" = \""<<prop->mData<<"\"");
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			Log("Have diffuse texture");

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Diffuse texture filepath: %s", path.data);

				diffrel = dir + path.data;
				diffrel = MakeRelative(diffrel.c_str());
				//InfoMess("path.data", path.data);
				//InfoMess("diffrel", diffrel.c_str());

#if 0
				if(strstr(relative, ".ms3d") >= 0)
				{
					char cdifffull[SFH_MAX_PATH+1];
					strcpy(cdifffull, path.data);
					StripPath(cdifffull);
					difffull = dir + cdifffull;
				}
#endif
#if 1
				char cbaserel[SFH_MAX_PATH+1];
				sprintf(cbaserel, "%s", diffrel.c_str());
				StripExt(cbaserel);
				baserel = cbaserel;
#endif
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Specular texture filepath: %s", path.data);
				specrel = dir + path.data;
				specrel = MakeRelative(specrel.c_str());
			}
		}
		else
		{
			char cspecrel[SFH_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			specrel = cspecrel;
			Log("Default specular texture filepath: %s", specrel);
		}

		if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Normal texture filepath: %s", path.data);
				normrel = dir + path.data;
				normrel = MakeRelative(normrel.c_str());
			}
		}
		else
		{
			char cnormrel[SFH_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			normrel = cnormrel;
			Log("Default normal texture filepath: %s", normrel);
		}

		char cteamrel[SFH_MAX_PATH+1];
		OwnPath(baserel.c_str(), cteamrel);
		teamrel = cteamrel;

		if(dontqueue)
		{
			CreateTex(pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->specularm, specrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->normalm, normrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse);
		}
		else
		{
			QueueTex(&pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->specularm, specrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->normalm, normrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse);
		}
	}
#endif

#if 0
	pmat->diffusem = 0;
	pmat->specularm = 0;
	pmat->normalm = 0;
	pmat->ownerm = 0;
#endif

#if 1
	//correct way from assimp_view
	if (paiMesh->mTextureCoords[0])
	{

//		Log("Mesh material "<<paiMesh->mMaterialIndex<<"/"<<pScene->mNumMaterials<<std::endl;

		////Entries.push_back(

		// extract all properties from the ASSIMP material structure
		const aiMaterial* pcMat = pScene->mMaterials[paiMesh->mMaterialIndex];
#if 0
Entries[i].MaterialIndex;

		//Log("mat "<<MaterialIndex<<" "<<Textures.size()<<std::endl;
		//

		if(MaterialIndex < Textures.size() && Textures.size())
		{

			if (Textures[MaterialIndex]) {
				Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}
			Material* mat = &Textures[MaterialIndex];
#endif
		Material* pmat = &Textures[paiMesh->mMaterialIndex];

		pmat->diffusem = 0;
		pmat->specularm = 0;
		pmat->normalm = 0;
		pmat->ownerm = 0;

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;

		aiString szPath;
		aiTextureMapMode mapU, mapV;

		//
		// DIFFUSE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_DIFFUSE(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piDiffuseTexture,&szPath);

			StripPath(szPath.data);
			//InfoMess("reldf", szPath.data);
			diffrel = dir + (szPath.data);
			//diffrel = MakeRelative(diffrel.c_str());

//			Log("\tDiffuse: "<<szPath.data<<std::endl;

			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0),(int*)&mapU);
			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0),(int*)&mapV);
			//aiTextureMapMode
			//aiTextureMapMode_Wrap;
			//aiTextureMapMode_Clamp;

#if 1
			char cbaserel[SFH_MAX_PATH+1];
			//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
				;//diffrel = MakeRelative( (dir + diffrel).c_str() );
			sprintf(cbaserel, "%s", diffrel.c_str());
			StripExt(cbaserel);
			baserel = cbaserel;
			//InfoMess("reldf", cbaserel);

			if(!TryRelative(diffrel.c_str()))
			{
				Log("Failed: %s", diffrel.c_str());
			//InfoMess("failed", cbaserel);

				diffrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
					;//diffrel = MakeRelative( (dir + diffrel).c_str() );
				sprintf(cbaserel, "%s", diffrel.c_str());
				StripExt(cbaserel);
				baserel = cbaserel;
				
				if(!TryRelative(baserel.c_str()))
				{
					Log("Failed: %s", diffrel.c_str());

					diffrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)diffrel.c_str(), &diffrel, dir.c_str()))
						;//diffrel = MakeRelative( (dir + diffrel).c_str() );
					sprintf(cbaserel, "%s", diffrel.c_str());
					StripExt(cbaserel);
					baserel = cbaserel;
				}
			}
#endif
//			Log("\tDiffuse after adjustment: "<<diffrel<<std::endl;
		}
		else
		{
//			Log("\tDiffuse not found! ");
		}

		//
		// SPECULAR TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SPECULAR(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piSpecularTexture,&szPath);
//			Log("\tSpecular texture filepath: " <<szPath.data<<std::endl;
			StripPath(szPath.data);
			specrel = dir + szPath.data;
			//specrel = MakeRelative(specrel.c_str());
			//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
				;//specrel = MakeRelative( (dir + specrel).c_str() );
			
			if(!TryRelative(specrel.c_str()))
			{
//				Log("Failed: %s", specrel.c_str());

				specrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
				
				if(!TryRelative(specrel.c_str()))
				{
					Log("Failed: %s", specrel.c_str());

					specrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)specrel.c_str(), &specrel, dir.c_str()))
						;//specrel = MakeRelative( (dir + specrel).c_str() );
				}
			}

			Log("\tSpecular texture filepath after adjustment: %s", specrel.c_str());
		}
		else
		{
			char cspecrel[SFH_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			cspecrel[SFH_MAX_PATH] = 0;
			specrel = cspecrel;
			Log("\tDefault specular texture filepath: %s", specrel.c_str());
			//InfoMess(cspecrel,cspecrel);
		}

		//
		// NORMAL/HEIGHT MAP ------------------------------------------------
		//
		ecbool bHM = ecfalse;
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_NORMALS(0),&szPath))
		{
			//FindValidPath(&szPath, dir.c_str());
			//LoadTexture(&pcMesh->piNormalTexture,&szPath);

			//LoadTexture(&pcMesh->piSpecularTexture,&szPath);
//			Log("\tNormal texture filepath: " <<szPath.data<<std::endl;
			StripPath(szPath.data);
			normrel = dir + szPath.data;
			//normrel = MakeRelative(normrel.c_str());
			//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
				;//normrel = MakeRelative( (dir + normrel).c_str() );
			
			if(!TryRelative(normrel.c_str()))
			{
//				Log("Failed: %s", normrel.c_str());

				normrel = dir + std::string("tex/") + std::string(szPath.data);
				//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
				
				if(!TryRelative(normrel.c_str()))
				{
					Log("Failed: %s", normrel.c_str());

					normrel = dir + std::string("textures/") + std::string(szPath.data);
					//if(SmartPath((char*)normrel.c_str(), &normrel, dir.c_str()))
						;//specrel = MakeRelative( (dir + specrel).c_str() );
				}
			}

			Log("\tNormal texture filepath after adjustment: %s", normrel.c_str());
		}
		else
		{
			char cnormrel[SFH_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			cnormrel[SFH_MAX_PATH] = 0;
			normrel = cnormrel;
			Log("\tDefault normal texture filepath: %s", normrel.c_str());
		}

		char cownrel[SFH_MAX_PATH+1];
		OwnPath(baserel.c_str(), cownrel);
		teamrel = cownrel;
		//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
			;//teamrel = MakeRelative( (dir + teamrel).c_str() );

		if(!TryRelative(teamrel.c_str()))
		{
			Log("Failed: %s", teamrel.c_str());

			teamrel = dir + std::string("tex/") + std::string(szPath.data);
			//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
				;//specrel = MakeRelative( (dir + specrel).c_str() );

			if(!TryRelative(teamrel.c_str()))
			{
				Log("Failed: %s", teamrel.c_str());

				teamrel = dir + std::string("textures/") + std::string(szPath.data);
				//if(SmartPath((char*)teamrel.c_str(), &teamrel, dir.c_str()))
					;//specrel = MakeRelative( (dir + specrel).c_str() );
			}
		}

		Log("\tDefault team texture filepath: %s", normrel.c_str());

		if(dontqueue)
		{
			if(!CreateTex(pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse))
				ErrMess(diffrel.c_str(), "Error loading diffuse");
			if(!CreateTex(pmat->specularm, specrel.c_str(), ecfalse, ecfalse))
				ErrMess(specrel.c_str(), "Error loading specular");
			if(!CreateTex(pmat->normalm, normrel.c_str(), ecfalse, ecfalse))
				ErrMess(normrel.c_str(), "Error loading normal");
			if(!CreateTex(pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse))
				ErrMess(teamrel.c_str(), "Error loading team");
		}
		else
		{
			QueueTex(&pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->specularm, specrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->normalm, normrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse);
		}
	}
	else
	{
		//ErrMess("Error", "Model without textures not supported");
		//return;

//		Log("Mesh material "<<paiMesh->mMaterialIndex<<"/"<<pScene->mNumMaterials<<std::endl;

		// extract all properties from the ASSIMP material structure
		const aiMaterial* pcMat = pScene->mMaterials[paiMesh->mMaterialIndex];

		Material* pmat = &Textures[paiMesh->mMaterialIndex];

		pmat->diffusem = 0;
		pmat->specularm = 0;
		pmat->normalm = 0;
		pmat->ownerm = 0;
	}
#endif
}

void Model2::LoadBones(uint MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
		uint BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		//InfoMess("bone", pMesh->mBones[i]->mName.data);

		if (BoneMapping.find(BoneName) == BoneMapping.end()) {
			// Allocate an index for a new bone
			BoneIndex = NumBones;
			NumBones++;
			BoneInfo bi;
			BoneInfo.push_back(bi);
			BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
			BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = BoneMapping[BoneName];
		}

		for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
			uint VertexID = Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}

	//if(NumBones <= 0)
	//	InfoMess("NumBones <= 0", "NumBones <= 0");
}


ecbool Model2::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = Filename.substr(0, SlashIndex);
	}

	Dir = MakeRelative(Dir.c_str());

	ecbool Ret = ectrue;

#if 0

	// Initialize the materials
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		Textures[i] = NULL;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\") {
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;

				//TODO load tex
				//Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());
#if 0
				if (!Textures[i]->Load()) {
					printf("Error loading texture '%s'\n", FullPath.c_str());
					delete Textures[i];
					Textures[i] = NULL;
					Ret = ecfalse;
				}
				else {
					printf("%d - loaded texture '%s'\n", i, FullPath.c_str());
				}
#endif
			}
		}
	}
#endif


#if 0
	//correct way from assimp_view
	if (pcSource->mTextureCoords[0])
	{

		//
		// DIFFUSE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_DIFFUSE(0),&szPath))
		{
			LoadTexture(&pcMesh->piDiffuseTexture,&szPath);

			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0),(int*)&mapU);
			aiGetMaterialInteger(pcMat,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0),(int*)&mapV);
		}

		//
		// SPECULAR TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SPECULAR(0),&szPath))
		{
			LoadTexture(&pcMesh->piSpecularTexture,&szPath);
		}

		//
		// OPACITY TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_OPACITY(0),&szPath))
		{
			LoadTexture(&pcMesh->piOpacityTexture,&szPath);
		}
		else
		{
			int flags = 0;
			aiGetMaterialInteger(pcMat,AI_MATKEY_TEXFLAGS_DIFFUSE(0),&flags);

			// try to find out whether the diffuse texture has any
			// non-opaque pixels. If we find a few, use it as opacity texture
			if (pcMesh->piDiffuseTexture && !(flags & aiTextureFlags_IgnoreAlpha) && HasAlphaPixels(pcMesh->piDiffuseTexture))
			{
				int iVal;

				// NOTE: This special value is set by the tree view if the user
				// manually removes the alpha texture from the view ...
				if (AI_SUCCESS != aiGetMaterialInteger(pcMat,"no_a_frod",0,0,&iVal))
				{
					pcMesh->piOpacityTexture = pcMesh->piDiffuseTexture;
					pcMesh->piOpacityTexture->AddRef();
				}
			}
		}

		//
		// AMBIENT TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_AMBIENT(0),&szPath))
		{
			LoadTexture(&pcMesh->piAmbientTexture,&szPath);
		}

		//
		// EMISSIVE TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_EMISSIVE(0),&szPath))
		{
			LoadTexture(&pcMesh->piEmissiveTexture,&szPath);
		}

		//
		// Shininess TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_SHININESS(0),&szPath))
		{
			LoadTexture(&pcMesh->piShininessTexture,&szPath);
		}

		//
		// Lightmap TEXTURE ------------------------------------------------
		//
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_LIGHTMAP(0),&szPath))
		{
			LoadTexture(&pcMesh->piLightmapTexture,&szPath);
		}


		//
		// NORMAL/HEIGHT MAP ------------------------------------------------
		//
		ecbool bHM = ecfalse;
		if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_NORMALS(0),&szPath))
		{
			LoadTexture(&pcMesh->piNormalTexture,&szPath);
		}
		else
		{
			if(AI_SUCCESS == aiGetMaterialString(pcMat,AI_MATKEY_TEXTURE_HEIGHT(0),&szPath))
			{
				LoadTexture(&pcMesh->piNormalTexture,&szPath);
			}
			else bib = ectrue;
			bHM = ectrue;
		}

		// normal/height maps are sometimes mixed up. Try to detect the type
		// of the texture std::list<Brush>::iteratormatically
		if (pcMesh->piNormalTexture)
		{
			HMtoNMIfNecessary(pcMesh->piNormalTexture, &pcMesh->piNormalTexture,bHM);
		}
	}
#endif

	ecbool dontqueue = ectrue;
	std::string dir = StripFile(fullpath);

#if 1
	for (uint i = 0 ; i < pScene->mNumMaterials ; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		Material* pmat = &Textures[i];

		std::string diffrel;
		std::string specrel;
		std::string normrel;
		std::string teamrel;
		std::string baserel;
#if 0
		char cbasefull[SFH_MAX_PATH+1];
		FullPath(relative, cbasefull);
		basefull = StripFile(std::string(cbasefull));
#endif

//		Log("Material #"<<i<<std::endl;


		for (unsigned int pi = 0; pi < pMaterial->mNumProperties;++pi) {
			aiMaterialProperty* prop = pMaterial->mProperties[pi];

//			g_applog <<"\tMaterial property: \""<<prop->mKey.data<<"\" = \""<<prop->mData<<"\"");
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			Log("Have diffuse texture");

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Diffuse texture filepath: %s", path.data);

				diffrel = dir + path.data;
				diffrel = MakeRelative(diffrel.c_str());
				//InfoMess("path.data", path.data);
				//InfoMess("diffrel", diffrel.c_str());

#if 0
				if(strstr(relative, ".ms3d") >= 0)
				{
					char cdifffull[SFH_MAX_PATH+1];
					strcpy(cdifffull, path.data);
					StripPath(cdifffull);
					difffull = dir + cdifffull;
				}
#endif
#if 1
				char cbaserel[SFH_MAX_PATH+1];
				sprintf(cbaserel, "%s", diffrel.c_str());
				cbaserel[SFH_MAX_PATH] = 0;
				StripExt(cbaserel);
				baserel = cbaserel;
#endif
			}
		}
		else
		{
			ErrMess("Error", "Material without diffuse texture! Choose a texture...");

			char path[SFH_MAX_PATH+1];
			char initdir[SFH_MAX_PATH+1];
			FullPath("textures/", initdir);

			if(!OpenFileDialog(initdir, path))
			{
				ErrMess("Error", "No texture chosen");
				return ecfalse;
			}

			diffrel = path;
			diffrel = MakeRelative(diffrel.c_str());
			//InfoMess("path.data", path.data);
			//InfoMess("diffrel", diffrel.c_str());

#if 0
			if(strstr(relative, ".ms3d") >= 0)
			{
				char cdifffull[SFH_MAX_PATH+1];
				strcpy(cdifffull, path.data);
				StripPath(cdifffull);
				difffull = dir + cdifffull;
			}
#endif
#if 1
			char cbaserel[SFH_MAX_PATH+1];
			sprintf(cbaserel, "%s", diffrel.c_str());
			StripExt(cbaserel);
			baserel = cbaserel;
#endif
		}

		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Specular texture filepath: %s", path.data);
				specrel = dir + path.data;
				specrel = MakeRelative(specrel.c_str());
			}
		}
		else
		{
			char cspecrel[SFH_MAX_PATH+1];
			SpecPath(baserel.c_str(), cspecrel);
			cspecrel[SFH_MAX_PATH] = 0;
			specrel = cspecrel;
			Log("Default specular texture filepath: %s", specrel.c_str());
		}

		if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				Log("Normal texture filepath: %s", path.data);
				normrel = dir + path.data;
				normrel = MakeRelative(normrel.c_str());
			}
		}
		else
		{
			char cnormrel[SFH_MAX_PATH+1];
			NormPath(baserel.c_str(), cnormrel);
			cnormrel[SFH_MAX_PATH] = 0;
			normrel = cnormrel;
			Log("Default normal texture filepath: %s", normrel.c_str());
		}

		char cteamrel[SFH_MAX_PATH+1];
		OwnPath(baserel.c_str(), cteamrel);
		teamrel = cteamrel;

		if(dontqueue)
		{
			CreateTex(pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->specularm, specrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->normalm, normrel.c_str(), ecfalse, ecfalse);
			CreateTex(pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse);
		}
		else
		{
			QueueTex(&pmat->diffusem, diffrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->specularm, specrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->normalm, normrel.c_str(), ecfalse, ecfalse);
			QueueTex(&pmat->ownerm, teamrel.c_str(), ecfalse, ecfalse);
		}
	}
#endif

	return Ret;
}


void Model2::Render(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat)
{
	//glBindVertexArray(VAO);
	//Log("entries "<<Entries.size()<<std::endl;

	Shader* s = g_sh+g_curS;

	//Vec3f pos(0,0,0);

	Matrix modelmat;
	modelmat.translation((const float*)&pos);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);

	Matrix mvp;
	mvp.set(g_camproj.m);
	mvp.postmult2(g_camview);
	mvp.postmult2(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m);
#endif
	modelview.postmult2(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m);

	//modelview.set(g_camview.m);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(pScene->mNumAnimations > 0)
		//if(0)
	{
		float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);

		//Log("bone transform ");
		//Log("pScene->mAnimations[0]->mTicksPerSecond "<<pScene->mAnimations[0]->mTicksPerSecond<<std::endl;
		//Log("pScene->mAnimations[0]->mDuration "<<pScene->mAnimations[0]->mDuration<<std::endl;
		//Log("RunningTime "<<RunningTime<<std::endl;
		//Log("BoneTransforms.size() "<<BoneTransforms.size()<<std::endl;
	}
	else
	{
		BoneTransforms.resize( BoneInfo.size() );

		for(int i=0; i<BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("NumBones = "<<NumBones<<std::endl;
#endif

	//Log("NumBones = "<<NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

#if 0
#define NUM_BONES_PER_VERTEX 4

	struct BoneInfo
	{
		Matrix BoneOffset;
		Matrix FinalTransformation;

		BoneInfo()
		{
			BoneOffset.SetZero();
			FinalTransformation.SetZero();
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
#endif

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
#if 0
		Matrix BoneTransform = Bones[BoneIDs[0]] * Weights[0];
		BoneTransform     += Bones[BoneIDs[1]] * Weights[1];
		BoneTransform     += Bones[BoneIDs[2]] * Weights[2];
		BoneTransform     += Bones[BoneIDs[3]] * Weights[3];
#else
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//ecbool influenced = ecfalse;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			//for(int bi=0; bi<1; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			//if(Bones[i].IDs[bi] >= BoneInfo.size())
			//	continue;

			//if(Bones[i].IDs[bi] >= BoneTransforms.size())
			//{
			//	Log("bone id "<<Bones[i].IDs[bi]<<" out of "<<BoneTransforms.size()<<std::endl;
			//	continue;
			//}

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			//influenced = ectrue;

			//Transform += BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		//if(!influenced)
		//	Transform.InitIdentity();
#endif

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * scale;
		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
	}

	//If we're including inclines, adjust vertex heights
	if(//(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME) &&
		g_doinclines && g_currincline)
	{
		int heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = (int)g_cornerinc[g_currincline][0];
		heights[1] = (int)g_cornerinc[g_currincline][1];
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = (int)g_cornerinc[g_currincline][2];
		heights[3] = (int)g_cornerinc[g_currincline][3];

		Heightmap hm;
		hm.alloc(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

#if 0
		if(g_currincline == 2)
			Log("g_currincline "<<g_currincline<<" inc4:"<<
			g_cornerinc[g_currincline][0]<<","<<
			g_cornerinc[g_currincline][1]<<","<<
			g_cornerinc[g_currincline][2]<<","<<
			g_cornerinc[g_currincline][3]<<std::endl;
#endif

		//TODO need to take into account rotation matrix also
		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight2(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
#if 0
			if(g_currincline == 2
				&& hm.accheight(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z) > 0.0f)
			{
				Log("adj #"<<i<<" xz"
					<<(g_tilesize/2.0f + pos.x + TransformedPos[i].x)<<","<<(g_tilesize/2.0f + pos.z + TransformedPos[i].z)
					<<" y+"<<
					hm.accheight(
					g_tilesize/2.0f + pos.x + TransformedPos[i].x,
					g_tilesize/2.0f + pos.z + TransformedPos[i].z)<<std::endl;
			}
#endif
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < Entries.size() ; i++)
	{
		const uint MaterialIndex = Entries[i].MaterialIndex;

		//Log("mat "<<MaterialIndex<<" "<<Textures.size()<<std::endl;
		//

		if(MaterialIndex < Textures.size() && Textures.size())
		{

#if 0
			if (Textures[MaterialIndex]) {
				Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}
#else
			Material* mat = &Textures[MaterialIndex];

			//Log("mat->diffusem"<<mat->diffusem<<std::endl;
			//Log("g_texture[ mat->diffusem ].texname"<<g_texture[ mat->diffusem ].texname<<std::endl;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->diffusem ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->specularm ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->normalm ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->ownerm ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_OWNERMAP], 3);
#endif

		}
		else
		{
			static unsigned int notex = 0;

			if(!notex)
				CreateTex(notex, "textures/notex.jpg", ecfalse, ectrue);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_OWNERMAP], 3);
		}

#if 0
		glDrawElementsBaseVertex(GL_TRIANGLES,
			Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(uint) * Entries[i].BaseIndex),
			Entries[i].BaseVertex);
#endif

		const unsigned int numindices = Entries[i].NumIndices;
		const unsigned int basevertex = Entries[i].BaseVertex;
		const unsigned int baseindex = Entries[i].BaseIndex;
		const unsigned int numunique = Entries[i].NumUniqueVerts;

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, &TransformedPos[basevertex]);
		glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[basevertex]);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, &TransformedNorm[basevertex]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//glDrawArrays(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, 3, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);

#if 0
		static ecbool once = ecfalse;

		if(!once)
		{
			once = ectrue;

			for(int v=0; v<numindices; v++)
			{
				Vec3f v3d = Positions[basevertex + Indices[baseindex + v]];
				Log("["<<v<<"] ("<<(v%3)<<"): "<<v3d.x<<","<<v3d.y<<","<<v3d.z<<std::endl;
			}
		}
#endif
		//Log("indices "<<numindices<<std::endl;
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
}

void Model2::RenderDepth(int frame, Vec3f pos, int origmodeli, Vec3f scale, Matrix rotmat)
{
	//glBindVertexArray(VAO);
	//Log("entries "<<Entries.size()<<std::endl;

	Shader* s = g_sh+g_curS;

	//Vec3f pos(0,0,0);

	Matrix modelmat;
	modelmat.translation((const float*)&pos);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);

	Matrix mvp;
	mvp.set(g_camproj.m);
	mvp.postmult2(g_camview);
	mvp.postmult2(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m);

	//modelview.set(g_camview.m);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m);

	//perform frame transformation on-the-fly
	std::vector<Matrix> BoneTransforms;

	//if(!pScene)
	//	ErrMess("No scene", "No scene");

#if 1
	if(pScene->mNumAnimations > 0)
	{
		float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float frames = TicksPerSecond * (float)pScene->mAnimations[0]->mDuration;
		float percentage = (float)frame / frames;
		float RunningTime = percentage * (float)pScene->mAnimations[0]->mDuration;

		BoneTransform(RunningTime, BoneTransforms);
	}
	else
	{
		BoneTransforms.resize( BoneInfo.size() );

		for(int i=0; i<BoneInfo.size(); i++)
		{
			BoneTransforms[i].InitIdentity();
		}
	}
#else
	//BoneTransform(0, BoneTransforms);
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;
	//Log("NumBones = "<<NumBones<<std::endl;
#endif

	//Log("NumBones = "<<NumBones<<std::endl;
	//Log("BoneTransforms.size() = "<<BoneTransforms.size()<<std::endl;

	std::vector<Vec3f> TransformedPos;
	std::vector<Vec3f> TransformedNorm;
	TransformedPos.resize(Positions.size());
	TransformedNorm.resize(Normals.size());

#if 0
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
#endif

	Model2* origm = &g_model2[origmodeli];

	for(uint i=0; i<Positions.size(); i++)
	{
#if 0
		Matrix BoneTransform = Bones[BoneIDs[0]] * Weights[0];
		BoneTransform     += Bones[BoneIDs[1]] * Weights[1];
		BoneTransform     += Bones[BoneIDs[2]] * Weights[2];
		BoneTransform     += Bones[BoneIDs[3]] * Weights[3];
#else
		Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
		Transform.InitIdentity();

		//ecbool influenced = ecfalse;

		for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
		{
			if(Bones[i].IDs[bi] < 0)
				continue;

			//if(Bones[i].IDs[bi] >= BoneInfo.size())
			//	continue;

			//if(Bones[i].IDs[bi] >= BoneTransforms.size())
			//{
			//	Log("bone id "<<Bones[i].IDs[bi]<<" out of "<<BoneTransforms.size()<<std::endl;
			//	continue;
			//}

			if(Bones[i].Weights[bi] == 0.0f)
				continue;

			//influenced = ectrue;

			//Transform += BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

			if(bi == 0)
				Transform = BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
			else
				Transform += BoneTransforms[ Bones[i].IDs[bi] ] * Bones[i].Weights[bi];
		}

		//if(!influenced)
		//	Transform.InitIdentity();
#endif

		Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
		Transformed.transform(Transform);
		Transformed.transform(rotmat);
		//Transformed = Transformed * scale;
		TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * scale;
		TransformedNorm[i] = origm->Normals[i];
		TransformedNorm[i].transform(Transform);
		TransformedNorm[i].transform(rotmat);
	}

	//If we're including inclines, adjust vertex heights
	if(//(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME) &&
		g_doinclines && g_currincline)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0];
		heights[1] = g_cornerinc[g_currincline][1];
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2];
		heights[3] = g_cornerinc[g_currincline][3];

		Heightmap hm;
		hm.alloc(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();

		for(uint i=0; i<TransformedPos.size(); i++)
		{
			//TransformedPos[i].y += Bilerp(&hm,
			//	g_tilesize/2.0f + pos.x + TransformedPos[i].x,
			//	g_tilesize/2.0f + pos.z + TransformedPos[i].z);
			TransformedPos[i].y += hm.accheight2(
				g_tilesize/2.0f + pos.x + TransformedPos[i].x,
				g_tilesize/2.0f + pos.z + TransformedPos[i].z);
		}

		//Regenerate normals:
		//Not possible, based on vertices alone, because we would also need to blend shared faces,
		//so leave this inaccuracy for now. TODO
	}

	for (uint i = 0 ; i < Entries.size() ; i++)
	{
		const uint MaterialIndex = Entries[i].MaterialIndex;

		if(MaterialIndex < Textures.size() && Textures.size())
		{

#if 0
			if (Textures[MaterialIndex]) {
				Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}
#else
			Material* mat = &Textures[MaterialIndex];

			//Log("mat->diffusem"<<mat->diffusem<<std::endl;
			//Log("g_texture[ mat->diffusem ].texname"<<g_texture[ mat->diffusem ].texname<<std::endl;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->diffusem ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
#if 0
			glActiveTextureARB(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->specularm ].texname);
			glUniform1iARB(g_sh[g_curS].slot[SSLOT_SPECULARMAP], 1);

			glActiveTextureARB(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->normalm ].texname);
			glUniform1iARB(g_sh[g_curS].slot[SSLOT_NORMALMAP], 2);

			glActiveTextureARB(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->ownerm ].texname);
			glUniform1iARB(g_sh[g_curS].slot[SSLOT_OWNERMAP], 3);
#endif
#endif
		}
		else
		{
			static unsigned int notex = 0;

			if(!notex)
				CreateTex(notex, "textures/notex.jpg", ecfalse, ectrue);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ notex ].texname);
			glUniform1i(g_sh[g_curS].slot[SSLOT_TEXTURE0], 0);
		}

#if 0
		glDrawElementsBaseVertex(GL_TRIANGLES,
			Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(uint) * Entries[i].BaseIndex),
			Entries[i].BaseVertex);
#endif

		const unsigned int numindices = Entries[i].NumIndices;
		const unsigned int basevertex = Entries[i].BaseVertex;
		const unsigned int baseindex = Entries[i].BaseIndex;
		const unsigned int numunique = Entries[i].NumUniqueVerts;

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, &TransformedPos[basevertex]);
		glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[basevertex]);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, &TransformedNorm[basevertex]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//glDrawArrays(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, 3, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);

#if 0
		static ecbool once = ecfalse;

		if(!once)
		{
			once = ectrue;

			for(int v=0; v<numindices; v++)
			{
				Vec3f v3d = Positions[basevertex + Indices[baseindex + v]];
				Log("["<<v<<"] ("<<(v%3)<<"): "<<v3d.x<<","<<v3d.y<<","<<v3d.z<<std::endl;
			}
		}
#endif
		//Log("indices "<<numindices<<std::endl;
	}

	// Make sure the VAO is not changed from the outside
	//glBindVertexArray(0);
}

uint Model2::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


uint Model2::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	//assert(pNodeAnim->mNumRotationKeys > 0);

	for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


uint Model2::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	//assert(pNodeAnim->mNumScalingKeys > 0);

	for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


void Model2::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	uint NextPositionIndex = (PositionIndex + 1);
	//assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void Model2::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	uint NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}


void Model2::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	uint NextScalingIndex = (ScalingIndex + 1);
	//assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void Model2::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const Matrix& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = pScene->mAnimations[0];

	Matrix NodeTransformation(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		Matrix ScalingM;
		ScalingM.initscale(Scaling.x, Scaling.y, Scaling.z);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		Matrix RotationM = Matrix(RotationQ.GetMatrix());

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		Matrix TranslationM;
		TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
		//NodeTransformation = ScalingM * RotationM * TranslationM;
		//NodeTransformation = ScalingM;
		//NodeTransformation.postmult(RotationM);
		//NodeTransformation.postmult(TranslationM);

		//NodeTransformation = TranslationM;
		//NodeTransformation.postmult(RotationM);
		//NodeTransformation.postmult(ScalingM);
	}

	Matrix GlobalTransformation = ParentTransform * NodeTransformation;

	if (BoneMapping.find(NodeName) != BoneMapping.end()) {
		uint BoneIndex = BoneMapping[NodeName];
		BoneInfo[BoneIndex].FinalTransformation = GlobalInverseTransform * GlobalTransformation * BoneInfo[BoneIndex].BoneOffset;
		//BoneInfo[BoneIndex].FinalTransformation = BoneInfo[BoneIndex].BoneOffset * GlobalTransformation * GlobalInverseTransform;

		//BoneInfo[BoneIndex].FinalTransformation = BoneInfo[BoneIndex].BoneOffset;
		//BoneInfo[BoneIndex].FinalTransformation.postmult(GlobalTransformation);
		//BoneInfo[BoneIndex].FinalTransformation.postmult(GlobalInverseTransform);

		//BoneInfo[BoneIndex].FinalTransformation = GlobalInverseTransform;
		//BoneInfo[BoneIndex].FinalTransformation.postmult(GlobalTransformation);
		//BoneInfo[BoneIndex].FinalTransformation.postmult(BoneInfo[BoneIndex].BoneOffset);
	}

	for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}


void Model2::BoneTransform(float TimeInSeconds, std::vector<Matrix>& Transforms)
{
	Matrix Identity;
	Identity.InitIdentity();

	float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)pScene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, pScene->mRootNode, Identity);

	Transforms.resize(NumBones);

	for (uint i = 0 ; i < NumBones ; i++) {
		Transforms[i] = BoneInfo[i].FinalTransformation;
	}
}


const aiNodeAnim* Model2::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void FreeModels2()
{
	for(int i=0; i<MODELS2; i++)
	{
		Model2* m = &g_model2[i];

		if(!m->on)
			continue;

		m->destroy();
	}
}
