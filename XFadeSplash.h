//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Fade IN and Fade OUT effects for Splash Screen
// Replacement Class for (VSTGUI 3.0) CSplashScreen
// Guido Scognamiglio - GSi - www.GenuineSoundware.com
// Feb 2009
//


#ifndef XFadeSplash_H
#define XFadeSplash_H

#include "stdafx.h"
#include "vstcontrols.h"


//-----------------------------------------------------------------------------
// CXfadeSplash Declaration
//!
//-----------------------------------------------------------------------------
class CXfadeSplash : public CControl
{
public:
	CXfadeSplash (const CRect &size, CControlListener *listener, long tag,
                              CBitmap *background,
                              CRect   &toDisplay,
                              CPoint  &offset)
:	CControl (size, listener, tag, background), 
	toDisplay (toDisplay), offset (offset), bitmapTransparency (255)
	{
		setSpeed(5); // Set default speed
	}
	virtual ~CXfadeSplash()
	{
	}

	class CXfadeSplashView : public CView
	{
	public:
		CXfadeSplashView (const CRect& size, CXfadeSplash* splashScreen) 
		: CView (size)
		, splashScreen (splashScreen) 
		{
			setBackground (splashScreen->getBackground ());
			fade_speed = splashScreen->getSpeed();
		}
		 
		void draw (CDrawContext *pContext)
		{
			pBackground->drawAlphaBlend (pContext, size, splashScreen->getOffset (), splashScreen->getBitmapTransparency ());
			setDirty (false);
		}

		void mouse (CDrawContext *pContext, CPoint &where, long button)
		{
			if (button & kLButton)
			{
				// FADE OUT
				alpha = splashScreen->getBitmapTransparency();
				while (alpha > 0)
				{
					splashScreen->setBitmapTransparency(alpha);
					getFrame()->doIdleStuff();
					setDirty();
					alpha -= fade_speed;
				}
				
				// Finish
				splashScreen->unSplash(pContext);
				getFrame()->setDirty();
				getFrame()->setModalView(0);
				forget();
			}
		}

	protected:
		CXfadeSplash* splashScreen;
		unsigned char fade_speed, alpha;
	};

	virtual void draw (CDrawContext* pContext)
	{
		if (value && pBackground)
			pBackground->drawAlphaBlend (pContext, toDisplay, offset, bitmapTransparency);
		setDirty (false);
	}

	virtual bool hitTest (const CPoint& where, const long buttons = -1)
	{
		bool result = CView::hitTest (where, buttons);
		if (result && !(buttons & kLButton))
			return false;
		return result;
	}

	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1)
	{
		if (!bMouseEnabled)
			return;

		if (button == -1) button = pContext->getMouseButtons ();

		if (listener && button & (kAlt | kShift | kControl | kApple))
		{
			if (listener->controlModifierClicked (pContext, this, button) != 0)
				return;
		}

		if (!(button & kLButton))
			return;

		value = !value;
		if (value)
		{
			CXfadeSplashView* ssv = new CXfadeSplashView (toDisplay, this);
			if (getFrame () && getFrame ()->setModalView (ssv))
			{
				// FADE IN...
				alpha = 0;
				while (alpha < 255)
				{
					setBitmapTransparency(alpha);
					getFrame()->doIdleStuff();
					ssv->setDirty();
					alpha += fade_speed;
				}
				// Finish
				alpha = 255;
				setBitmapTransparency(alpha);
				getFrame()->setDirty();
				
				if (listener)
					listener->valueChanged (pContext, this);
			}
			setDirty ();
		}
	}
	virtual void unSplash (CDrawContext *pContext = 0)
	{
		if (value)
		{
			value = 0;
			if (listener)
				listener->valueChanged (pContext, this);
		}
	}

	void setBitmapTransparency (unsigned char transparency)
	{
		bitmapTransparency = transparency;
	}
	unsigned char getBitmapTransparency () const { return bitmapTransparency; }

	void setSpeed(unsigned char speed = 1) 
	{ 
		if (speed < 1) speed = 1;
		if (speed > 64) speed = 64;
		fade_speed = speed; 
	}
	unsigned char getSpeed() const { return fade_speed; }

	const CPoint& getOffset () const { return offset; }

	CLASS_METHODS(CXfadeSplash, CControl)

protected:
	CRect	toDisplay;
	CPoint	offset;
	unsigned char bitmapTransparency;
	unsigned char fade_speed, alpha;
};

#endif