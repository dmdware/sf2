











#include "../platform.h"
#include "../window.h"
#include "../utils.h"
#include "../texture.h"
#include "../sim/player.h"
#include "../debug.h"

void SaveScreenshot()
{
	Py* py = &g_py[g_localP];

	LoadedTex screenshot;
	screenshot.channels = 3;
	screenshot.sizex = g_width;
	screenshot.sizey = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

	if(!screenshot.data)
	{
		OUTOFMEM();
		return;
	}

	//memset(screenshot.data, 0, g_width * g_height * 3);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	CHECKGLERROR();
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);
	CHECKGLERROR();

	FlipImage(&screenshot);

	char relative[256];
	std::string datetime = FileDateTime();
	//sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	sprintf(relative, "screenshots/%s.png", datetime.c_str());
	char fullpath[SFH_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	Log("Writing screenshot %s\r\n", fullpath);
	

	//SaveJPEG(fullpath, &screenshot, 0.9f);
	SavePNG2(fullpath, &screenshot);

	//free(screenshot.data);
}
