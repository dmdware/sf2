










#ifndef VIEWPORTW_H
#define VIEWPORTW_H

#include "../widget.h"

struct Viewport : public Widget
{
public:
	void (*drawfunc)(int p, int x, int y, int w, int h);
	ecbool (*ldownfunc)(int p, int relx, int rely, int w, int h);
	ecbool (*lupfunc)(int p, int relx, int rely, int w, int h);
	ecbool (*mousemovefunc)(int p, int relx, int rely, int w, int h);
	ecbool (*rdownfunc)(int p, int relx, int rely, int w, int h);
	ecbool (*rupfunc)(int p, int relx, int rely, int w, int h);
	ecbool (*mousewfunc)(int p, int d);
	ecbool (*mdownfunc)(int p, int relx, int rely, int w, int h);
	ecbool (*mupfunc)(int p, int relx, int rely, int w, int h);

	Viewport();

	Viewport(Widget* parent, const char* n, void (*reframef)(Widget* w),
	          void (*drawf)(int p, int x, int y, int w, int h),
	          ecbool (*ldownf)(int p, int relx, int rely, int w, int h),
			  ecbool (*lupf)(int p, int relx, int rely, int w, int h),
			  ecbool (*mousemovef)(int p, int relx, int rely, int w, int h),
			  ecbool (*rdownf)(int p, int relx, int rely, int w, int h),
			  ecbool (*rupf)(int p, int relx, int rely, int w, int h),
			  ecbool (*mousewf)(int p, int d),
			  ecbool (*mdownf)(int p, int relx, int rely, int w, int h),
			  ecbool (*mupf)(int p, int relx, int rely, int w, int h),
	          int parm);
	void inev(InEv* ie);
	void draw();
};

#endif
