
#include "ms3d.h"
#include "../texture.h"
#include "../utils.h"
#include "model2.h"
#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/quaternion.h"
#include "../math/matrix.h"
#include "../math/3dmath.h"
#include "../math/matrix.h"
#include "vertexarray.h"
#include "../save/saveedm.h"

//int frame = 0;

MS3DModel::MS3DModel()
{
	numMeshes = 0;
	pMeshes = NULL;
	numMaterials = 0;
	pMaterials = NULL;
	numTriangles = 0;
	pTriangles = NULL;
	numVertices = 0;
	pVertices = NULL;
	numJoints = 0;
	pJoints = NULL;
}

MS3DModel::~MS3DModel()
{
	destroy();
}

void MS3DModel::destroy()
{
	int i;
	for ( i = 0; i < numMeshes; i++ )
		delete[] pMeshes[i].pTriangleIndices;
	for ( i = 0; i < numMaterials; i++ )
		delete[] pMaterials[i].pTextureFilename;

	numMeshes = 0;
	if ( pMeshes != NULL )
	{
		delete[] pMeshes;
		pMeshes = NULL;
	}

	numMaterials = 0;
	if ( pMaterials != NULL )
	{
		delete[] pMaterials;
		pMaterials = NULL;
	}

	numTriangles = 0;
	if ( pTriangles != NULL )
	{
		delete[] pTriangles;
		pTriangles = NULL;
	}

	numVertices = 0;
	if ( pVertices != NULL )
	{
		delete[] pVertices;
		pVertices = NULL;
	}

	numJoints = 0;
	if(pJoints != NULL)
	{
		delete [] pJoints;
		pJoints = NULL;
	}
}

void MS3DModel::loadtex(unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, ecbool dontqueue)
{
	for ( int i = 0; i < numMaterials; i++ )
		if ( strlen( pMaterials[i].pTextureFilename ) > 0 )
		{
			//QueueTex(&pMaterials[i].diffusem, pMaterials[i].pTextureFilename, ectrue);

			char basefile[SFH_MAX_PATH+1];
			strcpy(basefile, pMaterials[i].pTextureFilename);
			StripPath(basefile);
			char basename[SFH_MAX_PATH+1];
			sprintf(basename, "%s%s", relative, basefile);
			char difffile[SFH_MAX_PATH+1];
			strcpy(difffile, basename);
			StripExt(basename);
			char specfile[SFH_MAX_PATH+1];
			char normfile[SFH_MAX_PATH+1];
			char ownfile[SFH_MAX_PATH+1];
			SpecPath(basename, specfile);
			NormPath(basename, normfile);
			OwnPath(basename, ownfile);

			if(dontqueue)
			{
				CreateTex(diffm, difffile, ecfalse, ectrue);
				CreateTex(specm, specfile, ecfalse, ectrue);
				CreateTex(normm, normfile, ecfalse, ectrue);
				CreateTex(ownm, ownfile, ecfalse, ectrue);

				if(diffm == 0)
				{
					char msg[SFH_MAX_PATH+1];
					sprintf(msg, "Couldn't load diffuse texture %s", difffile);
					ErrMess("Error", msg);
				}
				if(specm == 0)
				{
					char msg[SFH_MAX_PATH+1];
					sprintf(msg, "Couldn't load specular texture %s", specfile);
					ErrMess("Error", msg);
				}
				if(normm == 0)
				{
					char msg[SFH_MAX_PATH+1];
					sprintf(msg, "Couldn't load specular texture %s", normfile);
					ErrMess("Error", msg);
				}
				if(ownm == 0)
				{
					char msg[SFH_MAX_PATH+1];
					sprintf(msg, "Couldn't load team color texture %s", ownfile);
					ErrMess("Error", msg);
				}
			}
			else
			{
				QueueTex(&diffm, difffile, ecfalse, ectrue);
				QueueTex(&specm, specfile, ecfalse, ectrue);
				QueueTex(&normm, normfile, ecfalse, ectrue);
				QueueTex(&ownm, ownfile, ecfalse, ectrue);
			}
		}
		//else
		//	pMaterials[i].diffusem = 0;
}

ecbool MS3DModel::load(const char *relative, unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, ecbool dontqueue)
{
	char full[SFH_MAX_PATH+1];
	FullPath(relative, full);

	std::ifstream inputFile( full, std::ios::in | std::ios::binary );
	if ( inputFile.fail())
	{
		Log("Couldn't show the model file %s ", relative);

		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Couldn't show the model file %s", relative);

		ErrMess("Error", msg);

		return ecfalse;
	}

	std::string reltemp = StripFile(relative);

	//if(strlen(reltemp.c_str()) == 0)
	//	reltemp += CORRECT_SLASH;

	strcpy(relative, reltemp.c_str());

	/*
	char pathTemp[SFH_MAX_PATH+1];
	int pathLength;
	for ( pathLength = strlen( filename ); --pathLength; )
	{
		if ( filename[pathLength] == '/' || filename[pathLength] == '\\' )
			break;
	}
	strncpy( pathTemp, filename, pathLength );

	int i;
	if ( pathLength > 0 )
	{
		pathTemp[pathLength++] = '/';
	}

	strncpy( filepath, filename, pathLength );
	*/

	inputFile.seekg( 0, std::ios::end );
	long fileSize = inputFile.tellg();
	inputFile.seekg( 0, std::ios::beg );

	char *pBuffer = new char[fileSize];
	inputFile.read( pBuffer, fileSize );
	inputFile.close();

	const char *pPtr = pBuffer;
	MS3DHeader *pHeader = ( MS3DHeader* )pPtr;
	pPtr += sizeof( MS3DHeader );

	if ( strncmp( pHeader->ID, "MS3D000000", 10 ) != 0 )
	{
		Log("Not an MS3D file %s", relative);
		return ecfalse;
    }

	if ( pHeader->version < 3 )
	{
		Log("I know nothing about MS3D v1.2, %s" , relative);

		char msg[SFH_MAX_PATH+1];
		sprintf(msg, "Incompatible MS3D v1.2 ", relative);

		ErrMess("Error", msg);

		return ecfalse;
	}

	int nVertices = *( word* )pPtr;
	numVertices = nVertices;
	pVertices = new Vertex[nVertices];
	pPtr += sizeof( word );

	int i;
	for ( i = 0; i < nVertices; i++ )
	{
		MS3DVertex *pVertex = ( MS3DVertex* )pPtr;
		pVertices[i].boneID = pVertex->boneID;
		memcpy( pVertices[i].location, pVertex->vertex, sizeof( float )*3 );
		pPtr += sizeof( MS3DVertex );
	}

	int nTriangles = *( word* )pPtr;
	numTriangles = nTriangles;
	pTriangles = new Triangle[nTriangles];
	pPtr += sizeof( word );

	for ( i = 0; i < nTriangles; i++ )
	{
		MS3DTriangle *pTriangle = ( MS3DTriangle* )pPtr;
		int vertexIndices[3] = { pTriangle->vertexIndices[0], pTriangle->vertexIndices[1], pTriangle->vertexIndices[2] };
		float t[3] = { 1.0f-pTriangle->t[0], 1.0f-pTriangle->t[1], 1.0f-pTriangle->t[2] };
		memcpy( pTriangles[i].vertexNormals, pTriangle->vertexNormals, sizeof( float )*3*3 );
		memcpy( pTriangles[i].s, pTriangle->s, sizeof( float )*3 );
		memcpy( pTriangles[i].t, t, sizeof( float )*3 );
		memcpy( pTriangles[i].vertexIndices, vertexIndices, sizeof( int )*3 );
		pPtr += sizeof( MS3DTriangle );
	}

	int nGroups = *( word* )pPtr;
	numMeshes = nGroups;
	pMeshes = new Mesh[nGroups];
	pPtr += sizeof( word );
	for ( i = 0; i < nGroups; i++ )
	{
		pPtr += sizeof( byte );	// flags
		pPtr += 32;				// name

		word nTriangles = *( word* )pPtr;
		pPtr += sizeof( word );
		int *pTriangleIndices = new int[nTriangles];
		for ( int j = 0; j < nTriangles; j++ )
		{
			pTriangleIndices[j] = *( word* )pPtr;
			pPtr += sizeof( word );
		}

		char materialIndex = *( char* )pPtr;
		pPtr += sizeof( char );

		pMeshes[i].materialIndex = materialIndex;
		pMeshes[i].numTriangles = nTriangles;
		pMeshes[i].pTriangleIndices = pTriangleIndices;
	}

	int nMaterials = *( word* )pPtr;
	numMaterials = nMaterials;
	pMaterials = new Material[nMaterials];
	pPtr += sizeof( word );
	for ( i = 0; i < nMaterials; i++ )
	{
		MS3DMaterial *pMaterial = ( MS3DMaterial* )pPtr;
		memcpy( pMaterials[i].ambient, pMaterial->ambient, sizeof( float )*4 );
		memcpy( pMaterials[i].diffuse, pMaterial->diffuse, sizeof( float )*4 );
		memcpy( pMaterials[i].specular, pMaterial->specular, sizeof( float )*4 );
		memcpy( pMaterials[i].emissive, pMaterial->emissive, sizeof( float )*4 );
		pMaterials[i].shininess = pMaterial->shininess;
		if ( strncmp( pMaterial->diffusem, ".\\", 2 ) == 0 ) {
			// MS3D 1.5.x relative path
			//StripPath(pMaterial->diffusem);
			//pMaterials[i].pTextureFilename = new char[ strlen(relativepath.c_str()) + strlen(pMaterial->diffusem) + 1 ];
			//strcpy( pMaterials[i].pTextureFilename, reltemp.c_str() );
			//sprintf(pMaterials[i].pTextureFilename, "%s%s", relativepath.c_str(), pMaterial->diffusem);
			pMaterials[i].pTextureFilename = new char[strlen( pMaterial->diffusem )+1];
			strcpy( pMaterials[i].pTextureFilename, pMaterial->diffusem );
		}
		else {
			// MS3D 1.4.x or earlier - absolute path
			pMaterials[i].pTextureFilename = new char[strlen( pMaterial->diffusem )+1];
			strcpy( pMaterials[i].pTextureFilename, pMaterial->diffusem );
		}
		StripPath(pMaterials[i].pTextureFilename);
		pPtr += sizeof( MS3DMaterial );
	}

	loadtex(diffm, specm, normm, ownm, dontqueue);

	float animFPS = *( float* )pPtr;
	pPtr += sizeof( float );

	// skip currentTime
	pPtr += sizeof( float );

	totalFrames = *( int* )pPtr;
	pPtr += sizeof( int );

	totalTime = totalFrames*1000.0/animFPS;

	numJoints = *( word* )pPtr;
	pPtr += sizeof( word );

	pJoints = new Joint[numJoints];

	struct JointNameListRec
	{
		int jointIndex;
		const char *pName;
	};

	const char *pTempPtr = pPtr;

	JointNameListRec *pNameList = new JointNameListRec[numJoints];
	for ( i = 0; i < numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pTempPtr;
		pTempPtr += sizeof( MS3DJoint );
		pTempPtr += sizeof( MS3DKeyframe )*( pJoint->numRotationKeyframes+pJoint->numTranslationKeyframes );

		pNameList[i].jointIndex = i;
		pNameList[i].pName = pJoint->name;
	}

	for ( i = 0; i < numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pPtr;
		pPtr += sizeof( MS3DJoint );

		int j, parentIndex = -1;
		if ( strlen( pJoint->parentName ) > 0 )
		{
			for ( j = 0; j < numJoints; j++ )
			{
				if ( _stricmp( pNameList[j].pName, pJoint->parentName ) == 0 )
				{
					parentIndex = pNameList[j].jointIndex;
					break;
				}
			}
			if ( parentIndex == -1 ) {
				Log("Unable to find parent bone in MS3D file");
				return ecfalse;
			}
		}

		memcpy( pJoints[i].localRotation, pJoint->rotation, sizeof( float )*3 );
		memcpy( pJoints[i].localTranslation, pJoint->translation, sizeof( float )*3 );
		pJoints[i].parent = parentIndex;
		pJoints[i].numRotationKeyframes = pJoint->numRotationKeyframes;
		pJoints[i].pRotationKeyframes = new Keyframe[pJoint->numRotationKeyframes];
		pJoints[i].numTranslationKeyframes = pJoint->numTranslationKeyframes;
		pJoints[i].pTranslationKeyframes = new Keyframe[pJoint->numTranslationKeyframes];

		for ( j = 0; j < pJoint->numRotationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			setjointkf( i, j, pKeyframe->time*1000.0f, pKeyframe->parameter, ectrue );
		}

		for ( j = 0; j < pJoint->numTranslationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			setjointkf( i, j, pKeyframe->time*1000.0f, pKeyframe->parameter, ecfalse );
		}
	}
	delete[] pNameList;

	setupjoints();

	delete[] pBuffer;

	restart();

	Log(relative);

	return ectrue;
}

void MS3DModel::genva(VertexArray** vertexArrays, Vec3f scale, Vec3f translate, const char* filepath)
{
	(*vertexArrays) = new VertexArray[ totalFrames ];

	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;

	int numverts = 0;

	for ( int i = 0; i < numMeshes; i++ )
		numverts += pMeshes[i].numTriangles*3;

	for(int n = 0; n < totalFrames; n++)
	{
		(*vertexArrays)[n].numverts = numverts;
		(*vertexArrays)[n].vertices = new Vec3f[ numverts ];
		(*vertexArrays)[n].texcoords = new Vec2f[ numverts ];
		(*vertexArrays)[n].normals = new Vec3f[ numverts ];
	}

	restart();

	std::vector<Vec3f>* normalweights;

	normalweights = new std::vector<Vec3f>[numverts];

	for(int f = 0; f < totalFrames; f++)
	{
		advanceanim();

#if 0
		Log(std::endl<<std::endl;
		Log("frame #"<<f<<std::endl;
#endif

		for(int index = 0; index < numVertices; index++)
		{
			normalweights[index].clear();
		}

		int vert = 0;

		vertices = (*vertexArrays)[f].vertices;
		texcoords = (*vertexArrays)[f].texcoords;
		normals = (*vertexArrays)[f].normals;

		for(int i = 0; i < numMeshes; i++)
		{
			for(int j = 0; j < pMeshes[i].numTriangles; j++)
			{
				int triangleIndex = pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &pTriangles[triangleIndex];

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->vertexIndices[k];

					if(pVertices[index].boneID == -1)
					{
						//Log("\tno tran");

						texcoords[vert].x = pTri->s[k];
						texcoords[vert].y = 1.0f - pTri->t[k];

						normals[vert].x = pTri->vertexNormals[k][0];
						normals[vert].y = pTri->vertexNormals[k][1];
						normals[vert].z = pTri->vertexNormals[k][2];

						vertices[vert].x = pVertices[index].location[0] * scale.x + translate.x;
						vertices[vert].y = pVertices[index].location[1] * scale.y + translate.y;
						vertices[vert].z = pVertices[index].location[2] * scale.z + translate.z;
					}
					else
					{

						// rotate according to transformation matrix
						const Matrix& final = pJoints[pVertices[index].boneID].final;

						texcoords[vert].x = pTri->s[k];
						texcoords[vert].y = 1.0f - pTri->t[k];

						Vec3f newNormal(pTri->vertexNormals[k]);
						newNormal.transform3(final);
						newNormal = Normalize(newNormal);

						normals[vert] = newNormal;

						Vec3f newVertex(pVertices[index].location);
						newVertex.transform(final);

						vertices[vert].x = newVertex.x * scale.x + translate.x;
						vertices[vert].y = newVertex.y * scale.y + translate.y;
						vertices[vert].z = newVertex.z * scale.z + translate.z;

#if 0
						Vec3f off;
						off.set(pVertices[index].location);
						off = off * scale + translate;
						off = vertices[vert] - off;

						Log("\tyes tran "<<off.x<<","<<off.y<<","<<off.z<<" ");
#endif
					}

					//vert ++;

					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;

				}

				Vec3f normal;
				Vec3f tri[3];
				tri[0] = vertices[vert-3];
				tri[1] = vertices[vert-3+1];
				tri[2] = vertices[vert-3+2];
				//normal = Normal2(tri);
				normal = Normal(tri);	//Reverse order
				//normals[i] = normal;
				//normals[i+1] = normal;
				//normals[i+2] = normal;

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->vertexIndices[k];
					normalweights[index].push_back(normal);
				}
			}
		}

		vert = 0;

		for(int i = 0; i < numMeshes; i++)
		{
			for(int j = 0; j < pMeshes[i].numTriangles; j++)
			{
				int triangleIndex = pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &pTriangles[triangleIndex];

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->vertexIndices[k];

					Vec3f weighsum(0, 0, 0);

					for(int l=0; l<normalweights[index].size(); l++)
					{/*
						if(strstr(filepath, "flat"))
						{
							//Log("weighsum + "<<normalweights[index][l].x<<","<<normalweights[index][l].y<<","<<normalweights[index][l].z<<std::endl;
							//
						}*/

						weighsum = weighsum + normalweights[index][l] / (float)normalweights[index].size();
					}
					/*
					if(strstr(filepath, "flat"))
					{
						Log("weighsum = "<<weighsum.x<<","<<weighsum.y<<","<<weighsum.z<<std::endl;
						
					}*/

					normals[vert] = weighsum;
					//normals[vert+1] = weighsum;
					//normals[vert+2] = weighsum;

					//vert ++;

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
}

void MS3DModel::setjointkf( int jointIndex, int keyframeIndex, float time, float *parameter, ecbool isRotation )
{
	Keyframe& keyframe = isRotation ? pJoints[jointIndex].pRotationKeyframes[keyframeIndex] :
		pJoints[jointIndex].pTranslationKeyframes[keyframeIndex];

	keyframe.jointIndex = jointIndex;
	keyframe.time = time;
	memcpy( keyframe.parameter, parameter, sizeof( float )*3 );
}

void MS3DModel::setupjoints()
{
	int i;
	for ( i = 0; i < numJoints; i++ )
	{
		Joint& joint = pJoints[i];

		joint.relative.rotrad( joint.localRotation );
		joint.relative.translation( joint.localTranslation );
		if ( joint.parent != -1 )
		{
			joint.absolute.set( pJoints[joint.parent].absolute.m);
			joint.absolute.postmult2( joint.relative );
		}
		else
			joint.absolute.set( joint.relative.m);
	}

	for ( i = 0; i < numVertices; i++ )
	{
		Vertex& vertex = pVertices[i];

		if ( vertex.boneID != -1 )
		{
			Matrix& matrix = pJoints[vertex.boneID].absolute;

			matrix.inverseTranslateVect( vertex.location );
			matrix.inverseRotateVect( vertex.location );
		}
	}

	for ( i = 0; i < numTriangles; i++ ) {
		Triangle& triangle = pTriangles[i];
		for ( int j = 0; j < 3; j++ ) {
			const Vertex& vertex = pVertices[triangle.vertexIndices[j]];
			if ( vertex.boneID != -1 ) {
				Matrix& matrix = pJoints[vertex.boneID].absolute;
				matrix.inverseRotateVect( triangle.vertexNormals[j] );
			}
		}
	}
}

void MS3DModel::restart()
{
	frame = 0;
	for ( int i = 0; i < numJoints; i++ )
	{
		pJoints[i].currentRotationKeyframe = pJoints[i].currentTranslationKeyframe = 0;
		pJoints[i].final.set( pJoints[i].absolute.m);
	}
}

void MS3DModel::advanceanim()
{
	//static int frame = 0;

	double time = totalTime * (double)frame/(double)totalFrames;

	frame++;

	if(time > totalTime)
	{
		restart();
		time = 0;
	}

	for(int i = 0; i < numJoints; i++)
	{
		float transVec[3];
		Matrix transform;
		int frame;
		Joint *pJoint = &pJoints[i];

		if(pJoint->numRotationKeyframes == 0 && pJoint->numTranslationKeyframes == 0)
		{
#if 0
			pJoint->final.set( pJoint->absolute.m);
#else
			Matrix relativeFinal( pJoint->relative );

			if ( pJoint->parent == -1 )
				pJoint->final.set( relativeFinal.m );
			else
			{
				pJoint->final.set( pJoints[pJoint->parent].final.m );
				pJoint->final.postmult2( relativeFinal );
			}
#endif
			continue;
		}

		frame = pJoint->currentTranslationKeyframe;
		while(frame < pJoint->numTranslationKeyframes && pJoint->pTranslationKeyframes[frame].time < time)
		{
			frame++;
		}
		pJoint->currentTranslationKeyframe = frame;

		if(frame == 0)
			memcpy( transVec, pJoint->pTranslationKeyframes[0].parameter, sizeof ( float )*3 );
		else if(frame == pJoint->numTranslationKeyframes)
			memcpy( transVec, pJoint->pTranslationKeyframes[frame-1].parameter, sizeof ( float )*3 );
		else
		{
			//assert( frame > 0 && frame < pJoint->numTranslationKeyframes );

			const MS3DModel::Keyframe& curFrame = pJoint->pTranslationKeyframes[frame];
			const MS3DModel::Keyframe& prevFrame = pJoint->pTranslationKeyframes[frame-1];

			float timeDelta = curFrame.time-prevFrame.time;
			float interpValue = ( float )(( time-prevFrame.time )/timeDelta );

			transVec[0] = prevFrame.parameter[0]+( curFrame.parameter[0]-prevFrame.parameter[0] )*interpValue;
			transVec[1] = prevFrame.parameter[1]+( curFrame.parameter[1]-prevFrame.parameter[1] )*interpValue;
			transVec[2] = prevFrame.parameter[2]+( curFrame.parameter[2]-prevFrame.parameter[2] )*interpValue;
		}

		frame = pJoint->currentRotationKeyframe;
		while(frame < pJoint->numRotationKeyframes && pJoint->pRotationKeyframes[frame].time < time)
		{
			frame++;
		}
		pJoint->currentRotationKeyframe = frame;

		if(frame == 0)
			transform.rotrad( pJoint->pRotationKeyframes[0].parameter );
		else if(frame == pJoint->numRotationKeyframes)
			transform.rotrad( pJoint->pRotationKeyframes[frame-1].parameter );
		else
		{
			const MS3DModel::Keyframe& curFrame = pJoint->pRotationKeyframes[frame];
			const MS3DModel::Keyframe& prevFrame = pJoint->pRotationKeyframes[frame-1];

			float timeDelta = curFrame.time-prevFrame.time;
			float interpValue = (float)(( time-prevFrame.time )/timeDelta );

#if 0
			Quaternion qPrev( prevFrame.parameter );
			Quaternion qCur( curFrame.parameter );
			Quaternion qFinal( qPrev, qCur, interpValue );
			transform.setRotationQuaternion( qFinal );
#else
			float rotVec[3];

			rotVec[0] = prevFrame.parameter[0]+( curFrame.parameter[0]-prevFrame.parameter[0] )*interpValue;
			rotVec[1] = prevFrame.parameter[1]+( curFrame.parameter[1]-prevFrame.parameter[1] )*interpValue;
			rotVec[2] = prevFrame.parameter[2]+( curFrame.parameter[2]-prevFrame.parameter[2] )*interpValue;

			transform.rotrad( rotVec );
#endif
		}

		transform.translation( transVec );
		Matrix relativeFinal( pJoint->relative );
		relativeFinal.postmult2( transform );

		if ( pJoint->parent == -1 )
			pJoint->final.set( relativeFinal.m );
		else
		{
			pJoint->final.set( pJoints[pJoint->parent].final.m );
			pJoint->final.postmult2( relativeFinal );
		}
	}

#if 0
	for(int i = 0; i < numJoints; i++)
	{
		float transVec[3];
		Matrix transform;
		int frame;
		Joint *pJoint = &pJoints[i];

		float* m = pJoint->final.m;

		Log(std::endl<<std::endl;
		Log("joint #"<<i<<"  of "<<pJoint->parent<<std::endl;
		Log("["<<m[0]<<","<<m[1]<<","<<m[2]<<","<<m[3]<<"]");
		Log("["<<m[4]<<","<<m[5]<<","<<m[6]<<","<<m[7]<<"]");
		Log("["<<m[8]<<","<<m[9]<<","<<m[10]<<","<<m[11]<<"]");
		Log("["<<m[12]<<","<<m[13]<<","<<m[14]<<","<<m[15]<<"]");
		Log(std::endl<<std::endl;
	}
#endif
}
