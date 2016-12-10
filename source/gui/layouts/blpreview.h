










#ifndef BUILDPREVIEW_H
#define BUILDPREVIEW_H

#include "../../widget.h"
#include "../viewportw.h"
#include "../winw.h"

class BlPreview : public Win
{
public:
	BlPreview(Widget* parent, const char* n, void (*reframef)(Widget* w));

	virtual void inev(InEv* ie);
};


void Resize_BP_VP(Widget* w);
void Resize_BP_Tl(Widget* w);
void Resize_BP_Ow(Widget* w);

#endif
