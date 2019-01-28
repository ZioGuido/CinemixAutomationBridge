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

#include "stdafx.h"
#include "Plugin.h"
#include "Editor.h"

//////////////////////////////////////////////////////////////////////////
// CPluginEditor

// Constructor
#ifdef MACAU
COMPONENT_ENTRY(CPluginEditor)
CPluginEditor::CPluginEditor(AudioUnitCarbonView auv) : AEffGUIEditor (auv)
#else
CPluginEditor::CPluginEditor(AudioEffect* effect) : AEffGUIEditor(effect)
#endif
{
	// Load background image
	m_pBackground=new CBitmap(BACKGROUND);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)m_pBackground->getWidth();
	rect.bottom = (short)m_pBackground->getHeight();
}

// Destructor
CPluginEditor::~CPluginEditor()
{
	// free the background bitmap
	if (m_pBackground)
	{
		m_pBackground->forget();
		m_pBackground=NULL;
	}
}

// Implementation of open
bool CPluginEditor::open (void *ptr)
{
	// Do default
	AEffGUIEditor::open(ptr);

	// Create background frame
	CRect size(0, 0, m_pBackground->getWidth(), m_pBackground->getHeight());
	CFrame* pFrame = new CFrame(size, ptr, this);
	pFrame->setBackground(m_pBackground);

	// Setup GUI...
	CPoint point(0, 0);

	// Load Bitmaps
	CBitmap *bmp_AboutBox		= new CBitmap(ABOUTBOX);
	CBitmap *bmp_Fader			= new CBitmap(SLIDER);
	CBitmap *bmp_FaderBg		= new CBitmap(SLIDERBG);
	CBitmap *bmp_Button			= new CBitmap(BUTTON);
	CBitmap *bmp_PadFinger		= new CBitmap(PADFINGER);
	CBitmap *bmp_PadBg			= new CBitmap(PADBG);
	CBitmap *bmp_Activate		= new CBitmap(ACTIVATE);
	CBitmap *bmp_Deactivate		= new CBitmap(DEACTIVATE);
	CBitmap *bmp_Snapshot		= new CBitmap(SNAPSHOT);
	CBitmap *bmp_TestMode		= new CBitmap(TESTMODE);
	CBitmap *bmp_ResetAll		= new CBitmap(RESETALL);
	CBitmap *bmp_AllMutes		= new CBitmap(ALLMUTES);

	// About Screen
	size(0, 0, 340, 60); size.offset(0, 0); point(0, 0);
	CRect toDisplay(0, 0, bmp_AboutBox->getWidth(), bmp_AboutBox->getHeight());
	toDisplay.offset(250, 130);
	cAboutScreen = new CXfadeSplash (size, this, 10000, bmp_AboutBox, toDisplay, point);
	cAboutScreen->setTransparency(true);
	pFrame->addView(cAboutScreen);

#if 0
	// Print the date and time of the moment the source was compiled
	char buildDateTime[256]; sprintf(buildDateTime, "Build date: %s", __TIMESTAMP__);
	size(0, 0, 180, 10); size.offset(343, 53);
	CTextLabel *cVersionLabel = new CTextLabel(size);
	cVersionLabel->setText(buildDateTime);
	cVersionLabel->setFont(kNormalFontVerySmall);
	cVersionLabel->setFontColor(kBlackCColor);
	cVersionLabel->setHoriAlign(kLeftText);
	cVersionLabel->setTransparency(true);
	pFrame->addView(cVersionLabel);
#endif

#ifdef RTMIDI
	size(0, 0, 38, 38); size.offset(870, 5);
	cMidiMenu						= new COptionMenu(size, this, 10000, NULL, NULL, k3DIn | kNoTextStyle | kNoDrawStyle);
	COptionMenu *cMenuMidiIn1		= new COptionMenu(size, this, 10001, NULL, NULL, k3DIn | kCheckStyle);
	COptionMenu *cMenuMidiIn2		= new COptionMenu(size, this, 10002, NULL, NULL, k3DIn | kCheckStyle);
	COptionMenu *cMenuMidiOut1		= new COptionMenu(size, this, 10003, NULL, NULL, k3DIn | kCheckStyle);
	COptionMenu *cMenuMidiOut2		= new COptionMenu(size, this, 10004, NULL, NULL, k3DIn | kCheckStyle);

	// MidiIn1 SubMenu
	cMidiMenu->addEntry(cMenuMidiIn1, "MIDI Input port 1");
	for (int i=0; i<((CPlugin*)effect)->MidiInPorts; i++)
	{
		char txt[64]; sprintf(txt, "%d) %s", i+1, ((CPlugin*)effect)->MidiInPortName[i].c_str());
		cMenuMidiIn1->addEntry(txt);
		if (((CPlugin*)effect)->MidiInPort1 == i) cMenuMidiIn1->setCurrent(i);
	}
	// MidiIn2 SubMenu
	cMidiMenu->addEntry(cMenuMidiIn2, "MIDI Input port 2");
	for (int i=0; i<((CPlugin*)effect)->MidiInPorts; i++)
	{
		char txt[64]; sprintf(txt, "%d) %s", i+1, ((CPlugin*)effect)->MidiInPortName[i].c_str());
		cMenuMidiIn2->addEntry(txt);
		if (((CPlugin*)effect)->MidiInPort2 == i) cMenuMidiIn2->setCurrent(i);
	}
	// MidiOut1 SubMenu
	cMidiMenu->addEntry(cMenuMidiOut1, "MIDI Output port 1");
	for (int i=0; i<((CPlugin*)effect)->MidiOutPorts; i++)
	{
		char txt[64]; sprintf(txt, "%d) %s", i+1, ((CPlugin*)effect)->MidiOutPortName[i].c_str());
		cMenuMidiOut1->addEntry(txt);
		if (((CPlugin*)effect)->MidiOutPort1 == i) cMenuMidiOut1->setCurrent(i);
	}
	// MidiOut2 SubMenu
	cMidiMenu->addEntry(cMenuMidiOut2, "MIDI Output port 2");
	for (int i=0; i<((CPlugin*)effect)->MidiOutPorts; i++)
	{
		char txt[64]; sprintf(txt, "%d) %s", i+1, ((CPlugin*)effect)->MidiOutPortName[i].c_str());
		cMenuMidiOut2->addEntry(txt);
		if (((CPlugin*)effect)->MidiOutPort2 == i) cMenuMidiOut2->setCurrent(i);
	}
	pFrame->addView(cMidiMenu);
#endif


	// All Channel Strips
	int TagFader(0), TagMute(NUMBER_OF_FADERS), cn(1), S(1), PosX, PosY, i, b(0);
	for (i = 0; i < NUMBER_OF_CHANNELSTRIPS; i++)
	{
		// Set the horizontal position of the channel strip
		PosX = 10 + i * 22;

		// Make the label text
		char LabelText[8];
		if (i > 23 && i < 28) sprintf(LabelText, "S%d", S++);
		else sprintf(LabelText, "%d", cn++);

		///////////////////////////////////////////////////////////////////////////////////////////
		// Upper ROW //////////////////////////////////////////////////////////////////////////////
		PosY = 70;

		// Fader
		size(0, 0, 18, 200); size.offset(PosX, PosY);
		cFader[TagFader] = new CVerticalSlider(size, this, TagFader, point, 200, bmp_Fader, bmp_FaderBg, point);
		cFader[TagFader]->setTransparency(true);
		cFader[TagFader]->setDefaultValue(0.754f);
		pFrame->addView(cFader[TagFader]);

		// Mute
		size(0, 0,  16, 16); size.offset(PosX, PosY + 204);
		cButton[b] = new COnOffButton(size, this, TagMute, bmp_Button);
		pFrame->addView(cButton[b]);

		// Label
		size(0, 0, 16, 10); size.offset(PosX, PosY + 222);
		CTextLabel *cUpChLabel = new CTextLabel(size);
		cUpChLabel->setText(LabelText);
		cUpChLabel->setFont(kNormalFontVerySmall);
		cUpChLabel->setFontColor(kWhiteCColor);
		cUpChLabel->setTransparency(true);
		pFrame->addView(cUpChLabel);

		// Increment Tags
		TagFader++;
		TagMute++;

		// Buttons index
		b++;

		///////////////////////////////////////////////////////////////////////////////////////////
		// Lower ROW //////////////////////////////////////////////////////////////////////////////
		PosY = 325;

		// Fader
		size(0, 0, 18, 200); size.offset(PosX, PosY);
		cFader[TagFader] = new CVerticalSlider(size, this, TagFader, point, 200, bmp_Fader, bmp_FaderBg, point);
		cFader[TagFader]->setTransparency(true);
		cFader[TagFader]->setDefaultValue(0.754f);
		pFrame->addView(cFader[TagFader]);

		// Mute
		size(0, 0,  16, 16); size.offset(PosX, PosY + 204);
		cButton[b] = new COnOffButton(size, this, TagMute, bmp_Button);
		pFrame->addView(cButton[b]);

		// Label
		size(0, 0, 16, 10); size.offset(PosX, PosY + 222);
		CTextLabel *cLoChLabel = new CTextLabel(size);
		cLoChLabel->setText(LabelText);
		cLoChLabel->setFont(kNormalFontVerySmall);
		cLoChLabel->setFontColor(kWhiteCColor);
		cLoChLabel->setTransparency(true);
		pFrame->addView(cLoChLabel);

		// Increment Tags
		TagFader++;
		TagMute++;

		// Buttons index
		b++;
	}

	// Joystick 1 //////////////////////////////////////////////////////////
	size(0, 0, 86, 64); size.offset(PosX + 40, 70);
	cJoy1 = new CTouchPad(size, this, 500, bmp_PadBg, bmp_PadFinger, point);
	pFrame->addView(cJoy1);

	size(0, 0,  16, 16); size.offset(PosX + 40, 135);
	cJoyMute1 = new COnOffButton(size, this, 156, bmp_Button);
	pFrame->addView(cJoyMute1);

	size(0, 0, 60, 10); size.offset(PosX + 60, 140);
	CTextLabel *cJoy1Label = new CTextLabel(size);
	cJoy1Label->setText("JOYSTICK 1");
	cJoy1Label->setHoriAlign(kLeftText);
	cJoy1Label->setFont(kNormalFontVerySmall);
	cJoy1Label->setFontColor(kWhiteCColor);
	cJoy1Label->setTransparency(true);
	pFrame->addView(cJoy1Label);


	// Joystick 2 //////////////////////////////////////////////////////////
	size(0, 0, 86, 64); size.offset(PosX + 40, 170);
	cJoy2 = new CTouchPad(size, this, 501, bmp_PadBg, bmp_PadFinger, point);
	pFrame->addView(cJoy2);

	size(0, 0,  16, 16); size.offset(PosX + 40, 235);
	cJoyMute2 = new COnOffButton(size, this, 159, bmp_Button);
	pFrame->addView(cJoyMute2);

	size(0, 0, 60, 10); size.offset(PosX + 60, 240);
	CTextLabel *cJoy2Label = new CTextLabel(size);
	cJoy2Label->setText("JOYSTICK 2");
	cJoy2Label->setHoriAlign(kLeftText);
	cJoy2Label->setFont(kNormalFontVerySmall);
	cJoy2Label->setFontColor(kWhiteCColor);
	cJoy2Label->setTransparency(true);
	pFrame->addView(cJoy2Label);


	// Master Fader ////////////////////////////////////////////////////////
	size(0, 0, 18, 200); size.offset(PosX + 40, PosY);
	cMasterFader = new CVerticalSlider(size, this, 160, point, 200, bmp_Fader, bmp_FaderBg, point);
	cMasterFader->setTransparency(true);
	pFrame->addView(cMasterFader);

	// AUX Mutes
	for (int i = 0; i < 10; i++)
	{
		size(0, 0,  16, 16); size.offset(PosX + 70, PosY + i * 20);
		cAuxMute[i] = new COnOffButton(size, this, 144 + i, bmp_Button);
		pFrame->addView(cAuxMute[i]);

		char LabelText[16]; sprintf(LabelText, "AUX %d", i + 1);
		size(0, 0, 40, 10); size.offset(PosX + 90, PosY + 4 + i * 20);
		CTextLabel *cAuxLabel = new CTextLabel(size);
		cAuxLabel->setText(LabelText);
		cAuxLabel->setHoriAlign(kLeftText);
		cAuxLabel->setFont(kNormalFontVerySmall);
		cAuxLabel->setFontColor(kWhiteCColor);
		cAuxLabel->setTransparency(true);
		pFrame->addView(cAuxLabel);
	}

	// Activate Button /////////////////////////////////////////////////////
	size(0,0,86,18); size.offset(780,  5);
	CKickButton *cActivate = new CKickButton(size, this, 1000, bmp_Activate, point);
	pFrame->addView(cActivate);

	// Deactivate Button ///////////////////////////////////////////////////
	size(0,0,86,18); size.offset(780, 25);
	CKickButton *cDeactivate = new CKickButton(size, this, 1001, bmp_Deactivate, point);
	pFrame->addView(cDeactivate);

	// All Mutes Button ////////////////////////////////////////////////////
	size(0,0,86,18); size.offset(PosX + 40, PosY + 204 - 325 + 70);
	CKickButton *cAllMutes = new CKickButton(size, this, 1002, bmp_AllMutes, point);
	pFrame->addView(cAllMutes);

	// Reset All Button ////////////////////////////////////////////////////
	size(0,0,86,18); size.offset(PosX + 40, PosY + 204);
	CKickButton *cResetAll = new CKickButton(size, this, 1003, bmp_ResetAll, point);
	pFrame->addView(cResetAll);

	// Snapshot Button /////////////////////////////////////////////////////
	size(0,0,86,18); size.offset(680,  5);
	CKickButton *cSnapshot = new CKickButton(size, this, 1004, bmp_Snapshot, point);
	pFrame->addView(cSnapshot);

	// Snapshot Button /////////////////////////////////////////////////////
	size(0,0,86,18); size.offset(680, 25);
	cTestMode = new COnOffButton(size, this, 1005, bmp_TestMode);
	pFrame->addView(cTestMode);


	// Unload bitmaps
	bmp_AboutBox->forget();
	bmp_Fader->forget();
	bmp_Button->forget();
	bmp_PadFinger->forget();
	bmp_PadBg->forget();
	bmp_Activate->forget();
	bmp_Deactivate->forget();
	bmp_Snapshot->forget();
	bmp_TestMode->forget();
	bmp_ResetAll->forget();
	bmp_AllMutes->forget();

    // Set linear knob mode
	setKnobMode(kLinearMode);

	// Store frame pointer
	frame = pFrame;

	// Show current program settings
	for (int t=0; t < NUMBER_OF_PARAMETERS; t++)
		setParameter(t, effect->getParameter(t));

	return true;
}

// Implementation of close
void CPluginEditor::close()
{
	if (frame) { delete frame; frame = NULL; }
}

// Triggered when a parameter is received from VST automation
void CPluginEditor::setParameter(VstInt32 index, float value)
{
	if (!frame) return;

	// For all channel strips
	if (index < NUMBER_OF_FADERS*2)
	{
		if (index < NUMBER_OF_FADERS)
			cFader[index]->setValue(value);
		else
			cButton[index-NUMBER_OF_FADERS]->setValue(value);

	// Everything else
	} else {
		// AUX Mutes
		for (int i = 0; i < 10; i++)
			if (index == (144 + i))
				{ cAuxMute[i]->setValue(value); break; }

		// Master Section
		switch (index)
		{
		case 154:	cJoy1->setXValue(value); break;
		case 155:	cJoy1->setYValue(value); break;
		case 156:	cJoyMute1->setValue(value);	break;
		case 157:	cJoy2->setXValue(value); break;
		case 158:	cJoy2->setYValue(value); break;
		case 159:	cJoyMute2->setValue(value);	break;
		case 160:	cMasterFader->setValue(value);	break;
		}
	}
}

// Triggered when a GUI control is modified by the user
void CPluginEditor::valueChanged(CDrawContext* context, CControl* control)
{
	long paramID = control->getTag();
	float value = control->getValue();

	switch (paramID)
	{
	// Joysticks
	case 500:
		effect->setParameterAutomated(154, cJoy1->getXValue());
		effect->setParameterAutomated(155, cJoy1->getYValue());
		break;
	case 501:
		effect->setParameterAutomated(157, cJoy2->getXValue());
		effect->setParameterAutomated(158, cJoy2->getYValue());
		break;
	// Interface buttons
	case 1000:
		if (value > 0.5f) ((CPlugin*)effect)->ActivateMixer();
		break;
	case 1001:
		if (value > 0.5f) ((CPlugin*)effect)->DeactivateMixer();
		break;
	case 1002:
		if (value > 0.5f) ((CPlugin*)effect)->ToggleAllMutes();
		break;
	case 1003:
		if (value > 0.5f) ((CPlugin*)effect)->ResetAllMixer();
		break;
	case 1004:
		if (value > 0.5f) ((CPlugin*)effect)->SendSnapshot();
		break;
	case 1005:
		if (value > 0.5f) ((CPlugin*)effect)->TestModeON();
		else ((CPlugin*)effect)->TestModeOFF();
		break;

#ifdef RTMIDI
	// MIDI Ports Menu
	case 10001:		// MIDI In 1
		((CPlugin*)effect)->MidiInPort1 = (int)value;
		((CPlugin*)effect)->OpenMidiInPort1();
		((CPlugin*)effect)->SaveConfig();
		break;
	case 10002:		// MIDI In 2
		((CPlugin*)effect)->MidiInPort2 = (int)value;
		((CPlugin*)effect)->OpenMidiInPort2();
		((CPlugin*)effect)->SaveConfig();
		break;
	case 10003:		// MIDI Out 1
		((CPlugin*)effect)->MidiOutPort1 = (int)value;
		((CPlugin*)effect)->OpenMidiOutPort1();
		((CPlugin*)effect)->SaveConfig();
		break;
	case 10004:		// MIDI Out 2
		((CPlugin*)effect)->MidiOutPort2 = (int)value;
		((CPlugin*)effect)->OpenMidiOutPort2();
		((CPlugin*)effect)->SaveConfig();
		break;
#endif
	}

	// Pass values to the DSP thread (only parameter IDs)
	if (paramID < NUMBER_OF_PARAMETERS) effect->setParameterAutomated(paramID, value);

	control->setDirty();
}


void CPluginEditor::idle()
{
	// Call next cycle
	AEffGUIEditor::idle();

	if (((CPlugin*)effect)->TestMode)
	{
		// Animate faders and mutes
		for (int i=0; i<NUMBER_OF_FADERS*2; i++)
			// Calls the editor setParameter() which doesn't send MIDI data.
			setParameter(i, effect->getParameter(i));
	}
}
