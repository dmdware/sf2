#ifndef APPMAIN_H
#define APPMAIN_H

#define VERSION				0

#define CONFIGFILE			"config.ini"
#define TITLE				"T"

#define APPMODE_LOGO		0
#define APPMODE_INTRO		1
#define APPMODE_LOADING		2
#define APPMODE_RELOADING	3
#define APPMODE_MENU		4
#define APPMODE_PLAY		5
#define APPMODE_PAUSE		6
#define APPMODE_EDITOR		7
#define APPMODE_JOINING		8
extern char g_appmode;

#define VIEWMODE_FIRST		0
#define VIEWMODE_THIRD		1
#define VIEWMODES			2
extern char g_viewmode;

#endif
