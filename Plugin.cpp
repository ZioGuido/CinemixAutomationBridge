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

/////////////////////////////////////////////////////////////////////////////
// Instantiation

// This function is called by the startup code in vstplugmain.cpp in the SDK.
// It is used to create an instance of the Plugin class.
AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new CPlugin(audioMaster);
}


/////////////////////////////////////////////////////////////////////////////
// CPlugin
#ifdef MACAU
COMPONENT_ENTRY(CPlugin)
#endif

// Constructor
CPlugin::CPlugin(audioMasterCallback audioMaster) :
	AudioEffectX(audioMaster, NUMBER_OF_PROGRAMS, NUMBER_OF_PARAMETERS)
{
	// Init random seed
	srand(time(0));

	FullInit = isOpen = false;
	TestMode = false;

#ifdef RTMIDI
	MidiInPort1 = MidiInPort2 = MidiOutPort1 = MidiOutPort2 = 0;
	RtMidiPortsAreOpen = false;

	// Midi Inputs, instantiate, count ports and get names
	MidiIn1 = new RtMidiIn();
	MidiIn2 = new RtMidiIn();
	MidiInPorts = MidiIn1->getPortCount();
	for (int i=0; i<MidiInPorts; i++)
	{
#if WIN32
		// I don't know why but MIDI IN ports add the index number at the end of the string. I have to remove it, I only need the literal name.
		string portName = MidiIn1->getPortName(i);
		string portNum = static_cast<ostringstream*>( &(ostringstream() << i) )->str();
		int pos = portName.rfind(" " + portNum);
		MidiInPortName.push_back(portName.erase(pos));
#elif MAC
		MidiInPortName.push_back(MidiIn1->getPortName(i));
#endif
	}

	// Midi Outputs, instantiate, count ports and get names
	MidiOut1 = new RtMidiOut();
	MidiOut2 = new RtMidiOut();
	MidiOutPorts = MidiOut1->getPortCount();
	for (int i=0; i<MidiOutPorts; i++) MidiOutPortName.push_back(MidiOut1->getPortName(i));

	// Load configuration from file
	LoadConfig();
#endif


	// Init random seed
	srand(time(0));

	setNumInputs(0);
	isSynth();

	// Configure plugin capabilities
	canProcessReplacing(true);
	setNumOutputs(2);
	setUniqueID(_Plugin_Unique_ID_);

	// And set the first program
	curProgram = 0;

	// Assign parameter names to all channel strips
	int TagFader(0), TagMute(NUMBER_OF_FADERS), cn(1), S(1);
	for (int i = 0; i < NUMBER_OF_CHANNELSTRIPS; i++)
	{
		// Make the label text
		char LabelText[8];
		if (i > 23 && i < 28) sprintf(LabelText, "S%d", S++);
		else sprintf(LabelText, "M%d", cn++);

		sprintf(ParameterName[TagFader++], "Fader Upper %s", LabelText);
		sprintf(ParameterName[TagFader++], "Fader Lower %s", LabelText);
		sprintf(ParameterName[TagMute++], "Mute Upper %s", LabelText);
		sprintf(ParameterName[TagMute++], "Mute Lower %s", LabelText);
	}

	// Assign parameter names to the AUX mutes
	for (int i = 144; i < 154; i++)
		sprintf(ParameterName[i], "AUX %d Mute", i - 143);

	// Assign parameter names to the Master Section
	sprintf(ParameterName[154], "Joystick 1 X");
	sprintf(ParameterName[155], "Joystick 1 Y");
	sprintf(ParameterName[156], "Joystick 1 Mute");
	sprintf(ParameterName[157], "Joystick 2 X");
	sprintf(ParameterName[158], "Joystick 2 Y");
	sprintf(ParameterName[159], "Joystick 2 Mute");
	sprintf(ParameterName[160], "Master Fader");

	// Init parameters and MIDI
	for (int c = 0; c < NUMBER_OF_PARAMETERS; c++)
	{
		m_Programs[0].m_prmValue[c] = 0;	// Set all parameters to 0
		MidiChannel[c] = 0;					// Set all MIDI channels to 0 (none)
		MidiController[c] = 0;				// Set all MIDI CC# to 0
		prev_CC_Val[c] = -1;				// Start previous values from -1 (impossible value)
	}
	// As default, set Master Fader to max.
	m_Programs[0].m_prmValue[160] = 1.f;

	// Assign CC numbers and Channels
	// Left part, the first 24 channel strips (48 faders and mutes)
	int f, m;
	f = 0; m = NUMBER_OF_FADERS;
	for (int c = 0; c < 24; c++)
	{
		// Upper Fader
		MidiChannel[f] = 1;
		MidiController[f] = f * 2;
		MidiController2[f] = MidiController[f] + 1;
		f++;

		// Lower Fader
		MidiChannel[f] = 1;
		MidiController[f] = f * 2;
		MidiController2[f] = MidiController[f] + 1;
		f++;

		// Upper Mute
		MidiChannel[m] = 3;
		MidiController[m] = m - NUMBER_OF_FADERS;
		m++;

		// Lower Mute
		MidiChannel[m] = 3;
		MidiController[m] = m - NUMBER_OF_FADERS;
		m++;
	}

	// Right part, from channel 25 and above (other 24 faders and mutes)
	f = 48; m = 120;
	for (int c = 0; c < 12; c++)
	{
		// Upper Fader
		MidiChannel[f] = 2;
		MidiController[f] = (f - 48) * 2;
		MidiController2[f] = MidiController[f] + 1;
		f++;

		// Lower Fader
		MidiChannel[f] = 2;
		MidiController[f] = (f - 48) * 2;
		MidiController2[f] = MidiController[f] + 1;
		f++;

		// Upper Mute
		MidiChannel[m] = 4;
		MidiController[m] = m - 120;
		m++;

		// Lower Mute
		MidiChannel[m] = 4;
		MidiController[m] = m - 120;
		m++;
	}

	// AUX Mutes
	for (int i = 144; i < 154; i++)
	{
		// All AUX Mutes use CC# 96 on Ch. 5 but with different values according to AUX number.
		MidiChannel[i] = 5;
		MidiController[i] = 96;
	}

	// Master Section
	MidiChannel[154] = 2; MidiController[154] = 48;			// Joy 1 X
	MidiChannel[155] = 2; MidiController[155] = 50;			// Joy 1 Y
	MidiChannel[156] = 4; MidiController[156] = 24;			// Joy 1 Mute
	MidiChannel[157] = 2; MidiController[157] = 52;			// Joy 2 X
	MidiChannel[158] = 2; MidiController[158] = 54;			// Joy 2 Y
	MidiChannel[159] = 4; MidiController[159] = 26;			// Joy 2 Mute
	MidiChannel[160] = 5; MidiController[160] = 0;			// Master Fader

	AllMutesStatus = false;

	// Set default program name
	setProgramName((char*)_Plugin_Product_Name_);

	// Finally, Setup the GUI editor
#ifndef MACAU
	editor = new CPluginEditor(this);
#endif
#if MACAU
    // Fetch program names (for DP and auval)
    fetchProgramNames();
#endif
}

// Destuctor
CPlugin::~CPlugin()
{
	DeactivateMixer();

#ifdef RTMIDI
	RtMidiPortsAreOpen = false;

	SaveConfig();

	// Close MIDI Ports
	CloseAllMidiPorts();

	// Delete RtMidi pointers
	delete MidiIn1, MidiIn2, MidiOut1, MidiOut2;
#endif

	// Free some resources
	if (editor != NULL)
#if MACAU
		editor->~AEffGUIEditor();
#else
		editor->~AEffEditor();
#endif
	editor = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// Plugin information and capabilities

// Implementation of getEffectName
bool CPlugin::getEffectName(char* name)
{
	strcpy(name, _Plugin_Product_Name_);
	return true;
}

// Implementation of getProductString
bool CPlugin::getProductString(char* text)
{
	strcpy(text, _Plugin_Product_Name_);
	return true;
}

// Implementation of getVendorString
bool CPlugin::getVendorString(char* text)
{
	strcpy(text, "GSi");
	return true;
}

// Implementation of getVendorVersion
VstInt32 CPlugin::getVendorVersion()
{
	return _Plugin_Version_;
}

// Implementation of getPlugCategory
VstPlugCategory CPlugin::getPlugCategory()
{
	return kPlugCategSynth;
}

// Implementation of canDo
VstInt32 CPlugin::canDo(char* text)
{
	// Can Do...
	if(!strcmp(text, "receiveVstEvents"))		return 1;
	if(!strcmp(text, "receiveVstMidiEvent"))	return 1;
	if(!strcmp(text, "receiveVstTimeInfo"))		return 1;

#ifndef RTMIDI
	// Enable MIDI output
	if(!strcmp(text, "sendVstMidiEvent"))		return 1;
	if(!strcmp(text, "sendVstEvents"))			return 1;
#endif

	// Can't do anything else!
	return -1;
}

//--Some VST specific stuff----------------------------------------------------------------------------------
void CPlugin::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
}
void CPlugin::setBlockSize (VstInt32 blockSize)
{
	AudioEffectX::setBlockSize (blockSize);
}
bool CPlugin::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
#ifdef FXVERSION
	if (index < 2)
	{
		sprintf (properties->label, _Plugin_Product_Name_);
		sprintf (properties->shortLabel, _Plugin_Product_Name_);
		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;
		return true;
	}
#endif
	return false; // no audio inputs
}
bool CPlugin::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index < 2)
	{
		sprintf (properties->label, _Plugin_Product_Name_);
		sprintf (properties->shortLabel, _Plugin_Product_Name_);
		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;
		return true;
	}
	return false;
}
VstInt32 CPlugin::getNumMidiInputChannels ()
{
#ifdef RTMIDI
	return 1;
#else
	return 5;
#endif
}
VstInt32 CPlugin::getNumMidiOutputChannels ()
{
#ifdef RTMIDI
	return 0;
#else
	return 5;
#endif
}


/////////////////////////////////////////////////////////////////////////////
// Programs

// Implementation of getProgram
VstInt32 CPlugin::getProgram()
{
	return curProgram;
}
// Implementation of setProgram
void CPlugin::setProgram(VstInt32 program)
{
	if (!FullInit) return;
	if (program > NUMBER_OF_PROGRAMS-1) return;

	// Store current program number
	curProgram = program;

	for (int index=0; index < NUMBER_OF_PARAMETERS; index++) {
		float value = m_Programs[curProgram].m_prmValue[index];
		setParameterAutomated(index, value);
	}
}

// Implementation of setProgramName
void CPlugin::setProgramName(char *name)
{
	strcpy(m_Programs[curProgram].m_szName, name);
}

// Implementation of getProgramName
void CPlugin::getProgramName(char *name)
{
	if (!strcmp(m_Programs[curProgram].m_szName, "Init"))
		sprintf(name, "%s %d", m_Programs[curProgram].m_szName, curProgram + 1);
	else
		strcpy(name, m_Programs[curProgram].m_szName);
}

// Implementation of getProgramNameIndexed
bool CPlugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	if (index < _countof(m_Programs))
	{
		strcpy(text, m_Programs[index].m_szName);
		return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// Parameters

// Implementation of setParameter
void CPlugin::setParameter(VstInt32 index, float value)
{
	if (!FullInit) return;

	// First thing, store the parameter value to the right par. tag
	m_Programs[curProgram].m_prmValue[index] = value;

	///////////////////////////////////////////////////////////////
	// SEND MIDI DATA
	SendMidiData(index, value);

	// Update GUI when changing parameter values from VST automation
	if (editor) ((AEffGUIEditor*)editor)->setParameter(index, value);

#ifdef MACAU
	AUBase::SetParameter((UInt32)index,kAudioUnitScope_Global,0,(Float32)value,(UInt32)0);
#endif
}

// Implementation of getParameter
float CPlugin::getParameter(VstInt32 index)
{
	if (index < NUMBER_OF_PARAMETERS+1) return m_Programs[curProgram].m_prmValue[index];
	else return 0;
}

// Implementation of getParameterName
void CPlugin::getParameterName(VstInt32 index, char *label)
{
	strcpy(label, ParameterName[index]);
}

// Implementation of getParameterDisplay
void CPlugin::getParameterDisplay(VstInt32 index, char *text)
{
	float value = m_Programs[curProgram].m_prmValue[index];
	sprintf(text, "%d", int(value * 127));
}

// Implementation of getParameterLabel
void CPlugin::getParameterLabel(VstInt32 index, char *label)
{
	strcpy(label, ParameterName[index]);
}


/////////////////////////////////////////////////////////////////////////////
// Audio processing


// Implementation of resume
void CPlugin::resume()
{
	// Do default...
	AudioEffectX::resume();

	// Done
	FullInit = isOpen = true;

	// Recall parameter values
	setProgram(curProgram);
}


// Implementatin of processEvents
VstInt32 CPlugin::processEvents(VstEvents* events)
{
	// Process all events...
	for (VstInt32 i=0; i<events->numEvents; i++)
	{
		// Get event and check if its a MIDI event
		VstEvent* pEvent=events->events[i];
		if (pEvent && pEvent->type==kVstMidiType)
		{
			VstMidiEvent* pMidiEvent=(VstMidiEvent*)pEvent;
			unsigned char* midiData = (unsigned char*)pMidiEvent->midiData;
			unsigned char bChannel	= midiData[0] & 0x0f; bChannel += 1;
			unsigned char bByte1	= midiData[1] & 0x7f;
			unsigned char bByte2	= midiData[2] & 0x7f;

			switch (midiData[0] & 0xf0)
			{
				// This is an experiment: Send Midi note events to "play the faders" :)
				/*
				case MIDI_STATUS_NOTEON:
					if (bByte1 >= 48 && bByte1 <= 79)
					{
						int n = bByte1 < 72 ? bByte1 - 48 : bByte1 - 72 + 4;
						int c = bByte1 < 72 ? 1 : 2;
						int f = n * 4 + 2;
						ProcessMIDIControlChange(c, f, bByte2);
					}
					break;

				case MIDI_STATUS_NOTEOFF:
					if (bByte1 >= 48 && bByte1 <= 79)
					{
						int n = bByte1 < 72 ? bByte1 - 48 : bByte1 - 72 + 4;
						int c = bByte1 < 72 ? 1 : 2;
						int f = n * 4 + 2;
						ProcessMIDIControlChange(c, f, 0);
					}
					break;
				*/

				case MIDI_STATUS_CONTROLCHANGE:
#ifndef RTMIDI
					ProcessMIDIControlChange(bChannel, bByte1, bByte2);
#endif
					break;

				case MIDI_STATUS_PITCHBEND:
					break;

				case MIDI_STATUS_CHANNELPRESSURE:
					break;
			}
		}
	}

	return 1;
}

// Implementation of OnNoteOn
void CPlugin::OnNoteOn(unsigned char bChannel, unsigned char bNote, unsigned char bVelocity, VstInt32 deltaFrames)
{
}

// Implementation of OnNoteOff
void CPlugin::OnNoteOff(unsigned char bChannel, unsigned char bNote, unsigned char bVelocity, VstInt32 deltaFrames)
{
}

void CPlugin::OnDamper(bool PedalDown)
{
}

// Implementation of OnAllNotesOff
void CPlugin::OnAllNotesOff()
{
	for (int n=0; n<128; n++) {
		for (int c=0; c<16; c++) OnNoteOff(c, n, 0, 0);
	}
	PedalDown = false;
	OnDamper(PedalDown);
}


void CPlugin::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	processReplacing(inputs, outputs, sampleFrames);
}

// Implementation of processReplacing
void CPlugin::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
	// Init audio outputs
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	// The audio loop (REQUIRED BY SOME HOSTS!)
	for (int s=0; s<sampleFrames; s++)
	{
		if (TestMode)
		{
			// This routine is executed n times in a second in order to calculate and send MIDI data
			// at an acceptable rate that doesn't exceed the MIDI capabilities.
			// MIDI data is sent separately, while the GUI animation is called in the Editor idle function.
			// This is because each VST host calls the GUI idle funciton at a different rate. This way, I am sure
			// that MIDI data is sent at a controlled rate, separately from what's being processed for the GUI.
			// Also, the test animation doesn't write automation to the host sequencer.

			if (++count_samples1 > FaderSpeed) count_samples1 = 0;
			if (++count_samples2 > MuteSpeed) count_samples2 = 0;

			// Animate Faders
			if (count_samples1 == 0)
			{
				for (int i=0; i<NUMBER_OF_FADERS; i++)
				{
					Anim_ramp[i] += 0.05f; if (Anim_ramp[i] > 1.f) Anim_ramp[i] -= 2.f;	// ramp
					float y = (Anim_ramp[i] * (1.f - fabs(Anim_ramp[i])) * 2.f) + 0.5f;

					m_Programs[curProgram].m_prmValue[i] = y;
					SendMidiData(i, y);
				}
			}

			// Animate Mutes
			if (count_samples2 == 0)
			{
				for (int i=NUMBER_OF_FADERS; i<NUMBER_OF_FADERS*2; i++)
				{
					float y = RandGen() > 0.5f ? 1.f : 0;

					m_Programs[curProgram].m_prmValue[i] = y;
					SendMidiData(i, y);
				}
			}
		}

		// Output just silence :)
		*out1++ = 0;
		*out2++ = 0;
	}
}
