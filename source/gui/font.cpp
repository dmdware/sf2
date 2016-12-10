










#include "../platform.h"
#include "font.h"
#include "../texture.h"
#include "../render/shader.h"
#include "../window.h"
#include "../utils.h"
#include "gui.h"
#include "icon.h"
#include "richtext.h"
#include "../sim/player.h"
#include "../debug.h"

Font g_font[FONTS];

static short gx;
static short gy;
static int nextlb;  //next [i] to skip line
static int lastspace;
static int j;
static short x0;
static int i;
static short x;
static short y;
static int w;
static int h;
static int size;
static int line;
//static char* g_str;
static RichText* g_rtext;
static int g_currfont;
static int startline;
static int starti;
static short gstartx;
static short goffstartx;
static float frame[4];
static std::list<RichPart>::const_iterator g_rpartit;
static int pglyphi;	//RichText part's [i] index
static float currcolor[4];
static ecbool debugtest = ecfalse;
static ecbool shadowpass = ecfalse;

void BreakLine()
{
	Font* f = &g_font[g_currfont];
	line++;
	x = goffstartx;
	y += f->gheight;
}

void PrepLineBreak()
{
	Font* f = &g_font[g_currfont];
	Glyph* g2;

	int lastspace = -1;
	short x0 = gstartx;
	std::list<RichPart>::const_iterator p = g_rpartit;

	for(int pj=pglyphi, j=i; j<size; j++)
	{
		if(p->type == RICH_TEXT)
		{
			unsigned int k = p->text.data[pj];
			if(k == '\n')
			{
				nextlb = j+1;

				//if(debugtest)	InfoMess("j_1", "j+1");

				return;
			}

			g2 = &f->glyph[k];
			x0 += g2->origsize[0];

			if(k == ' ' || k == '\t')
				lastspace = j;

			pj++;

			if(pj >= p->text.length)
			{
				p++;	//corpc fix

				//fixed?
				if(p == g_rtext->part.end())
				{
					nextlb = -1;
					return;
				}

				pj = 0;
			}
		}
		else if(p->type == RICH_ICON)
		{
			Icon* icon = &g_icon[p->icon];
			float hscale = f->gheight / (float)icon->height;
			x0 += (float)icon->width * hscale;

			p++;

			//corpc fix
			if(p == g_rtext->part.end())
			{
				nextlb = -1;
				return;
			}

			pj = 0;
		}

		if(x0 > w+gstartx)
		{
			if(lastspace < 0)
			{
				nextlb = imax(j, i+1);

#if 0
				if(debugtest)
				{
					char msg[128];
					//sprintf(msg, "x0 > w+gstartx \n w=%d"
					InfoMess("x0 > w+gstartx", "x0 > w+gstartx");
				}
#endif

				//if(w <= g2->w)
				//	nextlb++;

				return;
			}

			nextlb = lastspace+1;
			return;
		}
	}
}

void NextLineBreak()
{
	Font* f = &g_font[g_currfont];
	Glyph* g2;

	if(nextlb != starti)
	{
		BreakLine();
	}

	PrepLineBreak();
}

void DrawGlyph()
{
	Font* f = &g_font[g_currfont];

	if(g_rpartit->type == RICH_ICON)
	{
		Icon* icon = &g_icon[g_rpartit->icon];
		float hscale = f->gheight / (float)icon->height;

		UseIconTex(g_rpartit->icon);
		
		if(!shadowpass)
			glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], 1, 1, 1, 1);

		short left = x;
		short right = (short)( left + (float)icon->width * hscale );
		short top = y;
		short bottom = top + (short)f->gheight;
		DrawGlyph((float)left, (float)top, (float)right, (float)bottom, 0, 0, 1, 1);

		UseFontTex();
		
		if(!shadowpass)
			glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], currcolor[0], currcolor[1], currcolor[2], currcolor[3]);

		//Log("color[3] = "<<currcolor[3]);
	}
	else if(g_rpartit->type == RICH_TEXT)
	{
		unsigned int k = g_rpartit->text.data[pglyphi];
		Glyph* g = &f->glyph[k];

		short left = x + g->offset[0];
		short right = left + g->texsize[0];
		short top = y + g->offset[1];
		short bottom = top + g->texsize[1];
		DrawGlyph((float)left, (float)top, (float)right, (float)bottom, g->texcoord[0], g->texcoord[1], g->texcoord[2], g->texcoord[3]);
	}
}

void DrawGlyphF()
{
	Font* f = &g_font[g_currfont];

	if(g_rpartit->type == RICH_ICON)
	{
		Icon* icon = &g_icon[g_rpartit->icon];
		float hscale = f->gheight / (float)icon->height;

		UseIconTex(g_rpartit->icon);

		if(!shadowpass)
			glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], 1, 1, 1, 1);

		short left = x;
		short right = (short)( left + (float)icon->width * hscale );
		short top = y;
		short bottom = top + (short)f->gheight;
		DrawGlyphF((float)left, (float)top, (float)right, (float)bottom, 0, 0, 1, 1);

		UseFontTex();

		if(!shadowpass)
			glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], currcolor[0], currcolor[1], currcolor[2], currcolor[3]);

		//Log("color[3] = "<<currcolor[3]);
	}
	else if(g_rpartit->type == RICH_TEXT)
	{
		unsigned int k = g_rpartit->text.data[pglyphi];
		Glyph* g = &f->glyph[k];

		short left = x + g->offset[0];
		short right = left + g->texsize[0];
		short top = y + g->offset[1];
		short bottom = top + g->texsize[1];
		DrawGlyphF((float)left, (float)top, (float)right, (float)bottom, g->texcoord[0], g->texcoord[1], g->texcoord[2], g->texcoord[3]);
	}
}

void HighlGlyphF()
{
	/*
	Font* f = &g_font[g_currfont];
	Glyph* g = &f->glyph[g_str[i]];

	int left = x;
	int right = x + g->offset[0] + g->texsize[0];
	int top = y;
	int bottom = y + g->offset[1] + g->texsize[1];
	HighlGlyphF(left, top, right, bottom);*/

	Font* f = &g_font[g_currfont];

	if(g_rpartit->type == RICH_ICON)
	{
		Icon* icon = &g_icon[g_rpartit->icon];
		float hscale = f->gheight / (float)icon->height;

		UseIconTex(g_rpartit->icon);

		short left = x;
		short right = (short)( left + (float)icon->width * hscale );
		short top = y;
		short bottom = top + (short)f->gheight;
		HighlGlyphF((float)left, (float)top, (float)right, (float)bottom);

		//UseFontTex();	//corpd fix
	}
	else if(g_rpartit->type == RICH_TEXT)
	{
		unsigned int k = g_rpartit->text.data[pglyphi];
		Glyph* g = &f->glyph[k];

		short left = x + g->offset[0];
		short right = left + g->texsize[0];
		short top = y + g->offset[1];
		short bottom = top + g->texsize[1];
		HighlGlyphF((float)left, (float)top, (float)right, (float)bottom);
	}
}

void DrawCaret()
{
	Font* f = &g_font[g_currfont];
	Glyph* g = &f->glyph['|'];

	short left = x - g->origsize[1]/14;
	short right = left + g->texsize[0];
	short top = y + g->offset[1];
	short bottom = top + g->texsize[1];
	DrawGlyph((float)left, (float)top, (float)right, (float)bottom, g->texcoord[0], g->texcoord[1], g->texcoord[2], g->texcoord[3]);
}

void DrawCaretF()
{
	Font* f = &g_font[g_currfont];
	Glyph* g = &f->glyph['|'];

	short left = x - g->origsize[1]/14;
	short right = left + g->texsize[0];
	short top = y + g->offset[1];
	short bottom = top + g->texsize[1];
	DrawGlyphF((float)left, (float)top, (float)right, (float)bottom, g->texcoord[0], g->texcoord[1], g->texcoord[2], g->texcoord[3]);
}

void AdvGlyph()
{
	Font* f = &g_font[g_currfont];
	//Glyph* g = &f->glyph[g_str[i]];
	//x += g->origsize[0];

	if(g_rpartit->type == RICH_ICON)
	{
		Icon* icon = &g_icon[g_rpartit->icon];
		float hscale = f->gheight / (float)icon->height;
		x += (float)icon->width * hscale;

		g_rpartit++;
		pglyphi = 0;
	}
	else if(g_rpartit->type == RICH_TEXT)
	{
		unsigned int k = g_rpartit->text.data[pglyphi];
		Glyph* g = &f->glyph[k];
		x += g->origsize[0];

		pglyphi++;

		if(pglyphi >= g_rpartit->text.length)
		{
			g_rpartit++;
			pglyphi = 0;
		}
	}

	//if(debugtest)	Log("x adv "<<x);
}

void StartText(const RichText* text, int fnt, float width, float height, int ln, int realstartx)
{
	g_currfont = fnt;
	//g_str = (char*)text;
	g_rtext = (RichText*)text;
	//size = strlen(g_str);
	size = g_rtext->texlen();
	w = width;
	h = height;
	startline = ln;
	starti = 0;
	gstartx = realstartx;
	//g_rpartit = g_rtext->part.begin();
	//pglyphi = 0;
}

void UseFontTex()
{
	Shader* s = g_sh+g_curS;	//corpd fix
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_font[g_currfont].texindex ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
}

void UseIconTex(int ico)
{
	Shader* s = g_sh+g_curS;	//corpd fix
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_icon[ico].tex ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
}

void StartTextF(const RichText* text, int fnt, float width, float height, int ln, int realstartx, int framex1, int framey1, int framex2, int framey2)
{
	frame[0] = framex1;
	frame[1] = framey1;
	frame[2] = framex2;
	frame[3] = framey2;
	StartText(text, fnt, width, height, ln, realstartx);
}

void TextLayer(int offstartx, int offstarty)
{
	x = offstartx;
	y = offstarty;
	goffstartx = offstartx;
	g_rpartit = g_rtext->part.begin();
	line = 0;
	i = starti;
	pglyphi = 0;
	nextlb = -1;  //next [i] to break line
	PrepLineBreak();

	for(; i<size && line < startline; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		//if(caret == i)
		//	DrawCaret();

		//DrawGlyph();
		AdvGlyph();
	}

	for(; i<size && i < starti; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		//if(caret == i)
		//	DrawCaret();

		//DrawGlyph();
		AdvGlyph();
	}

	x = offstartx;
	y = offstarty;
	goffstartx = offstartx;
}

void FSub(const char* substr)
{
	int subsize = strlen(substr);

	std::string subsubstr[9];

	int k = 0;
	for(j=0; j<9; j++)
	{
		subsubstr[j] = "";

		for(; k<subsize; k++)
		{
			if(substr[k] == ' ' || substr[k] == '\t')
				break;

			subsubstr[j] += substr[k];
		}

		for(; k<subsize; k++)
		{
			if(substr[k] != ' ' && substr[k] != '\t')
				break;
		}
	}

	unsigned int n = StrToInt(subsubstr[0].c_str());
	Font* f = &g_font[g_currfont];
	Glyph* g = &f->glyph[n];
	g->pixel[0] = StrToInt(subsubstr[1].c_str());
	g->pixel[1] = StrToInt(subsubstr[2].c_str());
	g->texsize[0] = StrToInt(subsubstr[3].c_str());
	g->texsize[1] = StrToInt(subsubstr[4].c_str());
	g->offset[0] = StrToInt(subsubstr[5].c_str());
	g->offset[1] = StrToInt(subsubstr[6].c_str());
	g->origsize[0] = StrToInt(subsubstr[7].c_str());
	g->origsize[1] = StrToInt(subsubstr[8].c_str());
	g->texcoord[0] = (float)g->pixel[0] / f->width;
	g->texcoord[1] = (float)g->pixel[1] / f->height;
	g->texcoord[2] = (float)(g->pixel[0]+g->texsize[0]) / f->width;
	g->texcoord[3] = (float)(g->pixel[1]+g->texsize[1]) / f->height;
}

#if 1
void LoadFont(int index, const char* fontfile)
{
	Font* f = &g_font[index];
	char texfile[128];
	strcpy(texfile, fontfile);
	FindTextureExtension(texfile);

	CHECKGLERROR();

	CreateTex(f->texindex, texfile, ectrue, ecfalse);
	f->width = g_texwidth;
	f->height = g_texheight;

	char fullfontpath[SFH_MAX_PATH+1];
	std::string fontfileext = std::string(fontfile) + ".fnt";
	sprintf(fullfontpath, "%s.fnt", fontfile);
	FullPath(fontfileext.c_str(), fullfontpath);
	FILE* fp = fopen(fullfontpath, "rb");
	if(!fp)
	{
		Log("Error loading font %s\r\n", fontfile);
		Log("Full path: %s\r\n", fullfontpath);
		return;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	char* file = new char[size];
	fread(file, 1, size, fp);
	fclose(fp);

	//NSLog(@"%s", file);

	std::string substr;
	//g_str = file;

	g_currfont = index;

	//skip 2 lines
	for(i=0; i<size; i++)
	{
		if(file[i] == '\n')
			break;
	}
	i++;
	for(; i<size; i++)
	{
		if(file[i] == '\n')
			break;
	}
	i++;

	do
	{
		substr = "";

		for(; i<size; i++)
		{
			if(file[i] == '\n')
				break;

			substr += file[i];
		}

		i++;

		if(substr.length() > 9)
			FSub(substr.c_str());
	}
	while(i<size);

	f->gheight = f->glyph['A'].origsize[1] + 1;

	delete [] file;
	Log("%s.fnt\r\n", fontfile);
}
#else
void LoadFont(int index, const char* fontfile, int sz)
{
	Font* f = &g_font[index];
	f->close();
	char texfile[128];
	strcpy(texfile, fontfile);
	FindTextureExtension(texfile);

	CHECKGLERROR();

	CreateTex(f->texindex, texfile, ectrue, ecfalse);
	f->width = g_texwidth;
	f->height = g_texheight;

	char fullfontpath[SFH_MAX_PATH+1];
	std::string fontfileext = std::string(fontfile) + ".ttf";
	sprintf(fullfontpath, "%s.ttf", fontfile);
	FullPath(fontfileext.c_str(), fullfontpath);
	FILE* fp = fopen(fullfontpath, "rb");
	if(!fp)
	{
		Log("Error loading font %s\r\n", fontfile);
		Log("Full path: %s\r\n", fullfontpath);
		return;
	}

	f->ttf = TTF_OpenFont(fullfontpath, sz);

	f->gheight = TTF_FontHeight(f->ttf);
	


	Log("%s.ttf\r\n", fontfile);
}
#endif

void AddGlyph(Font* f, unsigned int g)
{

}

void CheckGlyph(Font* f, unsigned int g)
{
	Glyph* pg =& f->glyph[g];
	if(!pg->rend)
		AddGlyph(f, g);
}

void CheckGlyphs(int fi, RichText* g)
{
	Font* f = &g_font[fi];

	for(std::list<RichPart>::iterator pit=g->part.begin(); pit!=g->part.end(); ++pit)
	{
		for(unsigned int* git=pit->text.data; *git; ++git)
		{
			CheckGlyph(f, *git);
		}
	}
}

void DrawGlyph(float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
	float vertices[] =
	{
		//posx, posy    texx, texy
		left, top,          texleft, textop,
		right, top,         texright, textop,
		right, bottom,      texright, texbottom,

		right, bottom,      texright, texbottom,
		left, bottom,       texleft, texbottom,
		left, top,          texleft, textop
	};

#ifdef DEBUGLOG
	Log("draw glyph: "<<texleft<<","<<textop<<","<<texright<<","<<texbottom);
	
#endif
    
    Shader* s = g_sh+g_curS;
    
#if 0
    GLuint vao, vbo[2];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*0));
    glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
#endif
	
#ifdef PLATFORM_GL14
	//glVertexPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[0]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[2]);
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[0]);
	glVertexPointer(2 /*3*/, GL_FLOAT, sizeof(float)*4, &vertices[0]);
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[2]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
#if 0
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);
#endif
}

void DrawGlyphF(float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
	//DrawGlyph(left, top, right, bottom, texleft, textop, texright, texbottom);

	//return;
	float newleft = left;
	float newtop = top;
	float newright = right;
	float newbottom = bottom;
	float newtexleft = texleft;
	float newtextop = textop;
	float newtexright = texright;
	float newtexbottom = texbottom;

#if 1
	if(newleft < frame[0])
	{
		newtexleft = texleft+(frame[0]-newleft)*(texright-texleft)/(right-left);
		newleft = frame[0];
	}
	else if(newleft > frame[2])
	{
		return;
		//newtexleft = texleft+(newleft-frame[2])*(texright-texleft)/(right-left);
		newleft = frame[2];
	}

	if(newright < frame[0])
	{
		return;
		//newtexleft = texleft+(frame[0]-newright)*(texright-texleft)/(right-left);
		newright = frame[0];
	}
	else if(newright > frame[2])
	{
		newtexright = texleft+(frame[2]-newleft)*(texright-texleft)/(right-left);
		newright = frame[2];
	}

	if(newtop < frame[1])
	{
		newtextop = textop+(frame[1]-newtop)*(texbottom-textop)/(bottom-top);
		newtop = frame[1];
	}
	else if(newtop > frame[3])
	{
		return;
		//newtextop = textop+(newtop-frame[3])*(texbottom-textop)/(bottom-top);
		newtop = frame[3];
	}

	if(newbottom < frame[1])
	{
		return;
		//newtexbottom = textop+(frame[1]-newbottom)*(texbottom-textop)/(bottom-top);
		newbottom = frame[1];
	}
	else if(newbottom > frame[3])
	{
		newtexbottom = textop+(frame[3]-newtop)*(texbottom-textop)/(bottom-top);
		newbottom = frame[3];
	}
#elif 0
	if(newleft < frame[0])
	{
		newtexleft = (frame[0]-newleft)*(texright-texleft)/(right-left);
		newleft = frame[0];
	}
	else if(newleft > frame[2])
	{
		newtexleft = (frame[2]-newleft)*(texright-texleft)/(right-left);
		newleft = frame[2];
	}

	if(newright < frame[0])
	{
		newtexleft = (frame[0]-newright)*(texright-texleft)/(right-left);
		newright = frame[0];
	}
	else if(newright > frame[2])
	{
		newtexright = (frame[2]-newright)*(texright-texleft)/(right-left);
		newright = frame[2];
	}

	if(newtop < frame[1])
	{
		newtextop = (frame[1]-newtop)*(texbottom-textop)/(bottom-top);
		newtop = frame[1];
	}
	else if(newtop > frame[3])
	{
		newtextop = (frame[3]-newtop)*(texbottom-textop)/(bottom-top);
		newtop = frame[3];
	}

	if(newbottom < frame[1])
	{
		newtexbottom = (frame[1]-newbottom)*(texbottom-textop)/(bottom-top);
		newbottom = frame[1];
	}
	else if(newbottom > frame[3])
	{
		newtexbottom = (frame[3]-newbottom)*(texbottom-textop)/(bottom-top);
		newbottom = frame[3];
	}
#endif

#ifdef DEBUGLOG
	Log("text "<<__FILE__<<" "<<__LINE__);
	
#endif

	float vertices[] =
	{
		//posx, posy    texx, texy
		newleft, newtop,          newtexleft, newtextop,
		newright, newtop,         newtexright, newtextop,
		newright, newbottom,      newtexright, newtexbottom,

		newright, newbottom,      newtexright, newtexbottom,
		newleft, newbottom,       newtexleft, newtexbottom,
		newleft, newtop,          newtexleft, newtextop
	};
    
    Shader* s = g_sh+g_curS;
    
#if 0
    GLuint vao, vbo[2];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*0));
    glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
#endif
	
#ifdef PLATFORM_GL14
	//glVertexPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[0]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[2]);
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[0]);
	glVertexPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[0]);
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float)*4, &vertices[2]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[0]);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
#if 0
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);
#endif
}

void HighlGlyphF(float left, float top, float right, float bottom)
{
	float newleft = left;
	float newtop = top;
	float newright = right;
	float newbottom = bottom;

	if(newleft < frame[0])
		newleft = frame[0];
	else if(newleft > frame[2])
		newleft = frame[2];

	if(newright < frame[0])
		newright = frame[0];
	else if(newright > frame[2])
		newright = frame[2];

	if(newtop < frame[1])
		newtop = frame[1];
	else if(newtop > frame[3])
		newtop = frame[3];

	if(newbottom < frame[1])
		newbottom = frame[1];
	else if(newbottom > frame[3])
		newbottom = frame[3];

	float vertices[] =
	{
		//posx, posy
		newleft, newtop,0,
		newright, newtop,0,
		newright, newbottom,0,

		newright, newbottom,0,
		newleft, newbottom,0,
		newleft, newtop,0
	};
    
    Shader* s = g_sh+g_curS;
    
#if 0
    GLuint vao, vbo[2];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(s->slot[SSLOT_POSITION], 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)(sizeof(float)*0));
    //glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
#endif
	
#ifdef PLATFORM_GL14
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
#endif
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
#if 0
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);
#endif
}

void DrawLine(int caret)
{
	if(caret == 0)
		DrawCaret();
	for(; i<size; i++)
	{
		if(caret == i)
			DrawCaret();

		DrawGlyph();
		AdvGlyph();
	}
	if(caret == size)
		DrawCaret();
}

void DrawLineF(int caret)
{
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(caret == i)
			DrawCaretF();

#ifdef DEBUGLOG
		Log("text "<<__FILE__<<" "<<__LINE__);
		
#endif

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
		DrawCaretF();
}

void DrawLine(int fnt, float startx, float starty, const RichText* text, const float* color, int caret)
{
	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		//glColor4f(color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}

	Py* py = &g_py[g_localP];
	StartText(text, fnt, (float)g_currw*2, (float)g_currh*2, 0, startx);
	UseFontTex();
	TextLayer(startx, starty);
	DrawLine(caret);
}

void DrawShadowedText(int fnt, float startx, float starty, const RichText* text, const float* color, int caret)
{
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0, 0, 0, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.0f, 0.0f, 0.0f, color != NULL ? color[3] : 1);
	//glColor4f(0, 0, 0, 1);
	currcolor[0] = 0;
	currcolor[1] = 0;
	currcolor[2] = 0;
	currcolor[3] = color != NULL ? color[3] : 1;

	Py* py = &g_py[g_localP];
	shadowpass = ectrue;
	StartText(text, fnt, (float)g_currw*2, (float)g_currh*2, 0, startx);
	UseFontTex();
	TextLayer(startx+1, starty);
	DrawLine(caret);
	TextLayer(startx, starty+1);
	DrawLine(caret);
	TextLayer(startx+1, starty+1);
	DrawLine(caret);

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		//glColor4f(color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}
	
	shadowpass = ecfalse;
	TextLayer(startx, starty);
	DrawLine(caret);

	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glColor4f(1, 1, 1, 1);
}

void DrawLineF(int fnt, float startx, float starty, float framex1, float framey1, float framex2, float framey2,  const RichText* text, const float* color, int caret)
{
	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		//glColor4f(color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}

	Py* py = &g_py[g_localP];
	StartTextF(text, fnt, (float)g_currw*2, (float)g_currh*2, 0, startx, framex1, framey1, framex2, framey2);
	UseFontTex();
	TextLayer(startx, starty);
	DrawLineF(caret);
}

void DrawShadowedTextF(int fnt, float startx, float starty, float framex1, float framey1, float framex2, float framey2, const RichText* text, const float* color, int caret)
{
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.0f, 0.0f, 0.0f, color != NULL ? color[3] : 1);
	currcolor[0] = 0;
	currcolor[1] = 0;
	currcolor[2] = 0;
	currcolor[3] = color != NULL ? color[3] : 1;

	Py* py = &g_py[g_localP];
	shadowpass = ectrue;
	StartTextF(text, fnt, (float)g_currw*2, (float)g_currh*2, 0, startx, framex1, framey1, framex2, framey2);

#ifdef DEBUGLOG
	Log("text "<<__FILE__<<" "<<__LINE__);
	
#endif

	UseFontTex();
	TextLayer(startx+1, starty);

#ifdef DEBUGLOG
	Log("text "<<__FILE__<<" "<<__LINE__);
	
#endif

	DrawLineF(caret);
	TextLayer(startx, starty+1);
	DrawLineF(caret);
	TextLayer(startx+1, starty+1);
	DrawLineF(caret);

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		//glColor4f(color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}
	
	shadowpass = ecfalse;
	TextLayer(startx, starty);
	DrawLineF(caret);

	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
}

void HighlightF(int fnt, float startx, float starty, float framex1, float framey1, float framex2, float framey2, const RichText* text, int highlstarti, int highlendi)
{
	Py* py = &g_py[g_localP];
	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)g_currw);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)g_currh);
	glUniform4f(g_sh[SHADER_COLOR2D].slot[SSLOT_COLOR], 1, 1, 1, 0.5f);

	currcolor[0] = 1;
	currcolor[1] = 1;
	currcolor[2] = 1;
	currcolor[3] = 0.5f;

	StartTextF(text, fnt, g_currw*2, g_currh*2, 0, startx, framex1, framey1, framex2, framey2);

	TextLayer(startx, starty);

	for(i=0; i<highlstarti; i++)
		AdvGlyph();

	for(; i<highlendi; i++)
	{
		HighlGlyphF();
		AdvGlyph();
	}

	EndS();
	CHECKGLERROR();
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

void DrawCenterShadText(int fnt, float startx, float starty, const RichText* text, const float* color, int caret)
{
	float linew = 0;
	int len = text->texlen();
	//for(int k=0; k<len; k++)
	//	linew += g_font[g_currfont].glyph[ text[k] ].origsize[0];

	/*
	for(int i=0; i<strlen(label.c_str()); i++)
	{
	length += g_font[font].glyph[label[i]].origsize[0];
	}*/

	Font* f = &g_font[fnt];
	for(std::list<RichPart>::const_iterator p=text->part.begin(); p!=text->part.end(); p++)
	{
		if(p->type == RICH_TEXT)
		{
			for(j=0; j<p->text.length; j++)
			{
				linew += f->glyph[ p->text.data[j] ].origsize[0];
			}
		}
		else if(p->type == RICH_ICON)
		{
			Icon* icon = &g_icon[p->icon];
			float hscale = f->gheight / (float)icon->height;
			linew += (float)icon->width * hscale;
		}
	}

	startx -= linew/2;

	float a = 1;
	if(color != NULL)
		a = color[3];

	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0, 0, 0, a);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.0f, 0.0f, 0.0f, color != NULL ? color[3] : 1);
	currcolor[0] = 0.0f;
	currcolor[1] = 0.0f;
	currcolor[2] = 0.0f;
	currcolor[3] = color != NULL ? color[3] : 1;

	Py* py = &g_py[g_localP];
	shadowpass = ectrue;
	StartText(text, fnt, g_currw*2, g_currh*2, 0, startx);
	UseFontTex();
	TextLayer(startx+1, starty);
	DrawLine(caret);
	TextLayer(startx, starty+1);
	DrawLine(caret);
	TextLayer(startx+1, starty+1);
	DrawLine(caret);

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}
	
	shadowpass = ecfalse;
	TextLayer(startx, starty);
	for(; i<size; i++)
	{
		DrawGlyph();
		AdvGlyph();
	}

	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
}

void DrawBoxShadText(int fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int ln, int caret)
{
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0, 0, 0, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.3f, 0.3f, 0.3f, color ? color[3] : 1);
	currcolor[0] = 0.3f;
	currcolor[1] = 0.3f;
	currcolor[2] = 0.3f;
	currcolor[3] = color != NULL ? color[3] : 1;
	
	shadowpass = ectrue;

	StartText(text, fnt, width, height, ln, startx);
	UseFontTex();
	TextLayer(startx+1, starty);
	if(caret == 0)
		DrawCaret();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaret();

		DrawGlyph();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaret();
	}

	TextLayer(startx, starty+1);
	if(caret == 0)
		DrawCaret();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaret();

		DrawGlyph();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaret();
	}

	TextLayer(startx+1, starty+1);
	if(caret == 0)
		DrawCaret();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaret();

		DrawGlyph();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaret();
	}

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}
	
	shadowpass = ecfalse;

	TextLayer(startx, starty);
	if(caret == 0)
		DrawCaret();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaret();

		DrawGlyph();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaret();
	}
}

void DrawBoxShadTextF(int fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int ln, int caret, float framex1, float framey1, float framex2, float framey2)
{
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0, 0, 0, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.3f, 0.3f, 0.3f, color ? color[3] : 1);
	currcolor[0] = 0.3f;
	currcolor[1] = 0.3f;
	currcolor[2] = 0.3f;
	currcolor[3] = color != NULL ? color[3] : 1;

	shadowpass = ectrue;

	StartTextF(text, fnt, width, height, ln, startx, framex1, framey1, framex2, framey2);
	UseFontTex();
	TextLayer(startx+1, starty);
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaretF();

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaretF();
	}

	TextLayer(startx, starty+1);
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaretF();

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaretF();
	}

	TextLayer(startx+1, starty+1);
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaretF();

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaretF();
	}

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}

	shadowpass = ecfalse;

	TextLayer(startx, starty);
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaretF();

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaretF();
	}
}

void DrawBoxTextF(int fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int ln, int caret, float framex1, float framey1, float framex2, float framey2)
{
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0, 0, 0, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.3f, 0.3f, 0.3f, color ? color[3] : 1);
	currcolor[0] = 0.3f;
	currcolor[1] = 0.3f;
	currcolor[2] = 0.3f;
	currcolor[3] = color != NULL ? color[3] : 1;

	if(color == NULL)
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		for(int c=0; c<4; c++) currcolor[c] = 1;
	}
	else
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);
		for(int c=0; c<4; c++) currcolor[c] = color[c];
	}

	StartTextF(text, fnt, width, height, ln, startx, framex1, framey1, framex2, framey2);
	UseFontTex();
	TextLayer(startx, starty);
	if(caret == 0)
		DrawCaretF();
	for(; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(caret == i)
			DrawCaretF();

		DrawGlyphF();
		AdvGlyph();
	}
	if(caret == size)
	{
		//if(g_str[size-1] == '\n')
		//	BreakLine();
		DrawCaretF();
	}
}


int CountLines(const RichText* text, int fnt, float startx, float starty, float width, float height)
{
	StartText(text, fnt, width, height, 0, startx);
	TextLayer(startx, starty);

	for(i=0; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		AdvGlyph();
	}
	//if(g_str[size-1] == '\n')
	//	BreakLine();

	return line+1;	//add 1 since line starts count from 0
}

int GetLineStart(const RichText* text, int fnt, float startx, float starty, float width, float height, int getline)
{
	StartText(text, fnt, width, height, 0, startx);
	TextLayer(startx, starty);

	for(i=0; i<size; i++)
	{
		if(i == nextlb)
			NextLineBreak();

		if(line == getline)
			return i;

		AdvGlyph();
	}
	//if(g_str[size-1] == '\n')
	//	BreakLine();
	
	if(line == getline)
		return i;

	return i;	//return last glyph anyway
}

int EndX(const RichText* text, int lastc, int fnt, float startx, float starty)
{
	//if(g_netmode == NETM_CLIENT && stricmp(text->rawstr().c_str(), "Join")==0) debugtest = ectrue;

	Py* py = &g_py[g_localP];
	//StartText(text, fnt, g_currw*100, g_currh*100, 0, startx);
	StartText(text, fnt, SHRT_MAX, SHRT_MAX, 0, startx);
	TextLayer(startx, starty);

	//Log("size = "<<size);
	//Log("lastc = "<<lastc);
	//Log("g_localP= "<<g_localP<<" g_currw*100 = "<<g_currw*100<<"   g_width="<<g_width<<" ");

	int highx = (int)startx;

	for(i=0; i<size && i<lastc; i++)
	{
		if(i == nextlb)
		{
#if 0
			if(g_netmode == NETM_CLIENT)
			{
				char msg[128];
				sprintf(msg, "line break at i=%d, x=%d", i, (int)x);
				InfoMess("lb", msg);
			}
#endif
			NextLineBreak();
		}

		//Log("g_str[i] = "<<g_str[i]);
		AdvGlyph();

		if(x > highx)
			highx = x;
	}

	//debugtest = ecfalse;

	return highx;
}

int MatchGlyphF(const RichText* text, int fnt, int matchx, float startx, float starty, float framex1, float framey1, float framex2, float framey2)
{
	Py* py = &g_py[g_localP];

	int lastclose = 0;

	//StartTextF(text, fnt, g_currw*2, g_currh*2, 0, startx, framex1, framey1, framex2, framey2);
	StartTextF(text, fnt, g_width*2, g_height*2, 0, startx, framex1, framey1, framex2, framey2);
	TextLayer(startx, starty);

	if(x >= matchx || size <= 0)
		return lastclose;

	int lastx = x;

	for(i=0; i<size && x <= framex2; i++)
	{
		AdvGlyph();

		lastclose = i;

		if((float)(x+lastx)/2.0f >= matchx)
			return lastclose;

		lastx = x;
	}

	return lastclose+1;
}

int TextWidth(int fnt, const RichText* text)
{
	return EndX(text, text->texlen(), fnt, 0, 0);
}

void LoadFonts()
{
	LoadFont(FONT_EUROSTILE32, "fonts/euphemia32s");
	LoadFont(FONT_MSUIGOTHIC16, "fonts/kalinga16s");
	LoadFont(FONT_MSUIGOTHIC10, "fonts/msuigothic10");
	LoadFont(FONT_SMALLFONTS8, "fonts/smallfonts8");
	LoadFont(FONT_SMALLFONTS10, "fonts/smallfonts10");
	LoadFont(FONT_ARIAL10, "fonts/arial10s");
	LoadFont(FONT_GULIM32, "fonts/gulim32");
	LoadFont(FONT_EUROSTILE16, "fonts/eurostile16");
//#ifdef PLATFORM_MOBILE
#if 0
	LoadFont(FONT_CALIBRILIGHT16, "fonts/kalinga16s");
#else
	LoadFont(FONT_CALIBRILIGHT16, "fonts/kalinga16s");
#endif
	//LoadFont(FONT_CALIBRILIGHT16, "fonts/gulim16");
	LoadFont(FONT_TERMINAL10, "fonts/terminal10");
	//LoadFont(FONT_TERMINAL11, "fonts/terminal11");
}
