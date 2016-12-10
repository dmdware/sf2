

#include "../utils.h"
#include "shadow.h"
#include "shader.h"
#include "../math/3dmath.h"
#include "model2.h"
#include "particle.h"
#include "projectile.h"
#include "../debug.h"
#include "billboard.h"
#include "../platform.h"
#include "../window.h"
#include "../math/vec3f.h"
#include "../math/matrix.h"
#include "../sim/sim.h"
#include "../sim/map.h"
#include "../math/camera.h"
#include "../gui/draw2d.h"

unsigned int g_depth;
unsigned int g_rbdepth;
unsigned int g_fbdepth;
ecbool g_shadowpass = ectrue;

//1000000.0f/300 = 3333.3333333333333333333333333333 cm =

Vec3f g_lightOff(-MAX_DISTANCE/500, MAX_DISTANCE/300, MAX_DISTANCE/400);
Vec3f g_lightPos;	//(-MAX_DISTANCE/2, MAX_DISTANCE/5, MAX_DISTANCE/3);
Vec3f g_lightEye;	//(-MAX_DISTANCE/2+1.0f/2.0f, MAX_DISTANCE/3-1.0f/3.0f, MAX_DISTANCE/3-1.0f/3.0f);
Vec3f g_lightUp(0,1,0);

Matrix g_lightproj;
Matrix g_lightview;
Matrix g_caminvmv;  //camera inverse modelview
Matrix g_lightmat;
Matrix g_cammodelview;
Matrix g_camproj;
Matrix g_cammvp;
Matrix g_camview;

Vec3f g_viewInter;

void InitShadows()
{
#if 0	//OpenGL 3.0 way
	CHECKGLERROR();

	glGenTextures(1, &g_depth);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	CHECKGLERROR();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, DEPTH_SIZE, DEPTH_SIZE, 0, GL_RGBA, GL_UNSIGNED_SHORT, 0);


	CHECKGLERROR();

	glBindTexture(GL_TEXTURE_2D, 0);

	CHECKGLERROR();

	glGenRenderbuffersEXT(1, &g_rbdepth);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_rbdepth);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, DEPTH_SIZE, DEPTH_SIZE);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	CHECKGLERROR();

	glGenFramebuffersEXT(1, &g_fbdepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbdepth);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_depth, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_rbdepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	CHECKGLERROR();
#else	//OpenGL 1.4 way
    glGenTextures(1, &g_depth);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    GLfloat v_bc[] = {1.0f,1.0f,1.0f,1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, v_bc);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT32, // tried 16 and 24
                 //GL_RGBA,
                 DEPTH_SIZE, DEPTH_SIZE,
                 0,
                 GL_DEPTH_COMPONENT,
                 //GL_RGBA,
                 GL_UNSIGNED_BYTE, // tried GL_FLOAT and GL_UNSIGNED_BYTE
                 NULL);

	//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->width, pImage->height, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &g_fbdepth);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, g_fbdepth);
    glDrawBuffer(GL_NONE); // No color buffer is drawn
    glReadBuffer(GL_NONE);

    //-------------------------
    //Attach depth texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D, g_depth, 0/*mipmap level*/);


    //-------------------------
    //Does the GPU support current FBO configuration?
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
    	ErrMess("Error", "GPU doesn't support required FBO configuration.");
        return;
    }

    glClearColor(0, 0, 0, 1);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
#endif
}

void inverse(Matrix* dstm, Matrix& srcm)
{
	float dst[16];
	const float* src = srcm.m;

	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0;

	dstm->set(dst);
}

void inverse(float dst[16], float src[16])
{
	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0;
}

ecbool gluInvertMatrix(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return ecfalse;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return ectrue;
}

ecbool Inverse2(Matrix mat, Matrix& invMat)
{
    double inv[16], det;
    int i;

	const float* m = mat.m;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return ecfalse;

    det = 1.0 / det;

	float invOut[16];
    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

	invMat.set(invOut);

    return ectrue;
}

void Transpose(Matrix mat, Matrix& transpMat)
{
	const float* m = mat.m;
	float transp[16];

	int j;
	int index_1;
	int index_2;

	for (int i = 0; i < 4; i++)
	{
		for (j = 0; j < 4;  j++)
		{
			index_1 = i * 4 + j;
			index_2 = j * 4 + i;
			transp[index_1] = m[index_2];
		}
	}

	transpMat.set(transp);
}


void (*DrawSceneDepthFunc)() = NULL;
void (*DrawSceneFunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]) = NULL;

void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawscenedepthfunc)())
{
    g_camproj = projection;
    g_camview = viewmat;
    //g_cammodelview = modelview;

    Matrix oldproj = g_camproj;
    Matrix oldview = g_camview;
    Matrix oldmvp = g_cammvp;

	glDisable(GL_CULL_FACE);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

#if 0	//OpenGL 3.0 way

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbdepth);
#else	//OpenGL 1.4 way
	GLint oldfb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbdepth);
    glDrawBuffers(0, NULL);
#endif

	glViewport(0, 0, DEPTH_SIZE, DEPTH_SIZE);

#ifdef DEBUG
	CHECKGLERROR();
	LastNum(__FILE__, __LINE__);
#endif

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0, 500.0);

	g_lightPos = focus + g_lightOff/300;
	g_lightEye = focus;

	//g_lightEye = Vec3f(0,0,0);
	//g_lightPos = g_lightEye + g_lightOff;

	//g_lightproj = PerspProj(90.0, 1.0, 30.0, 10000.0);
	g_lightproj = OrthoProj(-PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, -PROJ_RIGHT*2/g_zoom, MIN_DISTANCE, MAX_DISTANCE/300);
	g_lightview = LookAt(g_lightPos.x, g_lightPos.y, g_lightPos.z,
		g_lightEye.x, g_lightEye.y, g_lightEye.z,
		g_lightUp.x, g_lightUp.y, g_lightUp.z);

#ifdef DEBUG
	CHECKGLERROR();
	LastNum(__FILE__, __LINE__);
#endif

#if 1

	UseS(SHADER_DEPTH);
	glUniformMatrix4fv(g_sh[SHADER_DEPTH].slot[SSLOT_PROJECTION], 1, 0, g_lightproj.m);
	glUniformMatrix4fv(g_sh[SHADER_DEPTH].slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
	glUniformMatrix4fv(g_sh[SHADER_DEPTH].slot[SSLOT_VIEWMAT], 1, 0, g_lightview.m);
	//glUniform4f(g_sh[SHADER_MODEL].slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_TEXCOORD0]);

#endif

	g_camproj = g_lightproj;
	g_camview = g_lightview;

	Matrix mvp;
	mvp.set(g_lightproj.m);
	mvp.postmult2(g_lightview);	//prev
	//mvp.postmult(g_lightview);
	glUniformMatrix4fv(g_sh[SHADER_DEPTH].slot[SSLOT_MVP], 1, 0, mvp.m);

#ifdef DEBUG
	CHECKGLERROR();
#endif

#if 1
    if(g_shadowpass)
    {
        if(drawscenedepthfunc != NULL)
            drawscenedepthfunc();
    }

	//TurnOffShader();
	EndS();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, oldfb);

#endif
	//glViewport(0, 0, g_width, g_height);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glEnable(GL_CULL_FACE);

	g_camproj = oldproj;
	g_camview = oldview;
	g_cammvp = oldmvp;
}

void RenderToDepthMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawscenedepthfunc)())
{
    g_camproj = projection;
    g_camview = viewmat;
    //g_cammodelview = modelview;

    Matrix oldproj = g_camproj;
    Matrix oldview = g_camview;
    Matrix oldmvp = g_cammvp;

	//glDisable(GL_CULL_FACE);
	
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);

	g_lightPos = focus + g_lightOff/300;
	g_lightEye = focus;

	//g_lightEye = Vec3f(0,0,0);
	//g_lightPos = g_lightEye + g_lightOff;

	/*
	float aspect = fabsf((float)g_width / (float)g_height);

	//g_lightproj = PerspProj(90.0, 1.0, 30.0, 10000.0);
	//g_lightproj = OrthoProj(-PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, -PROJ_RIGHT*2/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	g_lightproj = OrthoProj(-PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, -PROJ_RIGHT*2/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	g_lightview = LookAt(g_lightPos.x, g_lightPos.y, g_lightPos.z,
		g_lightEye.x, g_lightEye.y, g_lightEye.z,
		g_lightUp.x, g_lightUp.y, g_lightUp.z);
		*/

	g_lightproj = g_camproj;
	g_lightview = g_camview;

#ifdef DEBUG
	CHECKGLERROR();
	LastNum(__FILE__, __LINE__);
#endif

#if 1

	UseS(SHADER_DEPTHRGBA);
	Shader* s = g_sh+g_curS;

	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_lightproj.m);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_lightview.m);
	glUniform1f(s->slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->slot[SSLOT_MAXD], MAX_DISTANCE);
	//glUniform1f(s->slot[SSLOT_BASEDEPTH], SPEDDEPTH);
	//glUniform4f(g_sh[SHADER_MODEL].slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_TEXCOORD0]);

#endif

	g_camproj = g_lightproj;
	g_camview = g_lightview;

	Matrix mvp;
	mvp.set(g_lightproj.m);
	mvp.postmult2(g_lightview);	//prev
	//mvp.postmult(g_lightview);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);

#ifdef DEBUG
	CHECKGLERROR();
#endif

    if(drawscenedepthfunc != NULL)
           drawscenedepthfunc();

	//TurnOffShader();
	EndS();

	//glViewport(0, 0, g_width, g_height);
	//glEnable(GL_CULL_FACE);

	g_camproj = oldproj;
	g_camview = oldview;
	g_cammvp = oldmvp;
}


void RenderToElevMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawsceneelevfunc)())
{
    g_camproj = projection;
    g_camview = viewmat;
    //g_cammodelview = modelview;

    Matrix oldproj = g_camproj;
    Matrix oldview = g_camview;
    Matrix oldmvp = g_cammvp;

	//glDisable(GL_CULL_FACE);
	
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);

	g_lightPos = focus + g_lightOff/300;
	g_lightEye = focus;

	//g_lightEye = Vec3f(0,0,0);
	//g_lightPos = g_lightEye + g_lightOff;

	/*
	float aspect = fabsf((float)g_width / (float)g_height);

	//g_lightproj = PerspProj(90.0, 1.0, 30.0, 10000.0);
	//g_lightproj = OrthoProj(-PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, -PROJ_RIGHT*2/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	g_lightproj = OrthoProj(-PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, PROJ_RIGHT*2/g_zoom, -PROJ_RIGHT*2/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	g_lightview = LookAt(g_lightPos.x, g_lightPos.y, g_lightPos.z,
		g_lightEye.x, g_lightEye.y, g_lightEye.z,
		g_lightUp.x, g_lightUp.y, g_lightUp.z);
		*/

	g_lightproj = g_camproj;
	g_lightview = g_camview;

#ifdef DEBUG
	CHECKGLERROR();
	LastNum(__FILE__, __LINE__);
#endif

#if 1

	UseS(SHADER_ELEVRGBA);
	Shader* s = g_sh+g_curS;

	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_lightproj.m);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_lightview.m);
	glUniform1f(s->slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->slot[SSLOT_MAXD], MAX_DISTANCE);
	//glUniform1f(s->slot[SSLOT_BASEDEPTH], SPEDDEPTH);
	//glUniform4f(g_sh[SHADER_MODEL].slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_sh[SHADER_DEPTH].slot[SSLOT_TEXCOORD0]);

#endif

	g_camproj = g_lightproj;
	g_camview = g_lightview;

	Matrix mvp;
	mvp.set(g_lightproj.m);
	mvp.postmult2(g_lightview);	//prev
	//mvp.postmult(g_lightview);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);

#ifdef DEBUG
	CHECKGLERROR();
#endif

    if(drawsceneelevfunc != NULL)
           drawsceneelevfunc();

	//TurnOffShader();
	EndS();

	//glViewport(0, 0, g_width, g_height);
	//glEnable(GL_CULL_FACE);

	g_camproj = oldproj;
	g_camview = oldview;
	g_cammvp = oldmvp;
}


void UseShadow(int shader, Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
	UseS(shader);
	Shader* s = g_sh+g_curS;
	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection.m);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat.m);
	//glUniformMatrix4fvARB(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m);
	//glUniformMatrix4fvARB(s->slot[SSLOT_INVMODLVIEWMAT], 1, 0, modelviewinv.m);
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD1]);
	//glEnableVertexAttribArray(s->slot[SSLOT_NORMAL]);

	//glUniformMatrix4fvARB(s->slot[SSLOT_LIGHTMATRIX], 1, ecfalse, g_lightmat);
	glUniformMatrix4fv(s->slot[SSLOT_LIGHTMATRIX], 1, ecfalse, g_lightmat.m);

	glUniform3f(s->slot[SSLOT_LIGHTPOS], mvLightPos[0], mvLightPos[1], mvLightPos[2]);
	glUniform3f(s->slot[SSLOT_SUNDIRECTION], lightDir[0], lightDir[1], lightDir[2]);
	glUniform1f(s->slot[SSLOT_MAXELEV], g_maxelev);
}

void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview, void (*drawscenefunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]))
{

#if 0
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbdepth);
    glDrawBuffersARB(0, NULL);
	glViewport(0, 0, DEPTH_SIZE, DEPTH_SIZE);
#endif

	//return;
	//glViewport(0, 0, g_width, g_height);
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//g_camproj = PerspProj(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//g_cam.Look();

	//glGetFloatv(GL_PROJECTION_MATRIX, g_camproj);
	g_camproj = projection;
	//glGetFloatv(GL_MODELVIEW_MATRIX, g_cammodelview);
	g_cammodelview = modelview;

	// Do non-shadowed drawing here
	//DrawSkyBox(g_cam.LookPos());

	inverse(&g_caminvmv, modelview);

	float trans[] = { 0.5f, 0.5f, 0.5f };
	g_lightmat.reset();
	g_lightmat.translation(trans);
	float scalef[] = { 0.5f, 0.5f, 0.5f };
	Matrix scalem;
	scalem.scale(scalef);
#if 0	//correct results when glsl constructs mvp in depth shader, though looks like perspective projection.
	g_lightmat.postmult(scalem);
	g_lightmat.postmult(g_lightproj);
	g_lightmat.postmult(g_lightview);
#else	//correct when setting gl_Position.w=1 or constructing mvp on CPU side using postmult2. looks like orthographic projection, as it should.
	g_lightmat.postmult2(scalem);
	g_lightmat.postmult2(g_lightproj);
	g_lightmat.postmult2(g_lightview);
#endif
	//g_lightmat.postmult(g_caminvmv);

	Matrix modelviewinv;
	Inverse2(modelview, modelviewinv);
	Transpose(modelviewinv, modelviewinv);

	const float* mv = g_cammodelview.m;
	float mvLightPos[3];
	mvLightPos[0] = mv[0] * g_lightPos.x + mv[4] * g_lightPos.y + mv[8] * g_lightPos.z + mv[12];
	mvLightPos[1] = mv[1] * g_lightPos.x + mv[5] * g_lightPos.y + mv[9] * g_lightPos.z + mv[13];
	mvLightPos[2] = mv[2] * g_lightPos.x + mv[6] * g_lightPos.y + mv[10] * g_lightPos.z + mv[14];

	float lightDir[3];
	//lightDir[0] = g_lightEye.x - g_lightPos.x;
	//lightDir[1] = g_lightEye.y - g_lightPos.y;
	//lightDir[2] = g_lightEye.z - g_lightPos.z;
	lightDir[0] = g_lightPos.x - g_lightEye.x;
	lightDir[1] = g_lightPos.y - g_lightEye.y;
	lightDir[2] = g_lightPos.z - g_lightEye.z;

	if(drawscenefunc != NULL)
		drawscenefunc(projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
		//DrawSceneFunc(projection, viewmat, modelmat, modelviewinv, (float*)&g_lightPos, lightDir);
#if 0
	//TurnOffShader();
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	/*
	if(g_appmode == APPMODE_EDITOR)
	{
		UseS(COLOR3D);
		glUniformMatrix4fvARB(g_sh[SHADER_COLOR3D].slot[SSLOT_PROJECTION], 1, 0, projection.m);
		glUniformMatrix4fvARB(g_sh[SHADER_COLOR3D].slot[SSLOT_MODELMAT], 1, 0, modelmat.m);
		glUniformMatrix4fvARB(g_sh[SHADER_COLOR3D].slot[SSLOT_VIEWMAT], 1, 0, viewmat.m);
		glUniform4f(g_sh[SHADER_COLOR3D].slot[SSLOT_COLOR], 0, 1, 0, 1);
		glEnableVertexAttribArray(g_sh[SHADER_COLOR3D].slot[SSLOT_POSITION]);
		glEnableVertexAttribArray(g_sh[SHADER_COLOR3D].slot[SSLOT_NORMAL]);
		DrawTileSq();
	}*/

	//TurnOffShader();
	//g_cam.Look();
#endif

#if 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
#endif
}


