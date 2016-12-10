










#ifndef BLGRAPHS_H
#define BLGRAPHS_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../winw.h"
#include "../../../sim/resources.h"

uint8_t BlTypeGraphIn(uint8_t bti, uint8_t sgi);
bool ValidBlInOutRes(uint8_t bti, uint8_t ri);
bool ValidBlDemRes(uint8_t bti, uint8_t ri);

class BlGraphs : public Win
{
public:
	BlGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_close;
	InsDraw m_graphs;
	DropList m_rsel;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
	void regvals(Selection* sel);
	virtual void reframe();
	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};

#define BLGR_INOUT_BEG	0
#define BLGR_INOUT_END	(BLGR_INOUT_BEG + RESOURCES)
#define BLGR_DEM_BEG	BLGR_INOUT_END
#define BLGR_DEM_END	(BLGR_DEM_BEG + RESOURCES)
#define BLGR_DEMEX_BEG	BLGR_DEM_END
#define BLGR_DEMEX_END	(BLGR_DEMEX_BEG + RESOURCES)

void Resize_BG_Close(Widget *thisw);
void Click_BG_Close();
void Resize_BG_RSel(Widget *thisw);

#endif
