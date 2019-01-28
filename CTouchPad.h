
#ifndef __XY_PAD_
#define __XY_PAD_

#include "stdafx.h"
#ifndef __vstgui__
#include "vstgui.h"
#endif


class CTouchPad : public CControl
{
public:

	CTouchPad (const CRect &size, CControlListener *listener, long tag, CBitmap *background, CBitmap *handle, const CPoint &offset);
	virtual ~CTouchPad ();

	virtual void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where, long button);
	virtual void drawHandle (CDrawContext *pContext);
	virtual void valueToPoint (CPoint &point);
	virtual float valueFromPoint (CPoint &point);
	void calcRestPoint(CPoint &point);
	virtual void setHandleBitmap (CBitmap *bitmap);

	float getXValue() { return xValue; }
	float getYValue() { return yValue; }
	void getRestPoint(float &x, float &y) { x = xRest; y = 1.f - yRest; }
	void setXValue(float x) { xValue = x; setDirty(); }
	void setYValue(float y) { yValue = y; setDirty(); }
	bool isMouseOver() { return MouseOver; }

protected:

	CPoint   offset;
	CBitmap *pHandle;

	float	 xValue, xRest;
	float    yValue, yRest;

	bool	MouseOver;
};

#endif