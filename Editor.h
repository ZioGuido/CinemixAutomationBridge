/////////////////////////////////////////////////////////////////////////////
/*
	D&R Cinemix Automation Bridge V.1.01
	by Guido Scognamiglio - www.GenuineSoundware.com
	Programming started: July 2012
	Last Update: Feb 2017

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	Any person wishing to distribute modifications to the Software is
	asked to send the modifications to the original developer so that
	they can be incorporated into the canonical version.  This is,
	however, not a binding provision of this license.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
	ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __PluginEDITOR_H
#define __PluginEDITOR_H

#include "CTouchPad.h"
#include "XfadeSplash.h"

// CPluginEditor Class
class CPluginEditor :
	public AEffGUIEditor,
	public CControlListener
{
public:
// Construction
#ifdef MACAU
	CPluginEditor(AudioUnitCarbonView auv);
#else
	CPluginEditor(AudioEffect* effect);
#endif
	virtual ~CPluginEditor();

	virtual void idle();

protected:
	// Bitmaps
	CBitmap			*m_pBackground;
	CXfadeSplash	*cAboutScreen;

	// GUI Controls/Parameters
	CVerticalSlider		*cFader[NUMBER_OF_FADERS];
	COnOffButton		*cButton[NUMBER_OF_FADERS];
	COnOffButton		*cAuxMute[10];

	CVerticalSlider		*cMasterFader;
	CTouchPad			*cJoy1, *cJoy2;
	COnOffButton		*cJoyMute1, *cJoyMute2;
	COnOffButton		*cTestMode;

#ifdef RTMIDI
	COptionMenu			*cMidiMenu;
#endif

// AEffGUIEditor
	virtual bool open(void* ptr);
	virtual void close();
	virtual void setParameter(VstInt32 index, float value);

// ControlListener
	virtual void valueChanged(CDrawContext* context, CControl* control);
};

#endif	// __PluginEDITOR_H
