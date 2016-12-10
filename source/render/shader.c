



#include "../utils.h"
#include "shader.h"
#include "../platform.h"
#include "../window.h"
#include "../sim/player.h"
#include "../debug.h"
#include "../utils.h"

Shader g_sh[SHADERS];
signed char g_curS = 0;

const char *SHTEXT [SHADERS][2]
=
{
	/* SHADER_ORTHO */
	{
""\		
"#version 120"\
""\
"attribute vec4 position;"\
""\
"uniform float width;"\
"uniform float height;"\
""\
"attribute vec2 texCoordIn0;"\
"varying vec2 texCoordOut0;"\
""\
"void main(void)"\
"{"\
"	gl_Position = vec4(gl_Vertex.x * 2.0 / width - 1.0,"\
"		gl_Vertex.y * -2.0 / height + 1.0,"\
"		gl_Vertex.z, "\
"		1.0);"\
"                    "\
"	gl_TexCoord[0] = gl_MultiTexCoord0;"\
"}"\
"",

""\
"#version 120"\
""\
"uniform vec4 color;"\
""\
"varying vec2 texCoordOut0;"\
"uniform sampler2D texture0;"\
""\
"void main(void)"\
"{"\
"	gl_FragColor = color * texture2D(texture0, gl_TexCoord[0].xy);"\
"}"\
""
	}
};

void Sh_init(Shader* s)
{
	s->program = NULL;
	s->vertshader = NULL;
	s->fragshader = NULL;
}

void Sh_free(Shader* s)
{
	if(s->vertshader)
	{
		glDetachShader(s->program, s->vertshader);
		glDeleteShader(s->vertshader);
		s->vertshader = NULL;
	}

	if(s->fragshader)
	{
		glDetachShader(s->program, s->fragshader);
		glDeleteShader(s->fragshader);
		s->fragshader = NULL;
	}

	if(s->program)
	{
		glDeleteProgram(s->program);
		s->program = NULL;
	}
}

GLint Sh_gu(Shader* s, const char* strVariable)
{
	if(!s->program)
		return -1;
	return glGetUniformLocation(s->program, strVariable);
}

GLint Sh_ga(Shader* s, const char* strVariable)
{
	if(!s->program)
		return -1;
	return glGetAttribLocation(s->program, strVariable);
}

void Sh_mu(Shader* s, int slot, const char* variable)
{
	s->slot[slot] = Sh_gu(s, variable);
}

void Sh_ma(Shader* s, int slot, const char* variable)
{
	s->slot[slot] = Sh_ga(s, variable);
}

void GLVer(int* major, int* minor)
{
	/* for all versions */
	char* var;

	char vermaj[6];
	char vermin[6];

	int i;
	int j;

	ver = (char*)glGetString(GL_VERSION); /* ver = "3.2.0" */

	for(i=0; i<6; i++)
	{
		if(ver[i] != '.')
			vermaj[i] = ver[i];
		else
		{
			vermaj[i] = '\0';
			i++;
			break;
		}
	}

	for(j=0; j<6; j++, i++)
	{
		if(ver[i] != '.')
			vermin[j] = ver[i];
		else
		{
			vermin[j] = '\0';
			i++;
			break;
		}
	}

	*major = StrToInt(vermaj);
	*minor = StrToInt(vermin);

	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
}

void InitGLSL()
{
	int major, minor;
	GLenum glewError;
	char* glexts;

#ifndef PLATFORM_MAC
	glewExperimental = GL_TRUE;
	glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		ErrMess("Error initializing GLEW!", (const char*)glewGetErrorString( glewError ));
		return;
	}
#endif

#ifdef __glew_h__
	if( !GLEW_VERSION_1_4 )
	{
		ErrMess("Error", "OpenGL 1.4 not supported!\n" );
		goto quit;
	}
#endif

#ifndef PLATFORM_MOBILE
	/* glGetString deprecated after 3.0 */
	glexts = (char*)glGetString(GL_EXTENSIONS);

	fprintf(g_applog, glexts);

#if !defined( PLATFORM_MAC ) && !defined( PLATFORM_IOS )
#ifdef GLDEBUG
	if(!strstr(glexts, "GL_ARB_debug_output"))
	{
		fprintf(g_applog, "GL_ARB_debug_output extension not supported\r\n");
	}
	else
	{
		glDebugMessageCallback(&GLMessageHandler, 0);

		CHECKGLERROR();
	}
#endif
#endif

	if(!strstr(glexts, "GL_ARB_shader_objects"))
	{
		ErrMess("Error", "GL_ARB_shader_objects extension not supported!");
		goto quit;
	}

	if(!strstr(glexts, "GL_ARB_shading_language_100"))
	{
		ErrMess("Error", "GL_ARB_shading_language_100 extension not supported!");
		goto quit;
	}
#endif

	GLVer(&major, &minor);

#ifndef PLATFORM_MOBILE
	if(major < 1 || ( major == 1 && minor < 4 ))
	{
		ErrMess("Error", "OpenGL 1.4 is not supported!");
		goto quit;
	}
#endif

	LoadSh(SH_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag", ectrue, ecfalse);

quit:
	g_quit = ectrue;
}

void LoadSh(int shader, const char* filev, const char* filef, ecbool hastexc, ecbool hasnorm)
{
	Shader* s;
	char *strv, *strf;
	char vfull[DMD_MAX_PATH+1];
	char ffull[DMD_MAX_PATH+1];
	GLint loglen;
	GLchar* log;

	s = g_sh+shader;

	if(s->vertshader || s->fragshader || s->program)
		Sh_free(s);

	s->hastexc = hastexc;
	s->hasnorm = hasnorm;
	s->vertshader = glCreateShader(GL_VERTEX_SHADER);
	s->fragshader = glCreateShader(GL_FRAGMENT_SHADER);

	FullPath(filev, vfull);
	FullPath(filef, ffull);

	strv = NULL;
	strf = NULL;

	LoadTextFile(&strv, vfull);
	LoadTextFile(&strf, ffull);

	if(!strv)
		OUTOFMEM();

	if(!strf)
		OUTOFMEM();

	glShaderSource(s->vertshader, 1, szVShader, NULL);
	glShaderSource(s->fragshader, 1, szFShader, NULL);

	glCompileShader(s->vertshader);

	free(strv);
	free(strf);

	glGetShaderiv(s->vertshader, GL_INFO_LOG_LENGTH, &loglen);
	if(loglen > 0)
	{
		log = (GLchar*)malloc(loglen);

		if(!log)
		{
			OUTOFMEM();
			return;
		}

		glGetShaderInfofprintf(g_applog, s->vertshader, loglen, &loglen, log);
		fprintf(g_applog, "Shader %s compile log: %s\r\n", filev, log);
		free(log);
	}

	glCompileShader(s->fragshader);
	glGetShaderiv(s->fragshader, GL_INFO_LOG_LENGTH, &loglen);
	if(loglen > 0)
	{
		log = (GLchar*)malloc(loglen);

		if(!log)
		{
			OUTOFMEM();
			return;
		}

		glGetShaderInfofprintf(g_applog, s->fragshader, loglen, &loglen, log);
		fprintf(g_applog, "Shader %s compile log: %s\r\n", filef, log);
		free(log);
	}

	s->program = glCreateProgram();
	glAttachShader(s->program, s->vertshader);
	glAttachShader(s->program, s->fragshader);
	glLinkProgram(s->program);

	fprintf(g_applog, "Program %s / %s :", filev, filef);

	glGetProgramiv(s->program, GL_INFO_LOG_LENGTH, &loglen);
	if (loglen > 0)
	{
		log = (GLchar*)malloc(loglen);
		glGetProgramInfofprintf(g_applog, s->program, loglen, &loglen, log);
		fprintf(g_applog, "Program link log: %s\r\n", log);
		free(log);
	}

	GLint status;
	glGetProgramiv(s->program, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		fprintf(g_applog, "link status 0\r\n");
	}
	else
	{
		fprintf(g_applog, "link status ok, program=%d\r\n", s->program);
	}

	fprintf(g_applog, "\r\n");

	Sh_ma(s, SSLOT_POSITION, "position");
	Sh_ma(s, SSLOT_NORMAL, "normalin");
	Sh_ma(s, SSLOT_TEXCOORD0, "texcoordin0");
	Sh_mu(s, SSLOT_TEXTURE0, "texture0");
	Sh_mu(s, SSLOT_COLOR, "color");
	Sh_mu(s, SSLOT_WIDTH, "width");
	Sh_mu(s, SSLOT_HEIGHT, "height");
}

void UseS(int shader)
{
	Shader* s;

	CHECKGLERROR();
	g_curS = shader;

	s = g_sh+g_curS;

	glUseProgram(s->program);
	CHECKGLERROR();

#ifdef PLATFORM_GL14
	/* opengl 1.4 way */
	glEnableClientState(GL_VERTEX_ARRAY);
	if(s->hastexc)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(s->hasnorm)	glEnableClientState(GL_NORMAL_ARRAY);
#endif
}

void EndS()
{
	Shader* s;
	CHECKGLERROR();

	if(g_curS < 0)
		return;

	s = g_sh+g_curS;

#ifdef PLATFORM_GL14
	//opengl 1.4 way
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#endif

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	g_curS = -1;
}
