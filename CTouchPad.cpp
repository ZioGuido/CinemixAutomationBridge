

#ifndef __CTouchPad__
#include "CTouchPad.h"
#endif

#include <math.h>

CTouchPad::CTouchPad (const CRect &size, CControlListener *listener, long tag, CBitmap *background, CBitmap *handle, const CPoint &offset)
		  : CControl (size, listener, tag, background), pHandle (handle)
{
	if (pHandle)						// if the handle image exists
		pHandle->remember ();			// remember it and setup the inset

	setDefaultValue (0.5f);				// set the controls default value
	xValue = yValue = value = getDefaultValue();

	MouseOver = false;
	xRest = 0; yRest = 1.f;
}

CTouchPad::~CTouchPad ()
{
	if (pHandle)
		pHandle->forget ();
}

//////////////////// DRAW ///////////////////////////////////////
//
// 	void CTouchPad::draw (CDrawContext)
//
//	draw the background panel
//
/////////////////////////////////////////////////////////////////

void CTouchPad::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, offset);
		else
			pBackground->draw (pContext, size, offset);
	}
	drawHandle (pContext);
	setDirty (false);
}

//////////////////// DRAW HANDLE ////////////////////////////////
//
// 	void CTouchPad::drawHandle(CDrawContext)
//
//	draw the handle, based on current x/y position
//
/////////////////////////////////////////////////////////////////

void CTouchPad::drawHandle (CDrawContext *pContext)
{
	CPoint where;
	valueToPoint (where);

	if (pHandle)
	{
		long width  = pHandle->getWidth ();
		long height = pHandle->getHeight ();

		// create a Crect of the right size at 0,0
		// offset it by the value in valueToPoint
		// draw the handle at new location
		CRect handleSize (0, 0, width, height);
		handleSize.offset (where.h, where.v);
		pHandle->drawTransparent (pContext, handleSize);
	}
}

//////////////////// MOUSE //////////////////////////////////////
//
// 	void CTouchPad::mouse (CDrawContext, CPoint)
//
//	Called when the user uses the mouse.
//	If they control-Left click, the pad resets to the default value.
//
/////////////////////////////////////////////////////////////////

void CTouchPad::mouse (CDrawContext *pContext, CPoint &where, long button)
{
	if (!bMouseEnabled)
		return;

	if (button == -1) button = pContext->getMouseButtons ();

	// set the default value
	if (listener && button & (kAlt | kShift | kControl | kApple))
	{
		value = getDefaultValue ();
		xValue = yValue = value;
		if (isDirty () && listener)
			listener->valueChanged (pContext, this);
		return;
	}

	// begin of edit parameter
	beginEdit();
	do
	{
		MouseOver = true;
		value = valueFromPoint (where);

		if (button & kRButton)
			calcRestPoint(where);

		if (listener)
			listener->valueChanged (pContext, this);

		pContext->getMouseLocation (where);

		doIdleStuff ();
	}
	while (pContext->getMouseButtons ());

	// end of edit parameter
	endEdit();
	MouseOver = false;
}

//////////////////// VALUE TO POINT /////////////////////////////
//
// 	void CTouchPad::valueToPoint (CPoint)
//
//	Given the current x and y values, where should we
//	draw the handle?
//
/////////////////////////////////////////////////////////////////

void CTouchPad::valueToPoint (CPoint &point)
{
	// Get the width and height of the handle
	float	pHandleWidth = (float)pHandle->getWidth();
	float	pHandleHeight = (float)pHandle->getHeight();

	// Convert the x/y values in to locations on screen
	float	numXPixels = xValue * (float)(size.width() - pHandleWidth) + size.left;
	float	numYPixels = yValue * (float)(size.height() - pHandleHeight) + size.top;

	// Store them in the point.
	point.h = (long)numXPixels;
	point.v = (long)numYPixels;
}

//////////////////// VALUE FROM POINT ///////////////////////////
//
// 	void CTouchPad::valueFromPoint (CPoint)
//
//	Given a point on the display, what values
//	should the x and y values have?
//
/////////////////////////////////////////////////////////////////

float CTouchPad::valueFromPoint (CPoint &point)
{
	float numXPixels, numYPixels;
	float pHandleWidth;
	float pHandleHeight;

	// Get the widths of the handle, and setup an offset
	pHandleWidth  = (float)pHandle->getWidth();
	pHandleHeight = (float)pHandle->getHeight();

	// Remove any offset applied by the onscreen position
	numXPixels = (float)point.h - (float)size.left - pHandleWidth  / 2;
	numYPixels = (float)point.v - (float)size.top  - pHandleHeight / 2;

	// Wrap them within the range of pixels
	if (numYPixels > (float)size.height() - pHandleHeight)
		numYPixels = (float)size.height() - pHandleHeight;
	if (numXPixels > (float)size.width()  - pHandleWidth)
		numXPixels = (float)size.width()  - pHandleWidth;

	// Convert the ranges
	numXPixels /= (float)(size.width()  - pHandleWidth);
	numYPixels /= (float)(size.height() - pHandleHeight);

	// Ensure the range
	if (numXPixels < 0.f)
		numXPixels = 0.f;
	if (numYPixels < 0.f)
		numYPixels = 0.f;

	// The output Values
	xValue = numXPixels;
	yValue = numYPixels;

	return (xValue + yValue) / 2.f;
}

void CTouchPad::calcRestPoint(CPoint &point)
{
	float numXPixels, numYPixels;
	float pHandleWidth;
	float pHandleHeight;

	// Get the widths of the handle, and setup an offset
	pHandleWidth  = (float)pHandle->getWidth();
	pHandleHeight = (float)pHandle->getHeight();

	// Remove any offset applied by the onscreen position
	numXPixels = (float)point.h - (float)size.left - pHandleWidth  / 2;
	numYPixels = (float)point.v - (float)size.top  - pHandleHeight / 2;

	// Wrap them within the range of pixels
	if (numYPixels > (float)size.height() - pHandleHeight)
		numYPixels = (float)size.height() - pHandleHeight;
	if (numXPixels > (float)size.width()  - pHandleWidth)
		numXPixels = (float)size.width()  - pHandleWidth;

	// Convert the ranges
	numXPixels /= (float)(size.width()  - pHandleWidth);
	numYPixels /= (float)(size.height() - pHandleHeight);

	// Ensure the range
	if (numXPixels < 0.f)
		numXPixels = 0.f;
	if (numYPixels < 0.f)
		numYPixels = 0.f;

	// The output Values
	xRest = numXPixels;
	yRest = numYPixels;
}

//////////////////// SET HANDLE BITMAP //////////////////////////
//
// 	void CTouchPad::setHandleBitmap (CBitmap)
//
//	Set the handlebitmap to be a new image
//
/////////////////////////////////////////////////////////////////

void CTouchPad::setHandleBitmap (CBitmap *bitmap)
{
	if (pHandle)
	{
		pHandle->forget ();
		pHandle = 0;
	}

	if (bitmap)
	{
		pHandle = bitmap;
		pHandle->remember ();
	}
}