

#include "../sim/map.h"
#include "save.h"
#include "../platform.h"
#include "../math/polygon.h"
#include "../texture.h"
#include "../render/vertexarray.h"
#include "../utils.h"
#include "saveedm.h"
#include "../save/edmap.h"
#include "savemap.h"
#include "../app/segui.h"
#include "../bsp/brush.h"
#include "../app/undo.h"






// chooses the brush texture from one of its sides
// that defines the attributes for the whole brush.
void BrushAttribTex(Brush* brush)
{
	int finaltex = 0;

	for(int sideidx=0; sideidx<brush->nsides; sideidx++)
	{
		BrushSide* side = &brush->sides[sideidx];

		int sidetex = side->diffusem;

		if(g_texture[sidetex].transp)
			finaltex = sidetex;

		if(g_texture[sidetex].grate)
			finaltex = sidetex;

		if(g_texture[sidetex].breakable)
			finaltex = sidetex;

		if(g_texture[sidetex].passthru)
			finaltex = sidetex;

		if(g_texture[sidetex].fabric)
			finaltex = sidetex;
	}

	brush->texture = finaltex;
}

// Compile a list of textures used by map brushes
// and save that table to file. Also, a list of
// texture references is set (texrefs) that
// indexes into the written texture table based
// on the diffuse texture index (which indexes into
// g_texture, the global texture array).
void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes)
{
	for(int i=0; i<TEXTURES; i++)
		texrefs[i] = -1;

	// the compiled index of textures used in the map
	std::list<TexRef> compilation;

	for(std::list<Brush>::iterator b=brushes.begin(); b!=brushes.end(); b++)
	{
		//List of sides we will check the textures of.
		//We make a list because we include not only
		//sides of the brush, but the door closed-state model
		//sides too, which might theoretically have different
		//textures if the door model/whatever wasn't updated.
		std::list<BrushSide*> sides;

		for(int i=0; i<b->nsides; i++)
			sides.push_back(&b->sides[i]);

		if(b->door)
		{
			EdDoor* door = b->door;

			for(int i=0; i<door->nsides; i++)
				sides.push_back(&door->sides[i]);
		}

		for(std::list<BrushSide*>::iterator si=sides.begin(); si!=sides.end(); si++)
		{
			BrushSide* s = *si;

			ecbool found = ecfalse;
			for(std::list<TexRef>::iterator j=compilation.begin(); j!=compilation.end(); j++)
			{
				if(j->diffindex == s->diffusem)
				{
					found = ectrue;
					break;
				}
			}

			if(!found)
			{
				Texture* t = &g_texture[s->diffusem];
				TexRef tr;
				tr.filepath = MakeRelative(t->fullpath.c_str());
				tr.texname = t->texname;
				tr.diffindex = s->diffusem;
				compilation.push_back(tr);
			}
		}

		ecbool found = ecfalse;
		for(std::list<TexRef>::iterator j=compilation.begin(); j!=compilation.end(); j++)
		{
			if(j->diffindex == b->texture)
			{
				found = ectrue;
				break;
			}
		}

		if(!found)
		{
			Texture* t = &g_texture[b->texture];
			TexRef tr;
			tr.filepath = MakeRelative(t->fullpath.c_str());
			tr.texname = t->texname;
			tr.diffindex = b->texture;
			compilation.push_back(tr);
		}
	}

	//Write the texture table to file.
	int nrefs = compilation.size();
	fwrite(&nrefs, sizeof(int), 1, fp);

#if 0
	Log("writing "<<nrefs<<" tex refs");
	
#endif

	int j=0;
	for(std::list<TexRef>::iterator i=compilation.begin(); i!=compilation.end(); i++, j++)
	{
		texrefs[ i->diffindex ] = j;
		int strl = i->filepath.length()+1;
		fwrite(&strl, sizeof(int), 1, fp);

#if 0
		Log("writing "<<strl<<"-long tex ref");
		
#endif

		fwrite(i->filepath.c_str(), sizeof(char), strl, fp);
	}
}

void ReadTexs(FILE* fp, TexRef** texrefs)
{
	int nrefs;
	fread(&nrefs, sizeof(int), 1, fp);

#if 0
	Log("reading "<<nrefs<<" tex refs");
	
#endif

#ifdef LOADMAP_DEBUG
	Log("nrefs = "<<nrefs<<std::endl;
	
#endif

	(*texrefs) = new TexRef[nrefs];

	for(int i=0; i<nrefs; i++)
	{
		TexRef* tr = &(*texrefs)[i];
		int strl;
		fread(&strl, sizeof(int), 1, fp);

#if 0
		Log("reading "<<strl<<"-long tex ref");
		
#endif

		char* filepath = new char[strl];
		fread(filepath, sizeof(char), strl, fp);
#ifdef LOADMAP_DEBUG
	Log("filepath = "<<filepath<<std::endl;
	
#endif
		tr->filepath = filepath;
		delete [] filepath;
		CreateTex(tr->diffindex, tr->filepath.c_str(), ecfalse, ectrue);
		tr->texname = g_texture[tr->diffindex].texname;

		char basepath[SFH_MAX_PATH+1];
		strcpy(basepath, tr->filepath.c_str());
		StripExt(basepath);

		char specpath[SFH_MAX_PATH+1];
		SpecPath(basepath, specpath);

		CreateTex(tr->specindex, specpath, ecfalse, ectrue);

		char normpath[SFH_MAX_PATH+1];
		NormPath(basepath, normpath);

		CreateTex(tr->normindex, normpath, ecfalse, ectrue);

		char ownpath[SFH_MAX_PATH+1];
		OwnPath(basepath, ownpath);

		CreateTex(tr->ownindex, ownpath, ecfalse, ectrue);
	}
}

void SaveBrushes(FILE* fp, int* texrefs, std::list<Brush>& brushes)
{
	int nbrush = brushes.size();
	fwrite(&nbrush, sizeof(int), 1, fp);

#if 0
	Log("writing "<<nbrush<<" brushes at "<<ftell(fp)<<std::endl;
	

	int i=0;
#endif

	for(std::list<Brush>::iterator b=brushes.begin(); b!=brushes.end(); b++)
	{
		SaveBrush(fp, texrefs, &*b);

#if 0
		Log("wrote brush "<<i<<" end at "<<ftell(fp)<<std::endl;
		i++;
#endif
	}
}

// We don't draw the sky textured brushes.
// They are replaced by a sky box.
// But we might want to know which
// ones are sky brushes for some reason.
// We need to save a table of transparent brushes
// because we need to alpha-sort them for proper drawing.
// Now that we saved the transparent brushes,
// we need to store the opaque ones.
// This is because we draw the opaque ones first,
// and then the transparent ones in the right order.
void SaveBrushRefs(FILE* fp, std::list<Brush>& brushes)
{
	std::list<int> opaqbrushrefs;
	std::list<int> transpbrushrefs;
	std::list<int> skybrushrefs;

	int brushidx = 0;
	for(std::list<Brush>::iterator brushitr=brushes.begin(); brushitr!=brushes.end(); brushitr++, brushidx++)
	{
		ecbool hassky = ecfalse;
		ecbool hastransp = ecfalse;

		//Check if any of the sides have a transparent texture
		for(int sideidx = 0; sideidx < brushitr->nsides; sideidx++)
		{
			BrushSide* pside = &brushitr->sides[sideidx];
			unsigned int sidetex = pside->diffusem;

			//If the brush has a sky texture attribute,
			//we want to write it.
			if(g_texture[sidetex].sky)
			{
				skybrushrefs.push_back(brushidx);
				hassky = ectrue;
				break;
			}

			//If side has transparent texture,
			//the brush index will be added to the table.
			if(g_texture[sidetex].transp)
			{
				transpbrushrefs.push_back(brushidx);
				hastransp = ectrue;
				break;
			}
		}

		if(!hassky && !hastransp)
		{
			opaqbrushrefs.push_back(brushidx);
		}
	}

	//Write the brush references tables.

	// Opaque brushes
	int nbrushrefs = opaqbrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = opaqbrushrefs.begin(); refitr != opaqbrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}

	// Transparent brushes
	nbrushrefs = transpbrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = transpbrushrefs.begin(); refitr != transpbrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}

	// Sky brushes
	nbrushrefs = skybrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = skybrushrefs.begin(); refitr != skybrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}
}

//Read the brush references tables
void ReadBrushRefs(FILE* fp, Map* map)
{
	// Opaque brushes
	int nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->opaquebrush.push_back(ref);
	}

	// Transparent brushes
	nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->transpbrush.push_back(ref);
	}

	// Sky brushes
	nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->skybrush.push_back(ref);
	}
}

void SaveMap(const char* fullpath, std::list<Brush>& brushes)
{
	FILE* fp = fopen(fullpath, "wb");

	char tag[] = TAG_MAP;
	fwrite(tag, sizeof(char), 5, fp);

	float version = MAP_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

#if 0
	Log("brushes to write: "<<brushes.size()<<std::endl;
#endif

	int texrefs[TEXTURES];
	SaveTexs(fp, texrefs, brushes);

#if 0
	Log("write brushes at "<<ftell(fp)<<std::endl;
	
#endif

	SaveBrushes(fp, texrefs, brushes);
	SaveBrushRefs(fp, brushes);

	fclose(fp);
}

void ReadBrushes(FILE* fp, TexRef* texrefs, Map* map)
{
	int nbrush;
	fread(&nbrush, sizeof(int), 1, fp);

	map->nbrush = nbrush;
	map->brush = new Brush[nbrush];

#ifdef LOADMAP_DEBUG
	Log("nbrush = "<<nbrush<<std::endl;
	
#endif

	for(int i=0; i<nbrush; i++)
	{
		ReadBrush(fp, texrefs, &map->brush[i]);

#ifdef LOADMAP_DEBUG
	Log("added b");
	
#endif

//		Log("read brush "<<i<<" end at "<<ftell(fp)<<std::endl;
	}
}

ecbool LoadMap(const char* fullpath, Map* map)
{
	map->destroy();

	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
	{
		Log("Failed to show map %s", fullpath);
		return ecfalse;
	}

	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	char realtag[] = TAG_MAP;
	//if(ecfalse)
	if(tag[0] != realtag[0] ||  tag[1] != realtag[1] || tag[2] != realtag[2] || tag[3] != realtag[3] || tag[4] != realtag[4])
	{
		fclose(fp);
		ErrMess("Error", "Not a map file (invalid header tag).");
		return ecfalse;
	}

	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != MAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map's version (%f) doesn't match %f.", version, MAP_VERSION);
		ErrMess("Error", msg);
		return ecfalse;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 1");
	
#endif

	TexRef* texrefs = NULL;

	ReadTexs(fp, &texrefs);

//	Log("read brushes at "<<ftell(fp)<<std::endl;

	ReadBrushes(fp, texrefs, map);
	ReadBrushRefs(fp, map);

//	Log("loaded "<<map->nbrush<<" brushes ");
	

	if(texrefs)
	{
		delete [] texrefs;
		texrefs = NULL;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 2");
	
#endif

	fclose(fp);

#ifdef LOADMAP_DEBUG
	Log("load map 3");
	
#endif

	return ectrue;
}






//#define LOADMAP_DEBUG

void SaveEdBrushSide(FILE* fp, BrushSide* s, int* texrefs)
{
	fwrite(&s->plane, sizeof(Plane3f), 1, fp);

	SaveVertexArray(fp, &s->drawva);

	fwrite(&texrefs[ s->diffusem ], sizeof(int), 1, fp);
	fwrite(&s->ntris, sizeof(int), 1, fp);
	fwrite(s->tris, sizeof(Triangle2), s->ntris, fp);
	fwrite(s->tceq, sizeof(Plane3f), 2, fp);

	SavePolygon(fp, &s->outline);

	fwrite(s->vindices, sizeof(int), s->outline.edv.size(), fp);
	fwrite(&s->centroid, sizeof(Vec3f), 1, fp);
}

void SaveEdBrushSides(FILE* fp, Brush* b, int* texrefs)
{
	int nsides = b->nsides;
	fwrite(&nsides, sizeof(int), 1, fp);

	for(int i=0; i<nsides; i++)
	{
		/*
	Plane3f plane;
	VertexArray drawva;
	unsigned int diffusem;
		*/
	/*
	int ntris;
	Triangle2* tris;
	Plane3f tceq[2];	//tex coord uv equations
	Polyg outline;
	int* vindices;	//indices into parent brush's shared vertex array
	Vec3f centroid;
	*/
		BrushSide* s = &b->sides[i];

		SaveEdBrushSide(fp, s, texrefs);
	}
}

void ReadBrushSide(FILE* fp, BrushSide* s, TexRef* texrefs)
{
	fread(&s->plane, sizeof(Plane3f), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("s->plane = "<<s->plane.normal.x<<","<<s->plane.normal.y<<","<<s->plane.normal.z<<","<<s->plane.d<<std::endl;
	
#endif

	ReadVertexArray(fp, &s->drawva);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 1");
	
#endif

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	s->diffusem = texrefs[ texrefindex ].diffindex;
	s->specularm = texrefs[ texrefindex ].specindex;
	s->normalm = texrefs[ texrefindex ].normindex;
	s->ownerm = texrefs[ texrefindex ].ownindex;
	fread(&s->ntris, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 2");
	
#endif

	s->tris = new Triangle2[ s->ntris ];
	fread(s->tris, sizeof(Triangle2), s->ntris, fp);
	fread(s->tceq, sizeof(Plane3f), 2, fp);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 3");
	
#endif

	ReadPolygon(fp, &s->outline);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 4");
	
#endif

	s->vindices = new int[ s->outline.edv.size() ];
	fread(s->vindices, sizeof(int), s->outline.edv.size(), fp);
	fread(&s->centroid, sizeof(Vec3f), 1, fp);
}

void ReadBrushSides(FILE* fp, Brush* b, TexRef* texrefs)
{
	int nsides;
	fread(&nsides, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nsides = "<<nsides<<std::endl;
	
#endif

	if(b->sides)
	{
		delete [] b->sides;
		b->sides = NULL;
		b->nsides = 0;
	}

	for(int i=0; i<nsides; i++)
	{
		BrushSide s;

		ReadBrushSide(fp, &s, texrefs);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 5");
	
#endif

		b->add(s);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 6");
	
#endif
	}
}

Brush* GetBrushNum(int target, EdMap* map)
{
	int cnt = 0;
	for(std::list<Brush>::iterator i=map->brush.begin(); i!=map->brush.end(); i++, cnt++)
	{
		if(cnt == target)
			return &*i;
	}

	return NULL;
}

void SaveEdDoor(FILE* fp, EdDoor* door, int* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	ecbool startopen;
	Brush* brushp;
	Brush closedstate;*/

	fwrite(&door->axis, sizeof(Vec3f), 1, fp);
	fwrite(&door->point, sizeof(Vec3f), 1, fp);
	fwrite(&door->opendeg, sizeof(float), 1, fp);
	fwrite(&door->startopen, sizeof(ecbool), 1, fp);

	fwrite(&door->nsides, sizeof(int), 1, fp);
	for(int i=0; i<door->nsides; i++)
		SaveEdBrushSide(fp, &door->sides[i], texrefs);

#if 0
	Log("save ed door");
	for(int i=0; i<door->nsides; i++)
	{
		Log("side "<<i<<std::endl;
		Plane3f* p = &door->sides[i].plane;

		Log("plane = "<<p->normal.x<<","<<p->normal.y<<","<<p->normal.z<<",d="<<p->d<<std::endl;
	}
#endif
}

void ReadEdDoor(FILE* fp, EdDoor* door, TexRef* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	ecbool startopen;
	Brush* brushp;
	Brush closedstate;*/

#if 0
	MessageBox(g_hWnd, "read door", "aasd", NULL);
#endif

	fread(&door->axis, sizeof(Vec3f), 1, fp);
	fread(&door->point, sizeof(Vec3f), 1, fp);
	fread(&door->opendeg, sizeof(float), 1, fp);
	fread(&door->startopen, sizeof(ecbool), 1, fp);

	fread(&door->nsides, sizeof(int), 1, fp);


#if 0
	char msg[128];
	sprintf(msg, "door sides %d", door->nsides);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif

	door->sides = new BrushSide[door->nsides];
	for(int i=0; i<door->nsides; i++)
		ReadBrushSide(fp, &door->sides[i], texrefs);

#if 0
	Log("read ed door");
	for(int i=0; i<door->nsides; i++)
	{
		Log("side "<<i<<std::endl;
		Plane3f* p = &door->sides[i].plane;

		Log("plane = "<<p->normal.x<<","<<p->normal.y<<","<<p->normal.z<<",d="<<p->d<<std::endl;
	}
#endif
}

void SaveBrush(FILE* fp, int* texrefs, Brush* b)
{
	/*
		int nsides;
		BrushSide* sides;
		int nsharedv;
		Vec3f* sharedv;	//shared vertices array
	*/

	SaveEdBrushSides(fp, b, texrefs);
	fwrite(&b->nsharedv, sizeof(int), 1, fp);
	fwrite(b->sharedv, sizeof(Vec3f), b->nsharedv, fp);
	fwrite(&texrefs[b->texture], sizeof(int), 1, fp);

	ecbool hasdoor = ecfalse;

	if(b->door)
		hasdoor = ectrue;

	fwrite(&hasdoor, sizeof(ecbool), 1, fp);

	if(hasdoor)
		SaveEdDoor(fp, b->door, texrefs);
}

void SaveBrushes(FILE* fp, int* texrefs, EdMap* map)
{
	int nbrush = map->brush.size();
	fwrite(&nbrush, sizeof(int), 1, fp);

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		SaveBrush(fp, texrefs, &*b);
	}
}

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b)
{
	ReadBrushSides(fp, b, texrefs);

	if(b->sharedv)
	{
		delete [] b->sharedv;
		b->sharedv = NULL;
		b->nsharedv = 0;
	}

	fread(&b->nsharedv, sizeof(int), 1, fp);
#ifdef LOADMAP_DEBUG
	Log("b->nsharedv = "<<b->nsharedv<<std::endl;
	
#endif

	b->sharedv = new Vec3f[ b->nsharedv ];
	fread(b->sharedv, sizeof(Vec3f), b->nsharedv, fp);

	//b.remaptex();	//comment this out

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	b->texture = texrefs[texrefindex].diffindex;

	ecbool hasdoor = ecfalse;
	fread(&hasdoor, sizeof(ecbool), 1, fp);

	if(hasdoor)
	{
		b->door = new EdDoor();
		ReadEdDoor(fp, b->door, texrefs);
	}
}

void ReadBrushes(FILE* fp, TexRef* texrefs, EdMap* map)
{
	int nbrush;
	fread(&nbrush, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nbrush = "<<nbrush<<std::endl;
	
#endif

	for(int i=0; i<nbrush; i++)
	{
		Brush b;
		ReadBrush(fp, texrefs, &b);
		map->brush.push_back(b);

#ifdef LOADMAP_DEBUG
	Log("added b");
	
#endif
	}
}

void ReadEdTexs(FILE* fp, TexRef** texrefs)
{
	int nrefs;
	fread(&nrefs, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nrefs = "<<nrefs<<std::endl;
	
#endif

	(*texrefs) = new TexRef[nrefs];

	for(int i=0; i<nrefs; i++)
	{
		TexRef* tr = &(*texrefs)[i];
		int strl;
		fread(&strl, sizeof(int), 1, fp);

		char* filepath = new char[strl];
		fread(filepath, sizeof(char), strl, fp);
#ifdef LOADMAP_DEBUG
	Log("filepath = "<<filepath<<std::endl;
	
#endif
		tr->filepath = filepath;
		delete [] filepath;
		CreateTex(tr->diffindex, tr->filepath.c_str(), ecfalse, ecfalse);
		tr->texname = g_texture[tr->diffindex].texname;

		char basepath[SFH_MAX_PATH+1];
		strcpy(basepath, tr->filepath.c_str());
		StripExt(basepath);

		char specpath[SFH_MAX_PATH+1];
		SpecPath(basepath, specpath);

		CreateTex(tr->specindex, specpath, ecfalse, ecfalse);

		char normpath[SFH_MAX_PATH+1];
		NormPath(basepath, normpath);

		CreateTex(tr->normindex, normpath, ecfalse, ecfalse);

		char ownpath[SFH_MAX_PATH+1];
		OwnPath(basepath, ownpath);

		CreateTex(tr->ownindex, ownpath, ecfalse, ecfalse);
	}
}

int BrushNum(Brush* b, EdMap* map)
{
	int cnt = 0;
	for(std::list<Brush>::iterator i=map->brush.begin(); i!=map->brush.end(); i++, cnt++)
	{
		if(&*i == b)
			return cnt;
	}

	return -1;
}

void SaveModelHolder(FILE* fp, ModelHolder* pmh)
{
	Model2* m = &g_model2[pmh->modeli];

	std::string relative = MakeRelative(m->fullpath.c_str());
	int nrelative = relative.size() + 1;

	fwrite(&nrelative, sizeof(int), 1, fp);
	fwrite(relative.c_str(), sizeof(char), nrelative, fp);

#if 0
	InfoMess("save relative", relative.c_str());
	InfoMess("save full", m->fullpath.c_str());
	char msg[128];
	sprintf(msg, "mi %d", pmh->modeli);
	InfoMess("modeli", msg);
#endif

	fwrite(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->translation, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void ReadModelHolder(FILE* fp, ModelHolder* pmh)
{
	int nrelative =0;
	fread(&nrelative, sizeof(int), 1, fp);

	char* relative = new char[nrelative];
	fread(relative, sizeof(char), nrelative, fp);
	pmh->modeli = LoadModel2(relative, Vec3f(1,1,1), Vec3f(0,0,0), ectrue);
	delete [] relative;

	fread(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fread(&pmh->translation, sizeof(Vec3f), 1, fp);
	fread(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void SaveModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = modelholders.size();

	fwrite(&nmh, sizeof(int), 1, fp);

	for(std::list<ModelHolder>::iterator iter = modelholders.begin(); iter != modelholders.end(); iter++)
	{
		SaveModelHolder(fp, &*iter);
	}
}

void ReadModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = 0;

	fread(&nmh, sizeof(int), 1, fp);

	for(int i = 0; i < nmh; i++)
	{
		ModelHolder mh;
		ReadModelHolder(fp, &mh);
		mh.retransform();
		modelholders.push_back(mh);
	}
}

void SaveEdMap(const char* fullpath, EdMap* map)
{
	FILE* fp = fopen(fullpath, "wb");

	char tag[] = TAG_EDMAP;
	fwrite(tag, sizeof(char), 5, fp);

	float version = EDMAP_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

	int texrefs[TEXTURES];
	SaveTexs(fp, texrefs, map->brush);

	int nframes = GetNumFrames();
	fwrite(&nframes, sizeof(int), 1, fp);

	SaveBrushes(fp, texrefs, map);
	SaveModelHolders(fp, g_modelholder);

	fclose(fp);
}

void ScaleAll(float factor)
{
	EdMap* map = &g_edmap;

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		std::list<float> oldus;
		std::list<float> oldvs;

		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* s = &b->sides[i];

			Vec3f sharedv = b->sharedv[ s->vindices[0] ];
			float u = sharedv.x*s->tceq[0].normal.x + sharedv.y*s->tceq[0].normal.y + sharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float v = sharedv.x*s->tceq[1].normal.x + sharedv.y*s->tceq[1].normal.y + sharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->tceq[0].normal = s->tceq[0].normal / factor;
			s->tceq[1].normal = s->tceq[1].normal / factor;
			Vec3f pop = PointOnPlane(s->plane);
			pop = pop * factor;
			s->plane.d = PlaneDistance(s->plane.normal, pop);
		}

		b->collapse();

		std::list<float>::iterator oldu = oldus.begin();
		std::list<float>::iterator oldv = oldvs.begin();

		for(int i=0; i<b->nsides; i++, oldu++, oldv++)
		{
			BrushSide* s = &b->sides[i];

			Vec3f newsharedv = b->sharedv[ s->vindices[0] ];

			float newu = newsharedv.x*s->tceq[0].normal.x + newsharedv.y*s->tceq[0].normal.y + newsharedv.z*s->tceq[0].normal.z + s->tceq[0].d;
			float newv = newsharedv.x*s->tceq[1].normal.x + newsharedv.y*s->tceq[1].normal.y + newsharedv.z*s->tceq[1].normal.z + s->tceq[1].d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->tceq[0].d -= changeu;
			s->tceq[1].d -= changev;
		}

		b->remaptex();
	}

	char msg[128];
	sprintf(msg, "scaled %f", factor);
	InfoMess("asd", msg);
}

ecbool LoadEdMap(const char* fullpath, EdMap* map)
{
	FreeEdMap(map);

	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
		return ecfalse;

	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	char realtag[] = TAG_EDMAP;
	//if(ecfalse)
	if(tag[0] != realtag[0] ||  tag[1] != realtag[1] || tag[2] != realtag[2] || tag[3] != realtag[3] || tag[4] != realtag[4])
	{
		fclose(fp);
		ErrMess("Error", "Not a project file (invalid header tag).");
		return ecfalse;
	}

	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != EDMAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		ErrMess("Error", msg);
		return ecfalse;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 1");
	
#endif

	TexRef* texrefs = NULL;

	ReadEdTexs(fp, &texrefs);

	
#ifdef LOADMAP_DEBUG
	Log("load map 1.01");
	
#endif


#if 1
	int nframes = 0;
	fread(&nframes, sizeof(int), 1, fp);
	SetNumFrames(nframes);
#endif

	
#ifdef LOADMAP_DEBUG
	Log("load map 1.1");
	
#endif

	ReadBrushes(fp, texrefs, map);
	
#ifdef LOADMAP_DEBUG
	Log("load map 1.2");
	
#endif

	ReadModelHolders(fp, g_modelholder);

#ifdef LOADMAP_DEBUG
	Log("load map 1.3");
	
#endif

	if(texrefs)
	{
		delete [] texrefs;
		texrefs = NULL;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 2");
	
#endif

	fclose(fp);

#ifdef LOADMAP_DEBUG
	Log("load map 3");
	
#endif

	//ScaleAll(1.75f);

	return ectrue;
}

void FreeEdMap(EdMap* map)
{
	g_sel1b = NULL;
	g_selB.clear();
	g_dragV = -1;
	g_dragS = -1;
	g_dragD = -1;
	g_dragW = ecfalse;
	g_dragM = -1;
	g_selM.clear();

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		for(int i=0; i<b->nsides; i++)
		{
			BrushSide* s = &b->sides[i];

			if(s->diffusem != 0)
				FreeTexture(s->diffusem);
			if(s->specularm != 0)
				FreeTexture(s->specularm);
			if(s->normalm != 0)
				FreeTexture(s->normalm);
			if(s->ownerm != 0)
				FreeTexture(s->ownerm);
		}
	}

	map->brush.clear();

	FreeModels2();
	FreeModelHolders();
	ClearUndo();
}
