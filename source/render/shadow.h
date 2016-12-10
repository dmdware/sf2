

#ifndef SHADOW_H
#define SHADOW_H

#include "../math/vec3f.h"

#define DEPTH_SIZE		2048	//512	//4096
//#define DEPTH_SIZE		128

extern unsigned int g_depth;
extern Vec3f g_lightOff;
extern Vec3f g_lightPos;
extern Vec3f g_lightEye;
extern Vec3f g_lightUp;
extern ecbool g_shadowpass;

struct Matrix;

extern Matrix g_lightproj;
extern Matrix g_lightview;
extern Matrix g_caminvmv;  //camera inverse modelview
extern Matrix g_lightmat;
extern Matrix g_cammodelview;
extern Matrix g_camproj;
extern Matrix g_cammvp;
extern Matrix g_camview;

void Transpose(Matrix mat, Matrix& transpMat);
ecbool Inverse2(Matrix mat, Matrix& invMat);

void InitShadows();
void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawscenedepthfunc)());
void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview, void (*drawscenefunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]));
void UseShadow(int shader, Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
void RenderToDepthMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawscenedepthfunc)());
void RenderToElevMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, void (*drawsceneelevfunc)());

#endif
